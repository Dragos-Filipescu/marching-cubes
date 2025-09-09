#include <profiling/chunk_manager.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/noise.hpp>
#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <deque>
#include <mutex>
#include <print>
#include <random>
#include <stop_token>
#include <utility>
#include <vector>

#include <camera/camera_first_person.hpp>
#include <core/aliases.hpp>
#include <managers/chunk_manager.hpp>
#include <scene/vertex.hpp>
#include <utils/frame_stats.hpp>
#include <utils/marching_cubes.hpp>
#include <utils/thread_pool.hpp>

namespace marching_cubes::profiling {

    using camera::CameraFirstPerson;
	using utils::frame_stats::FrameStats;
	using utils::frame_stats::FrameStatsSnapshot;
	using scene::VertexPacking;
    using scene::BasicVertex;
    using scene::ChunkId;
    using scene::ChunkInfo;
    using scene::ChunkState;
    using scene::Normal;
    using scene::Position;

    // ----------------- Dummy GPU buffers -----------------
    struct DummyBuffer {
        std::size_t getSize() const noexcept { return 1ull; }
        void* getBuffer() const noexcept { return nullptr; }
        void updateSize(VkDeviceSize) {}
    };

    using IndexT = u32;
    using DummyChunkManager = managers::BasicChunkManager<
        scene::BasicVertex<VertexPacking::Standard, scene::Position, scene::Normal>,
        DummyBuffer,
        IndexT,
        DummyBuffer
    >;

    // ----------------- Samplers and MC entry -----------------
    struct SphereSampler {
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            constexpr f32 r = 30.0f; // large-ish to produce geometry
            return std::sqrt(x * x + y * y + z * z) - r;
        }
    };

    struct NoiseSampler {
        f32 operator()(f32 x, f32 y, f32 z) const noexcept
        {
            return glm::simplex(glm::vec3{ x, y, z } * 0.1f) * 10.0f;
		}
    };

    struct Result {
        ChunkId id;
        u64 version;
        u32 vcount;
        u32 icount;
    };

    struct ResultQueue {
        std::mutex mtx{};
        std::vector<Result> q{};
	};

    template<typename VertexT, typename IndexT>
    static utils::marching_cubes::MeshData<VertexT, IndexT> genMesh32Cube(
        const glm::vec3& origin,
        const glm::vec3& spacing
    )
    {
        using namespace utils::marching_cubes;
        return marchingCubes<VertexT, IndexT>(
            SphereSampler{},
            glm::ivec3{ 33, 33, 33 },  // 32^3 cells => 33^3 grid points
            0.0f,
            origin,
            spacing
        );
    }

    template<typename VertexT, typename IndexT>
    static utils::marching_cubes::MeshData<VertexT, IndexT> genNoise(
        const glm::vec3& origin,
        const glm::vec3& spacing
    )
    {
        using namespace utils::marching_cubes;
        return marchingCubes<VertexT, IndexT>(
            NoiseSampler{},
            glm::ivec3{ 33, 33, 33 },  // 32^3 cells => 33^3 grid points
            0.0f,
            origin,
            spacing
        );
    }

    static void simulateGameWork()
    {
        static thread_local std::mt19937_64 rng{ 0xDEADBEEFull }; // independent, deterministic seed
        // Noise
        f64 sum = 0.0;
        // Entity update
        for (int i = 0; i < 100; ++i) {
            sum = std::sin(sum + i * 0.001);
        }
        volatile f64 sink = sum; // stop optimization
    }

    // ----------------- Camera setup -----------------
    static void setupCamera(CameraFirstPerson& cam)
    {
        cam.getTransform().setPosition({ 5.0f, 3.0f, 10.0f });
        const glm::vec3 eye = cam.getTransform().getPosition();
        const glm::vec3 forward = glm::normalize(glm::vec3{ -0.6f, -0.2f, -0.8f });
        cam.lookAt(eye + forward);
        cam.setProjection(glm::radians(90.0f), 1.0f, 0.1f, 300.0f);
    }

    // ----------------- Frame driver -----------------
    struct BenchStats {
        u32 frames = 0;
        u64 totalDirty = 0;
        u64 totalDraws = 0;
        u64 submitted = 0;    // async only
        u64 completed = 0;    // async only
        u32 maxInflight = 0;  // async only
        std::chrono::nanoseconds wall = {};
    };

    static void randomCameraJitter(CameraFirstPerson& cam)
    {
        static thread_local std::mt19937_64 rng{ 0xCAFEBEEFull };
        std::uniform_real_distribution<f32> yaw(-5.0f, 5.0f);
        std::uniform_real_distribution<f32> pitch(-3.0f, 3.0f);
        std::uniform_real_distribution<f32> step(-8.0f, 8.0f);

        int mode = std::uniform_int_distribution<int>(0, 3)(rng);
        switch (mode) {
        case 0: // rotation jitter
            cam.processMouseDelta(
                glm::vec2{
                    yaw(rng) * cam.getSensitivityX(),
                    pitch(rng) * cam.getSensitivityY()
                }
            );
            break;
        case 1: // sub-chunk nudges
        {
            int axis = std::uniform_int_distribution<int>(0, 2)(rng);
            f32 s = step(rng) * 0.25f; // smaller
            if (axis == 0) cam.translateRight(s);
            else if (axis == 1) cam.translateForward(s);
            else cam.translateUpward(s);
            break;
        }
        case 2: // chunk-sized jumps
        {
            int axis = std::uniform_int_distribution<int>(0, 2)(rng);
            f32 s = step(rng) * 32.0f; // multiple voxels
            if (axis == 0) cam.translateRight(s);
            else if (axis == 1) cam.translateForward(s);
            else cam.translateUpward(s);
            break;
        }
        case 3: // combined: rotate+translate
            cam.processMouseDelta(glm::vec2{ yaw(rng) * cam.getSensitivityX(), 0 });
            cam.translateForward(step(rng));
            break;
        }
    }

    // ----------------- Dummy staged upload -----------------
    static void simulateUpload()
    {
        static thread_local std::mt19937_64 rng{ 0xBEEFDEADull }; // independent, deterministic seed
        // Pretend vkQueueSubmit + fence wait cost
        std::uniform_int_distribution<int> delay(100, 500); // µs range
        int micros = delay(rng);
        std::this_thread::sleep_for(std::chrono::microseconds(micros));
    }

    // ----------------- SYNC bench -----------------
    static BenchStats runSync(u32 frames)
    {
        std::println("=== ChunkManager Bench: SYNC ===");

        CameraFirstPerson cam;
        setupCamera(cam);

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ 32 },
            DummyChunkManager::VoxelSize{ 4 },
            DummyBuffer{},
            DummyBuffer{}
        };

        BenchStats stats{};  // default 1s reporting interval
        FrameStats fs{ std::chrono::seconds{1} }; // 1s reporting interval
        auto prev = FrameStats::ClockType::now();
        const auto t0 = prev;

        for (u32 f = 0; f < frames; ++f) {

            auto now = FrameStats::ClockType::now();
            fs.addFrame(now, prev);
            prev = now;

            randomCameraJitter(cam);

            u32 frameDraws = 0;

            // generator: do work synchronously and mark Clean
            auto gen_cb = [&](const ChunkId& cid, std::shared_ptr<ChunkInfo> info)
            {
                (void)cid;
                // Generate a small, fixed MC mesh to emulate work
                auto mesh = genMesh32Cube<
                    BasicVertex<VertexPacking::Standard, Position, Normal>,
                    IndexT
                >(
                    glm::vec3{ cid } *static_cast<f32>(mgr.getChunkSize()),
                    glm::vec3{ static_cast<f32>(mgr.getVoxelSize()) }
                );
                info->vertexCount = static_cast<u32>(mesh.vertices.size());
                info->indexCount = static_cast<u32>(mesh.indices.size());

                simulateUpload();

                info->state = ChunkState::Clean;
            };

            auto draw_cb = [&](const ChunkId&, std::shared_ptr<ChunkInfo> info)
            {
                (void)info;
                // In real app we’d record vkCmdDrawIndexed; here just count
                frameDraws++;
            };

            mgr.update().generate(gen_cb);

            simulateGameWork();

            mgr.draw(draw_cb);

            stats.totalDirty += static_cast<u64>(mgr.getDirtyChunks().size());
            stats.totalDraws += frameDraws;
            stats.frames++;

            if (fs.ready(now)) {
                FrameStatsSnapshot snap = fs.snapshotAndBump(now);
                std::println("  [Frame {}] {}", f, snap); // uses your formatter
            }
        }

        const auto t1 = FrameStats::ClockType::now();
        stats.wall = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);

        // Report
        const f64 sec = std::chrono::duration<f64>(stats.wall).count();
        std::println(
            "Frames: {}  time: {:.3f}s  FPS: {:.1f}",
            stats.frames,
            sec,
            stats.frames / sec
        );
        std::println(
            "Avg dirty/frame: {:.2f}  Avg draws/frame: {:.2f}",
            static_cast<f64>(stats.totalDirty) / stats.frames,
            static_cast<f64>(stats.totalDraws) / stats.frames
        );

        // Final snapshot (percentiles)
        auto snap = fs.snapshotAndBump(t1);
        std::println("{}", snap); // prints verbose: avg, 1% low, 0.1% low, p99, etc.

        std::println("");

        return stats;
    }

    // ----------------- ASYNC bench -----------------
    static BenchStats runAsync(u32 frames, unsigned int threads)
    {
        std::println("=== ChunkManager Bench: ASYNC ===");

        CameraFirstPerson cam;
        setupCamera(cam);

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ 32 },
            DummyChunkManager::VoxelSize{ 4 },
            DummyBuffer{},
            DummyBuffer{}
        };

        if (threads == 0) {
            auto hc = std::thread::hardware_concurrency();
            threads = (hc > 0 ? std::max(1u, hc - 1) : 1u);
        }
        utils::threading::ThreadPool pool{ threads };

        std::atomic<u32> inflight{ 0 };
        std::atomic<u64> submitted{ 0 };
        std::atomic<u64> completed{ 0 };
        std::atomic<u32> maxInflight{ 0 };

        ResultQueue resultQueue{};
        BenchStats stats{};
        FrameStats fs{};

        auto prev = FrameStats::ClockType::now();
        const auto t0 = prev;

        auto flushResults = [&]() {
            std::vector<Result> local;
            {
                std::lock_guard lock{ resultQueue.mtx };
                local.swap(resultQueue.q);  // grab all results quickly
            }
            // process outside the lock
            for (auto& r : local) {
                if (auto info = mgr.getChunkInfo(r.id)) {
                    if (info->version.load(std::memory_order_relaxed) == r.version) {
                        info->vertexCount = r.vcount;
                        info->indexCount = r.icount;
                        info->state.store(ChunkState::Clean, std::memory_order_release);
                    }
                }
            }
            };

        auto countCleanVisible = [&]() {
            u32 cleanCount = 0;
            for (auto& id : mgr.getVisibleChunks()) {
                if (auto info = mgr.getChunkInfo(id)) {
                    if (info->state.load(std::memory_order_acquire) == ChunkState::Clean) {
                        ++cleanCount;
                    }
                }
            }
            return cleanCount;
        };

        u32 minThreshold = 5;
        u32 maxThreshold = 80;
        u32 currentThreshold = 50; // baseline
        u32 framesBelow = 0;
        u32 framesAbove = 0;

        for (u32 f = 0; f < frames; ++f) {
            auto now = FrameStats::ClockType::now();
            fs.addFrame(now, prev);
            prev = now;

            randomCameraJitter(cam);

            std::atomic<u32> frameDraws = 0;

            auto gen_cb = [&](const ChunkId& cid, std::shared_ptr<ChunkInfo> info)
                {
                    submitted.fetch_add(1, std::memory_order_relaxed);
                    inflight.fetch_add(1, std::memory_order_relaxed);
                    maxInflight.store(
                        std::max(
                            maxInflight.load(std::memory_order_relaxed),
                            inflight.load(std::memory_order_relaxed)
                        ),
                        std::memory_order_relaxed
                    );

                    u64 myVer = info->version.fetch_add(1, std::memory_order_relaxed) + 1;

                    pool.submit(
                        [&, cid, myVer](std::stop_token) {
                            auto mesh = genNoise<BasicVertex<VertexPacking::Standard, Position, Normal>, IndexT>(
                                glm::vec3{ cid } *static_cast<f32>(mgr.getChunkSize()),
                                glm::vec3{ static_cast<f32>(mgr.getVoxelSize()) }
                            );
                            Result r{};
                            r.id = cid;
                            r.version = myVer;
                            r.vcount = static_cast<u32>(mesh.vertices.size());
                            r.icount = static_cast<u32>(mesh.indices.size());

                            {
                                std::lock_guard lk{ resultQueue.mtx };
                                resultQueue.q.emplace_back(std::move(r));
                            }

                            simulateUpload();

                            completed.fetch_add(1, std::memory_order_relaxed);
                            inflight.fetch_sub(1, std::memory_order_relaxed);
                        }
                    );
            };

            auto draw_cb = [&](const ChunkId&, std::shared_ptr<ChunkInfo> info) {
                (void)info;
                frameDraws.fetch_add(1);
            };

            mgr.update().generate(gen_cb);

            simulateGameWork();

            while (true) {
                flushResults();
                u32 clean = countCleanVisible();

                if (clean >= currentThreshold) {
                    framesAbove++;
                    framesBelow = 0;
                    // If we've been above for several frames, try nudging up
                    if (framesAbove > 20 && currentThreshold < maxThreshold) {
                        currentThreshold += 5;
                        framesAbove = 0;
                    }
                    break; // good enough for this frame
                }
                else {
                    framesBelow++;
                    framesAbove = 0;
                    if (framesBelow > 5 && currentThreshold > minThreshold) {
                        currentThreshold -= 5;
                        framesBelow = 0;
                    }
                    std::this_thread::yield(); // or hybrid yield/sleep
                }
            }

            mgr.draw(draw_cb);

            stats.totalDirty += static_cast<u64>(mgr.getDirtyChunks().size());
            stats.totalDraws += frameDraws;
            stats.frames++;
        }

        // wait for background tasks to finish
        while (completed.load() < submitted.load()) {
            std::this_thread::yield();
        }

        const auto t1 = FrameStats::ClockType::now();
        stats.wall = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0);
        stats.submitted = submitted.load();
        stats.completed = completed.load();
        stats.maxInflight = maxInflight.load();

        // Report
        const f64 sec = std::chrono::duration<f64>(stats.wall).count();
        std::println(
            "Frames: {}  time: {:.3f}s  FPS: {:.1f}",
            stats.frames,
            sec,
            stats.frames / sec
        );
        std::println(
            "Avg dirty/frame: {:.2f}  Avg draws/frame: {:.2f}",
            static_cast<f64>(stats.totalDirty) / stats.frames,
            static_cast<f64>(stats.totalDraws) / stats.frames
        );
        std::println(
            "Submitted: {}  Completed: {}  Max inflight: {}  Threads: {}",
            stats.submitted,
            stats.completed,
            stats.maxInflight,
            threads
        );

        // Final frame stats snapshot (over entire run)
        auto snap = fs.snapshotAndBump(t1);
        std::println("{}", snap);

        std::println("");
        return stats;
    }

    // ----------------- Public entry -----------------
    void run_chunk_manager_benchmarks(u32 frames, unsigned int threads)
    {
        std::println("============================================");
        std::println("      ChunkManager Sync/Async Bench");
        std::println("============================================");

        auto s = runSync(frames);
        auto a = runAsync(frames, threads);

        auto s_sec = std::chrono::duration<f64>(s.wall).count();
        auto a_sec = std::chrono::duration<f64>(a.wall).count();

        std::println("--------------- Summary --------------------");
        std::println(
            "SYNC : {:.1f} FPS, avg draws/frame {:.2f}",
            s.frames / s_sec,
            static_cast<f64>(s.totalDraws) / s.frames
        );
        std::println(
            "ASYNC: {:.1f} FPS, avg draws/frame {:.2f} (submitted={}, completed={}, maxInflight={})",
            a.frames / a_sec,
            static_cast<f64>(a.totalDraws) / a.frames,
            a.submitted,
            a.completed,
            a.maxInflight
        );
        std::println("============================================\n");
    }

} // namespace marching_cubes::tests
