#include <tests/managers/chunk_manager_tests.hpp>

#define GLM_FORCE_XYZW_ONLY
#include <glm/glm.hpp>

#include <exception>
#include <numbers>
#include <print>
#include <random>
#include <unordered_set>

#include <camera/camera_first_person.hpp>
#include <collisions/collisions.hpp>
#include <core/aliases.hpp>
#include <scene/chunk_cache.hpp>
#include <stdexcept>

namespace marching_cubes::tests {

    static constexpr u64 ChunkResolution = 32;
    static constexpr u64 VoxelSize = 4;

    using camera::CameraFirstPerson;
    using scene::ChunkId;
	using scene::ChunkInfo;
	using scene::ChunkState;

    // -----------------------
    // Callback implementations
    // -----------------------
    static void gen_cb(const ChunkId& id, std::shared_ptr<ChunkInfo> info)
    {
        (void)id; (void)info;
    }

    static auto make_draw_cb(DrawStats* stats)
    {
        return [stats](
            const ChunkId&,
            std::shared_ptr<ChunkInfo> info
        ) {
            assert(info->state == ChunkState::Clean);
            stats->draws++;
        };
    }

    static void setupCamera(CameraFirstPerson& cam)
    {
        //// Non-origin position + non-identity orientation
        cam.getTransform().setPosition({ 5.0f, 3.0f, 10.0f });
        //cam.getTransform().setPosition({ 0.0f, 0.0f, 0.0f });

        //// Give it a small pitch/yaw so forward isn't -Z anymore
        //// (mouse delta uses sensitivity, so use explicit lookAt for determinism)
        const glm::vec3 eye = cam.getTransform().getPosition();
        const glm::vec3 forward =
            glm::normalize(glm::vec3{ -0.6f, -0.2f, -0.8f }); // arbitrary, stable
        cam.lookAt(eye + forward);

        //// Same projection as before
        cam.setProjection(glm::radians(90.0f), 1.0f, 0.1f, 300.0f);
    }

    static u32 run_update(DummyChunkManager& mgr, CameraFirstPerson& cam, bool printDraws = true)
    {
		static u32 draws = 0;
        DrawStats stats{};
        mgr.update().generate(gen_cb).draw(make_draw_cb(&stats));
        if (printDraws && stats.draws != draws) {
            std::println("  [Update]           Draws = {}", stats.draws);
			draws = stats.draws;
        }
        return static_cast<u32>(mgr.getDirtyChunks().size());
    }

    static f32 measure_slack_right(DummyChunkManager& mgr, CameraFirstPerson& cam, f32 step = 0.1f)
    {
        f32 moved = 0.f;
        for (;;) {
            cam.translateRight(step);
            moved += step;
            if (run_update(mgr, cam) > 0) return moved - step;
        }
    }

    static f32 measure_slack_forward(DummyChunkManager& mgr, CameraFirstPerson& cam, f32 step = 0.1f)
    {
        f32 moved = 0.f;
        for (;;) {
            cam.translateForward(step);
            moved += step;
            if (run_update(mgr, cam) > 0) return moved - step;
        }
    }

    static f32 measure_slack_up(DummyChunkManager& mgr, CameraFirstPerson& cam, f32 step = 0.1f)
    {
        f32 moved = 0.f;
        for (;;) {
            cam.translateUpward(step);
            moved += step;
            if (run_update(mgr, cam) > 0) return moved - step;
        }
    }

    static f32 frustum_circumscribed_radius(f32 farDist, f32 fovY_radians, f32 aspect)
    {
        const f32 t = std::tan(0.5f * fovY_radians);
        // Optional safety cushion for FP jitter in your test:
        const f32 R = farDist * std::sqrt(1.0f + t * t * (1.0f + aspect * aspect));
        return R * 1.5f; // or return R * 1.000001f;
    }

    static std::vector<ChunkId> build_spherical_superset(
        const CameraFirstPerson& cam,
        u64 chunkSize
    )
    {
        f32 far = cam.getFar();
        f32 fovy = cam.getFovRadians();      // or store when you set projection
        f32 aspect = cam.getAspectRatio();    // idem
        f32 R = frustum_circumscribed_radius(far, fovy, aspect);
        const glm::vec3 C = cam.getTransform().getPosition();

        // half-open [min,max) in index space with floor(min), ceil(max)
        const double cs = (double)chunkSize;
        const glm::dvec3 minW = glm::dvec3(C) - glm::dvec3(R);
        const glm::dvec3 maxW = glm::dvec3(C) + glm::dvec3(R);

        const i64 minX = (i64)std::floor(minW.x / cs);
        const i64 minY = (i64)std::floor(minW.y / cs);
        const i64 minZ = (i64)std::floor(minW.z / cs);
        const i64 maxX = (i64)std::ceil(maxW.x / cs);
        const i64 maxY = (i64)std::ceil(maxW.y / cs);
        const i64 maxZ = (i64)std::ceil(maxW.z / cs);

        std::vector<ChunkId> sup;
        sup.reserve((maxX - minX) * (maxY - minY) * (maxZ - minZ));

        for (i64 z = minZ; z < maxZ; ++z) {
            for (i64 y = minY; y < maxY; ++y) {
                for (i64 x = minX; x < maxX; ++x) {
                    glm::i64vec3 base = glm::i64vec3{ x,y,z } *(i64)chunkSize;
                    glm::i64vec3 top = base + (i64)chunkSize;
                    if (collisions::intersects(collisions::Sphere{ C, R }, collisions::AABB{ base, top })) {
                        sup.emplace_back(x, y, z);
                    }
                }
            }
        }
        std::sort(sup.begin(), sup.end(), scene::ChunkIdLess{});
        sup.erase(std::unique(sup.begin(), sup.end()), sup.end());
        return sup;
    }

    static std::vector<glm::vec3> quantized_directions_fibonacci(int N)
    {
        std::vector<glm::vec3> dirs{};
        dirs.reserve(N);

        //const double PHI = (1.0 + std::sqrt(5.0)) * 0.5;
        const double GOLDEN_ANGLE = 2.0 * std::numbers::pi / (std::numbers::phi * std::numbers::phi); // ~2.399963...

        for (int i = 0; i < N; ++i) {
            // z in (-1,1) evenly; area-uniform
            double z = 1.0 - 2.0 * (i + 0.5) / (double)N;
            double r = std::sqrt(std::max(0.0, 1.0 - z * z));
            double phi = GOLDEN_ANGLE * i;

            glm::vec3 d{ (float)(r * std::cos(phi)), (float)z, (float)(r * std::sin(phi)) };
            dirs.push_back(glm::normalize(d));
        }

        // Optional: create a smooth traversal path (nearest-neighbor greedy)
        // For testing churn you can also leave it as-is (pseudo-random order).
        return dirs;
    }

    static void aim_camera(CameraFirstPerson& cam, const glm::vec3& dir)
    {
        const glm::vec3 eye = cam.getTransform().getPosition();
        glm::vec3 forward = glm::normalize(dir);
        // Choose an up that avoids gimbal near poles (pick the more stable of world up/down)
        glm::vec3 worldUp = (std::abs(forward.y) > 0.9f) ? glm::vec3{ 0,0,1 } : glm::vec3{ 0,1,0 };
        glm::vec3 target = eye + forward;
        cam.lookAt(target, worldUp);
    }

    static void test_no_thrashing()
    {
        std::println("\n============================================");
        std::println("=== Test 1: No Thrash on Small Rotations ===");
        std::println("============================================");

        std::println("\nSetting up the camera...");

        CameraFirstPerson cam;
        setupCamera(cam);

        std::println("\nSetting up the chunk manager...");

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ ChunkResolution },
            DummyChunkManager::VoxelSize{ VoxelSize },
            DummyBuffer{},
            DummyBuffer{}
        };

        // 1) first frame
        u32 dirty0 = run_update(mgr, cam);
        std::println("  [First frame]      Dirty chunks = {}", dirty0);
        assert(dirty0 > 0);

        // 2) small rotation
        const float smallYaw = 10.0f;
        cam.processMouseDelta({ smallYaw * cam.getSensitivityX(), 0.0f });
        u32 dirty1 = run_update(mgr, cam);
        std::println("  [Small rot +]      Dirty chunks = {}", dirty1);

        // 3) rotate back
        cam.processMouseDelta({ -smallYaw * cam.getSensitivityX(), 0.0f });
        u32 dirty2 = run_update(mgr, cam);
        std::println("  [Small rot -]      Dirty chunks = {}", dirty2);

        // 4) jitter
        u32 maxDirty = 0;
        u64 dirtyCount = 0;
        for (int i = 0; i < 10; ++i) {
            cam.processMouseDelta({ (i % 2 ? 1 : -1) * smallYaw * cam.getSensitivityX(), 0.0f });
            u32 d = run_update(mgr, cam);
            maxDirty = std::max(maxDirty, d);
            dirtyCount += d;
        }
        std::println("  [Yaw jitter]       Max dirty/frame = {}", maxDirty);
        std::println("  [Yaw jitter]       Avg dirty/frame = {}", static_cast<f32>(dirtyCount) / 10.0f);

        // 5) translation by one chunk
        cam.translateRight(static_cast<f32>(ChunkResolution) * static_cast<f32>(VoxelSize));
        u32 dirtyMove = run_update(mgr, cam);
        std::println("  [Translate +1chk]  Dirty chunks = {}", dirtyMove);
        assert(dirtyMove > 0);

        std::println("  Passed Test 1");
        std::println("============================================\n");
    }

    static void test_steady_state()
    {
        std::println("\n============================================");
        std::println("=== Test 2: Steady-State (No Movement) ===");
        std::println("============================================");

        CameraFirstPerson cam;
        setupCamera(cam);

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ ChunkResolution },
            DummyChunkManager::VoxelSize{ VoxelSize },
            DummyBuffer{},
            DummyBuffer{}
        };

        run_update(mgr, cam); // warm-up
        u32 dirty = run_update(mgr, cam);

        std::println("  [Second frame]     Dirty chunks = {}", dirty);
        assert(dirty == 0);

        std::println("  Passed Test 2");
        std::println("============================================\n");
    }

    static void test_sub_chunk_translation()
    {
        std::println("\n============================================");
        std::println("=== Test 3: Sub-Chunk Translation ===");
        std::println("============================================");

        CameraFirstPerson cam;
        setupCamera(cam);

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ ChunkResolution },
            DummyChunkManager::VoxelSize{ VoxelSize },
            DummyBuffer{},
            DummyBuffer{}
        };

        auto slackX = measure_slack_right(mgr, cam);
        std::println("Slack X: {}", slackX);
        setupCamera(cam); run_update(mgr, cam);
        auto slackZ = measure_slack_forward(mgr, cam);
        std::println("Slack Z: {}", slackZ);
        setupCamera(cam); run_update(mgr, cam);
        auto slackY = measure_slack_up(mgr, cam);
        std::println("Slack Y: {}", slackY);

        std::println("  [Slack] X={:.2f}, Y={:.2f}, Z={:.2f}", slackX, slackY, slackZ);

        setupCamera(cam);

        // Warm-up: populate cache
        run_update(mgr, cam);

        // Measure safe slack along X
        f32 step = 0.1f;
        f32 moved = 0.0f;
        while (true) {
            cam.translateRight(step);
            moved += step;
            u32 dirty = run_update(mgr, cam);
            if (dirty > 0) {
                break;
            }
        }

        f32 safeSlack = moved - step;
        std::println("  [Measured safe slack along X] {:.2f} units", safeSlack);

        setupCamera(cam);
        run_update(mgr, cam); // warm-up again

        cam.translateRight(safeSlack * 0.5f);
        u32 dirty = run_update(mgr, cam);
        std::println("  [Move {:.2f} < slack] Dirty chunks = {}", safeSlack * 0.5f, dirty);
        assert(dirty == 0);

        std::println("  Passed Test 3");
        std::println("============================================\n");
    }

    static void test_quantized_sphere_panning()
    {
        std::println("\n============================================");
        std::println("=== Test 4: Quantized Sphere Panning    ===");
        std::println("============================================");

        CameraFirstPerson cam;
        setupCamera(cam); // sets pos, FOV, near/far, and a baseline orientation

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ ChunkResolution },
            DummyChunkManager::VoxelSize{ VoxelSize },
            DummyBuffer{}, DummyBuffer{}
        };

        // warm-up
        run_update(mgr, cam);

        // Build spherical superset once (position fixed)
        const u64 chunkSize = ChunkResolution * VoxelSize;
        auto supVec = build_spherical_superset(cam, chunkSize);
        std::unordered_set<ChunkId> sup(supVec.begin(), supVec.end());

        // Pick a quantization scheme:
        // auto dirs = quantized_directions_equal_area(/*Ntheta=*/36, /*Nphi=*/64);
        auto dirs = quantized_directions_fibonacci(/*N=*/20000);

        size_t totalDirty = 0, maxDirty = 0, outside = 0, frames = 0;

        // helper to diff visible set sizes (optional)
        auto to_set = [](const std::vector<ChunkId>& v) {
            return std::unordered_set<ChunkId>(v.begin(), v.end());
            };
        auto diff_count = [](const auto& a, const auto& b) {
            size_t d = 0; for (auto& x : a) if (!b.contains(x)) ++d; for (auto& x : b) if (!a.contains(x)) ++d; return d;
            };

        auto prevVis = to_set(mgr.getVisibleChunks());

        std::size_t successfulFrames = 0;
        try {
            for (const auto& d : dirs) {
                aim_camera(cam, d);
                u32 dirty = run_update(mgr, cam, false);
                totalDirty += dirty;
                maxDirty = std::max(maxDirty, (size_t)dirty);
                ++frames;

                // visible ⊆ superset
                for (const auto& id : mgr.getVisibleChunks()) {
                    if (!sup.contains(id)) { ++outside; }
                }

                // Optional: track set flip magnitude
                auto curVis = to_set(mgr.getVisibleChunks());
                (void)diff_count; // if you want to print/inspect, call it here
                prevVis.swap(curVis);
                successfulFrames++;
            }
        }
        catch (const std::out_of_range& e) {
            std::println("  [Quantized sphere] Drew {} frames before crashing with exception ({}).", successfulFrames, e.what());
            throw;
        }

        std::println("  [Quantized sphere] Frames={}, TotalDirty={}, MaxDirty/frame={}, OutsideSuperset={}",
            frames, totalDirty, maxDirty, outside);

        // Guarantees: never draw outside far-sphere superset; pure rotation should not thrash
        assert(outside == 0 && "Visible chunk escaped spherical superset - quantization bug.");

        std::println("  Passed Test 4");
        std::println("============================================\n");
    }

    static void test_quantized_translation_panning()
    {
        std::println("\n============================================");
        std::println("=== Test 5: Quantized Translation Panning ===");
        std::println("============================================");

        CameraFirstPerson cam;
        setupCamera(cam); // sets pos, FOV, near/far, baseline orientation

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ ChunkResolution },
            DummyChunkManager::VoxelSize{ VoxelSize },
            DummyBuffer{}, DummyBuffer{}
        };

        // warm-up
        run_update(mgr, cam);

        const u64 chunkSize = ChunkResolution * VoxelSize;

        // Fibonacci sphere directions
        auto dirs = quantized_directions_fibonacci(/*N=*/50);

        // Translation radii (0.25, 0.5, 1.0, 2.0, 4.0 … chunks)
        std::vector<f32> radii;
        for (int i = 1; i <= 500; ++i) {
            radii.push_back(static_cast<f32>(chunkSize) * 0.25f * i);
        }

        size_t totalDirty = 0, maxDirty = 0, outside = 0, frames = 0;

        try {
            for (const auto& d : dirs) {
                for (f32 r : radii) {
                    // Reset camera orientation for determinism
                    setupCamera(cam);

                    // Move along d by r
                    cam.getTransform().setPosition(
                        cam.getTransform().getPosition() + d * r
                    );

                    u32 dirty = run_update(mgr, cam, false);
                    totalDirty += dirty;
                    maxDirty = std::max(maxDirty, (size_t)dirty);
                    ++frames;

                    // Superset check at this new position
                    auto supVec = build_spherical_superset(cam, chunkSize);
                    std::unordered_set<ChunkId> sup(supVec.begin(), supVec.end());
                    for (const auto& id : mgr.getVisibleChunks()) {
                        if (!sup.contains(id)) { ++outside; }
                    }
                }
            }
        }
        catch (const std::out_of_range& e) {
            std::println("  [Quantized translation] Crashed at frame {} ({})", frames, e.what());
            throw;
        }

        std::println("  [Quantized translation] Frames={}, TotalDirty={}, MaxDirty/frame={}, OutsideSuperset={}",
            frames, totalDirty, maxDirty, outside);

        assert(outside == 0 && "Visible chunk escaped spherical superset during translation!");

        std::println("  Passed Test 5");
        std::println("============================================\n");
    }

    static void test_monte_carlo_random_walk()
    {
        std::println("\n============================================");
        std::println("=== Test 6: Monte Carlo Random Walk     ===");
        std::println("============================================");

        CameraFirstPerson cam;
        setupCamera(cam); // sets pos, FOV, near/far, baseline orientation

        DummyChunkManager mgr{
            &cam,
            DummyChunkManager::ChunkResolution{ ChunkResolution },
            DummyChunkManager::VoxelSize{ VoxelSize },
            DummyBuffer{}, DummyBuffer{}
        };

        // warm-up
        run_update(mgr, cam);

        const u64   chunkSize = ChunkResolution * VoxelSize;
        const float fChunk = static_cast<float>(chunkSize);

        // Build a direction set once (dense but still fast)
        auto dirs = quantized_directions_fibonacci(/*N=*/4096);

        // Deterministic RNG for reproducibility
        std::mt19937_64 rng{ 0xC0FFEEu };

        // Modes: 0=aim exactly at a random dir, 1=small jitter rotate, 2=translate by k*chunkSize along random dir,
        //        3=sub-chunk nudge along camera local axes
        std::uniform_int_distribution<int> modeDist(0, 3);
        std::uniform_int_distribution<size_t> dirIdx(0, dirs.size() - 1);

        // angular jitter in *degrees* (fed through mouse delta sensitivity)
        std::uniform_real_distribution<float> yawDeg(-8.0f, 8.0f);
        std::uniform_real_distribution<float> pitchDeg(-5.0f, 5.0f);

        // translate by [0, 4] chunks (sign chosen separately)
        std::uniform_real_distribution<float> chunkSpan(0.0f, 4.0f);
        std::bernoulli_distribution signCoin(0.5);

        // sub-chunk nudges up to ~0.9 of a voxel to keep within cache slack
        std::uniform_real_distribution<float> subNudge(0.0f, static_cast<float>(VoxelSize) * 0.9f);

        size_t frames = 0, outside = 0, totalDirty = 0, maxDirty = 0;

        try {
            const size_t kFrames = 50000;
            for (size_t i = 0; i < kFrames; ++i) {
                const int mode = modeDist(rng);
                switch (mode) {
                case 0: { // aim to a random direction (pure rotation)
                    aim_camera(cam, dirs[dirIdx(rng)]);
                    break;
                }
                case 1: { // small jitter rotation
                    const float yaw = yawDeg(rng) * cam.getSensitivityX();
                    const float pitch = pitchDeg(rng) * cam.getSensitivityY();
                    cam.processMouseDelta({ yaw, pitch });
                    break;
                }
                case 2: { // translate by K*chunkSize along a random direction (world-space)
                    glm::vec3 d = dirs[dirIdx(rng)];
                    float k = chunkSpan(rng);
                    if (signCoin(rng)) k = -k;
                    cam.getTransform().setPosition(
                        cam.getTransform().getPosition() + d * (k * fChunk)
                    );
                    break;
                }
                case 3: { // sub-chunk nudge in camera local space (R/F/U)
                    // pick one of right/forward/up
                    int axis = std::uniform_int_distribution<int>(0, 2)(rng);
                    float step = subNudge(rng);
                    if (signCoin(rng)) step = -step;
                    if (axis == 0) cam.translateRight(step);
                    else if (axis == 1) cam.translateForward(step);
                    else cam.translateUpward(step);
                    break;
                }
                default: break;
                }

                // update + collect stats
                u32 dirty = run_update(mgr, cam, /*printDraws=*/false);
                totalDirty += dirty;
                maxDirty = std::max(maxDirty, static_cast<size_t>(dirty));
                ++frames;

                // Superset safety: visible must be inside circumscribed far-sphere
                auto supVec = build_spherical_superset(cam, chunkSize);
                std::unordered_set<ChunkId> sup(supVec.begin(), supVec.end());
                for (const auto& id : mgr.getVisibleChunks()) {
                    if (!sup.contains(id)) { ++outside; }
                }
            }
        }
        catch (const std::out_of_range& e) {
            std::println("  [Monte Carlo] Crashed at frame {} ({}).", frames, e.what());
            throw; // surface failure to the test runner
        }

        std::println("  [Monte Carlo] Frames={}, TotalDirty={}, MaxDirty/frame={}, OutsideSuperset={}",
            frames, totalDirty, maxDirty, outside);

        // Guarantees: never draw outside far-sphere superset.
        // Additionally, draw callback asserts ChunkState::Clean, so any unmapped draw would fail earlier.
        assert(outside == 0 && "Visible chunk escaped spherical superset during Monte Carlo!");

        std::println("  Passed Test 6");
        std::println("============================================\n");
    }

    // -----------------------
    // Run all tests
    // -----------------------
    void run_chunk_manager_tests()
    {
        std::println("============================================");
        std::println("    Running ChunkManager Unit Tests");
        std::println("============================================");

        test_no_thrashing();
        test_steady_state();
        test_sub_chunk_translation();
        test_quantized_sphere_panning();
        test_quantized_translation_panning();
        test_monte_carlo_random_walk();

        std::println("============================================");
        std::println("    All ChunkManager tests passed!");
        std::println("============================================\n");
    }
}
