#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_XYZW_ONLY
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/noise.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <vulkan/vulkan_core.h>

#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <print>
#include <set>
#include <span>
#include <stdexcept>
#include <stop_token>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include <camera/camera.hpp>
#include <camera/camera_first_person.hpp>

#include <controllers/input_controller.hpp>
#include <controllers/camera_controller.hpp>

#include <collisions/collisions.hpp>
#include <core/aliases.hpp>
#include <core/device.hpp>
#include <core/frame.hpp>
#include <core/framebuffer.hpp>
#include <core/image_view.hpp>
#include <core/jobs.hpp>
#include <core/job_manager.hpp>
#include <core/wrapper.hpp>
#include <core/physical_device.hpp>
#include <core/pipeline.hpp>
#include <core/swapchain.hpp>
#include <core/builders/graphics_pipeline_builder.hpp>
#include <core/builders/render_pass_builder.hpp>
#include <core/helpers/command_buffer.hpp>
#include <core/helpers/command_pool.hpp>
#include <core/helpers/descriptor.hpp>
#include <core/helpers/fence.hpp>
#include <core/helpers/sampler.hpp>
#include <core/helpers/semaphore.hpp>

#include <managers/chunk_manager.hpp>

#include <profiling/chunk_manager.hpp>

#include <resources/image_resource.hpp>
#include <resources/object_buffer.hpp>
#include <resources/staging_buffer.hpp>

#include <scene/chunk_cache.hpp>
#include <scene/ubo.hpp>
#include <scene/vertex.hpp>

#include <tests/managers/chunk_manager_tests.hpp>
#include <tests/utils/thread_pool_tests.hpp>

#include <utils/frame_stats.hpp>
#include <utils/marching_cubes.hpp>
#include <utils/render_conventions.hpp>
#include <utils/samplers.hpp>
#include <utils/thread_pool.hpp>
#include <utils/utils.hpp>
#include <window/window.hpp>

using namespace marching_cubes;
using namespace marching_cubes::window;

using camera::Camera;
using camera::CameraFirstPerson;
using collisions::AABB;
using collisions::Sphere;
using collisions::intersects;
using controllers::CameraController;
using controllers::InputController;
using core::Device;
using core::Frame;
using core::FramebufferSet;
using core::ImageView;
using core::OwningWrapper;
using core::PhysicalDevice;
using core::Pipeline;
using core::Swapchain;
using core::builders::GraphicsPipelineBuilder;
using core::builders::RenderPassBuilder;
using core::jobs::BufferTransferJob;
using core::jobs::ImageBlitJob;
using core::jobs::ImageLayoutTransitionJob;
using core::jobs::JobManager;
using managers::BasicChunkManager;
using marching_cubes::utils::marching_cubes::marchingCubes;
using resources::ImageResource;
using resources::StagedObjectBuffer;
using resources::StagingBuffer;
using scene::ChunkId;
using scene::ChunkInfo;
using scene::VertexPacking;
using utils::frame_stats::FrameStats;
using utils::render_conventions::RenderConventions;
using utils::threading::ThreadPool;

using ShapeVertexType = scene::BasicVertex<VertexPacking::Tight, scene::Position, scene::Normal>;
using LineVertexType = scene::BasicVertex<VertexPacking::Standard, scene::Position>;

using IndexT = RenderConventions::IndexTypeT<u32>;

struct ChunkResult {
    ChunkId id;
    u64 version;
    std::vector<ShapeVertexType> vertices;
    std::vector<IndexT> indices;
    VkDeviceSize vertexOffset;
    VkDeviceSize indexOffset;
};

struct ResultQueue {
    std::mutex mtx;
    std::vector<ChunkResult> q;
};

static LineVertexType makeV(
    const glm::vec3& p,
    const glm::vec3& rgb
)
{
    return LineVertexType{
        p,
    };
}

static void pushLine(
    std::vector<LineVertexType>& out,
    const glm::vec3& a,
    const glm::vec3& b,
    const glm::vec3& rgb
)
{
    out.push_back(makeV(a, rgb));
    out.push_back(makeV(b, rgb));
}

static void pushBoxWire(
    std::vector<LineVertexType>& out,
    const glm::vec3& mn,
    const glm::vec3& mx,
    const glm::vec3& rgb
)
{
    const glm::vec3 A{ mn.x,mn.y,mn.z }, B{ mx.x,mn.y,mn.z }, C{ mx.x,mx.y,mn.z }, D{ mn.x,mx.y,mn.z };
    const glm::vec3 E{ mn.x,mn.y,mx.z }, F{ mx.x,mn.y,mx.z }, G{ mx.x,mx.y,mx.z }, H{ mn.x,mx.y,mx.z };
    // bottom
    pushLine(out, A, B, rgb); pushLine(out, B, C, rgb); pushLine(out, C, D, rgb); pushLine(out, D, A, rgb);
    // top
    pushLine(out, E, F, rgb); pushLine(out, F, G, rgb); pushLine(out, G, H, rgb); pushLine(out, H, E, rgb);
    // verticals
    pushLine(out, A, E, rgb); pushLine(out, B, F, rgb); pushLine(out, C, G, rgb); pushLine(out, D, H, rgb);
}

static void buildAxes(
    std::vector<LineVertexType>& out,
    f32 L = 10.0f
)
{
    pushLine(out, { 0,0,0 }, { L,0,0 }, { 1,0,0 });  // X
    pushLine(out, { 0,0,0 }, { 0,L,0 }, { 0,1,0 });  // Y
    pushLine(out, { 0,0,0 }, { 0,0,L }, { 0,0,1 });  // Z
}

static void buildVisibleChunkBoxes(
    std::vector<LineVertexType>& out,
    const std::vector<ChunkId>& visible,
    f32 chunkSize,
    const glm::vec3& camPos,
    f32 maxDist = 200.0f,
    u64 step = 1,
    const glm::vec3& color = glm::vec3{ 0.2f, 0.8f, 1.0f }
)
{
    auto mod = [](i64 v, i64 s) {
        i64 m = v % s;
        return (m < 0) ? m + s : m;
    };
    for (auto& c : visible) {
        /*if (mod(c.x, step) || mod(c.y, step) || mod(c.z, step)) {
            continue;
        }*/
        glm::vec3 mn = glm::vec3{ c } *chunkSize;
        glm::vec3 center = mn + 0.5f * glm::vec3{ chunkSize };
        if (glm::length2(center - camPos) > maxDist * maxDist) {
            continue;
        }
        glm::vec3 mx = mn + (glm::vec3{ chunkSize });
        pushBoxWire(out, mn, mx, color);
    }
}

class App final {
public:

    static constexpr int kWindowWidth = 800;
    static constexpr int kWindowHeight = 600;
	static constexpr std::size_t kMaxFramesInFlight = 2;
    static constexpr u64 kChunkResolution = 32;
    static constexpr u64 kVoxelSize = 4;

    using ChunkManagerType = BasicChunkManager<
        ShapeVertexType,
        StagedObjectBuffer<ShapeVertexType>,
        IndexT,
        StagedObjectBuffer<IndexT>
    >;

    App()
    {
        init();
    }

    void uploadChunks()
    {

        std::vector<VkBufferCopy> vCopies{};
        std::vector<VkBufferCopy> iCopies{};

        // 1) Accumulate all mesh data into CPU-side vectors first
        std::vector<ShapeVertexType> allVertices{};
        std::vector<IndexT> allIndices{};

        std::vector<ChunkResult> toMark{};    // which chunks are in this batch

        // Drain results from worker threads
        std::vector<ChunkResult> local{};
        {
            std::lock_guard lk{ m_ResultQueue.mtx };
            local.swap(m_ResultQueue.q); // grab all results quickly
        }

        vCopies.reserve(local.size());
        iCopies.reserve(local.size());
        allVertices.reserve(local.size());
        allIndices.reserve(local.size());
        toMark.reserve(local.size());
        for (auto& r : local) {
            if (auto info = m_ChunkManager.peekChunkInfo(r.id); info != nullptr
                //&& info->state.load(std::memory_order_acquire) == scene::ChunkState::Generated
                && info->version.load(std::memory_order_acquire) == r.version)
            {
                // ---- pack into the big vectors ----
                VkDeviceSize vOffsetSrc = allVertices.size() * sizeof(ShapeVertexType);
                allVertices.insert(allVertices.end(), r.vertices.begin(), r.vertices.end());
                vCopies.push_back(
                    VkBufferCopy{
                        .srcOffset = vOffsetSrc,
                        .dstOffset = r.vertexOffset,
                        .size = r.vertices.size() * sizeof(ShapeVertexType)
                    }
                );

                VkDeviceSize iOffsetSrc = allIndices.size() * sizeof(IndexT);
                allIndices.insert(allIndices.end(), r.indices.begin(), r.indices.end());
                iCopies.push_back(
                    VkBufferCopy{
                        .srcOffset = iOffsetSrc,
                        .dstOffset = r.indexOffset,
                        .size = r.indices.size() * sizeof(IndexT)
                    }
                );

                toMark.emplace_back(std::move(r));
            }
        }

        // 2) Do the upload once (if we drained any results)
        if (!vCopies.empty() || !iCopies.empty()) {
            auto& vb = m_ChunkManager.getVertexBuffer().getBuffer();
            auto& ib = m_ChunkManager.getIndexBuffer().getBuffer();

            // allocate *one* staging buffer for verts + indices
            StagingBuffer<ShapeVertexType> vStage(m_Device, m_PhysicalDevice, allVertices);
            StagingBuffer<IndexT> iStage(m_Device, m_PhysicalDevice, allIndices);

            m_JobManager.enqueue(BufferTransferJob{ vStage.getBuffer(), vb, vCopies });
            m_JobManager.enqueue(BufferTransferJob{ iStage.getBuffer(), ib, iCopies });
            m_JobManager.flush();

            for (const auto& r : toMark) {
                if (auto info = m_ChunkManager.peekChunkInfo(r.id);
                    info && info->version.load(std::memory_order_acquire) == r.version)
                {
                    // set counts first, then Clean
                    info->vertexCount = static_cast<u32>(r.vertices.size());
                    info->indexCount = static_cast<u32>(r.indices.size());
                    info->state.store(scene::ChunkState::Clean, std::memory_order_release);
                }
            }
        }
    }

    void updateChunkManager()
    {
        // Submit new jobs for dirty chunks
        m_ChunkManager
            .update()
            .generate(
                [&](const ChunkId& cid, std::shared_ptr<ChunkInfo> info)
                {
                    const auto chunkSize = m_ChunkManager.getChunkSize();
                    const glm::vec3 chunkMin = glm::vec3{ cid } *static_cast<f32>(chunkSize);
                    const glm::u64vec3 gridSize{ chunkSize + 1, chunkSize + 1, chunkSize + 1 };

                    //const auto sampler = utils::samplers::Torus{ { 0.0f, 0.0f, 0.0f }, 100.0f, 10.0f };
                    /*const auto sampler = utils::samplers::Sphere{
                        { chunkSize * 0.0f, chunkSize * 0.0f, chunkSize * 0.0f },
                        chunkSize * 0.0f + 1.0f
                    };*/

                    const auto sampler = utils::samplers::TerrainNoiseSampler{};

                    u64 myVer = info->version.fetch_add(1, std::memory_order_acq_rel) + 1;

                    // push work to thread pool
                    m_ThreadPool.submit(
                        [=, this](std::stop_token st) {
                            auto mesh = marchingCubes<ShapeVertexType, IndexT>(
                                sampler,
                                glm::vec3{ gridSize },
                                /*iso*/ 0.0f,
                                chunkMin,
                                glm::vec3{ static_cast<f32>(m_ChunkManager.getVoxelSize()) },
                                st
                            );

                            if (info->state.load(std::memory_order_acquire) == scene::ChunkState::Evicted) {
                                // This chunk was evicted while we were working -> skip
                                return;
                            }

                            if (mesh.vertices.empty() || mesh.indices.empty()) {
                                return;
                            }

                            info->state.store(scene::ChunkState::Generated, std::memory_order_release);

                            ChunkResult result{};
                            result.id = cid;
							result.version = myVer;
                            result.vertices = std::move(mesh.vertices);
                            result.indices = std::move(mesh.indices);
                            result.vertexOffset = info->vertexOffset;
                            result.indexOffset = info->indexOffset;

                            {
                                std::lock_guard lk{ m_ResultQueue.mtx };
                                m_ResultQueue.q.emplace_back(std::move(result));
                            }
                        }
                    );
                }
            );
    }

    void updateGizmos()
    {
        m_GizmoVertices.clear();
        buildAxes(m_GizmoVertices, /*L*/ 50.0f);

        const auto& visible = m_ChunkManager.getVisibleChunks();
        const f32 cs = static_cast<f32>(m_ChunkManager.getChunkSize());
        buildVisibleChunkBoxes(m_GizmoVertices, visible, cs, m_Camera.getPosition());

        // Upload: staging copy into m_GizmoVB
        StagingBuffer stage{ m_Device, m_PhysicalDevice, m_GizmoVertices };
        if (stage.getSize() > m_GizmoVB.getSize()) {
            m_GizmoVB.updateSize(stage.getSize()); // grow if needed
        }
        m_JobManager
            .enqueue(
                BufferTransferJob{
                    stage.getBuffer(),
                    m_GizmoVB.getBuffer(),
                    std::vector{
                        VkBufferCopy{
                            .srcOffset = 0,
                            .dstOffset = 0,
                            .size = stage.getSize()
                        }
                    }
                }
            )
            .flush();
    }

    void drawGizmos(VkCommandBuffer cmd, Pipeline& linePipe, Frame& frame)
    {
        if (m_GizmoVertices.empty()) {
            return;
        }

        // bind same UBO (uses your camera matrices)
        const auto ds = frame.getGizmoDescriptorSet();
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, linePipe);
        vkCmdBindDescriptorSets(
            cmd,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            linePipe.getPipelineLayout(),
            0,
            1,
            &ds,
            0,
            nullptr
        );

        VkDeviceSize off = 0;
        VkBuffer vb = m_GizmoVB.getBuffer();
        vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &off);

        // no index buffer; line list → vertex count is vector size
        vkCmdDraw(cmd, static_cast<u32>(m_GizmoVertices.size()), 1, 0, 0);
    }

    void run()
    {
        std::size_t frameCount = 0;
        auto currentTime = std::chrono::steady_clock::now();
        m_LastFrameTime = currentTime;
        m_LastOrigin = currentTime;
        auto lastFpsTime = currentTime;

        m_Camera.translateForward(-50.0f).translateUpward(100.0f);

        glfwMakeContextCurrent(m_Window);
        glfwShowWindow(m_Window);

        FrameStats frameStats{ std::chrono::seconds{ 1 } }; // report every 1s

        while (!m_Window.shouldClose()) {
            currentTime = std::chrono::steady_clock::now();
            m_DeltaTime = currentTime - m_LastFrameTime;

            frameStats.addFrame(currentTime, m_LastFrameTime);

            if (frameStats.ready(currentTime)) {
                auto snapshot = frameStats.snapshotAndBump(currentTime);
                std::println("{}", snapshot);
            }

            frameCount++;
            if (auto fpsDelta = std::chrono::duration<f64>(currentTime - lastFpsTime).count(); fpsDelta > 1.0) {
                frameCount = 0;
                lastFpsTime = currentTime;
            }
            if ((currentTime - m_LastOrigin) > std::chrono::years{ 3 }) {
                m_LastOrigin = currentTime;
            }
            
            m_Window.pollEvents();

            if (m_InputController.isMouseButtonDown(GLFW_MOUSE_BUTTON_RIGHT)) {
                m_InputController.setCaptureMode(InputController::CaptureMode::MouseLook);
            }
            else {
                m_InputController.setCaptureMode(InputController::CaptureMode::None);
            }
            
            m_InputController.update(m_DeltaTime.count());

            updateChunkManager();
            updateGizmos();
            uploadChunks();

            if (!drawFrame()) {
                recreateSwapchain();
                m_CurrentFrameIndex = 0;
            }

            m_LastFrameTime = currentTime;
        }
    }

	~App()
    {
        cleanup();
	}

private:

    void initInputController()
    {
        m_InputController = InputController{};
    }

    void initWindow()
    {
        m_Window = Window{
            kWindowWidth,
            kWindowHeight,
            "Vulkan",
            &m_InputController,
            std::vector{
                std::make_pair(GLFW_CLIENT_API, GLFW_NO_API),
                std::make_pair(GLFW_RESIZABLE, GLFW_TRUE),
            },
        };
    }

    void initCamera()
    {
        m_Camera = CameraFirstPerson{};
        f32 w = static_cast<f32>(kWindowWidth);
        f32 h = static_cast<f32>(kWindowHeight);
        m_Camera.setProjection(glm::radians(90.0f), w / h, 0.1f, 300.0f);
        m_CameraController = CameraController{ &m_InputController, &m_Camera };
    }

    void initTexture()
    {
        m_ImageData = getImageData(
            joinPaths("assets", "textures", "viking_room.png"),
            VK_FORMAT_R8G8B8A8_SRGB
        );

        m_Texture = ImageResource{
            m_Device,
            m_PhysicalDevice,
            VkImageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = VK_FORMAT_R8G8B8A8_SRGB,
                .extent = m_ImageData.extent,
                .mipLevels = m_ImageData.mipLevels,
                .arrayLayers = 1,
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = (
                    VK_IMAGE_USAGE_TRANSFER_DST_BIT
                    | VK_IMAGE_USAGE_SAMPLED_BIT
                    | VK_IMAGE_USAGE_TRANSFER_SRC_BIT
                ),
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            },
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ImageResource::LinearBlitCheck::Enable,
        };
        m_TextureImageView = ImageView{
            m_Device,
            VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_Texture.getImage(),
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_Texture.getImage().getCreateInfo().format,
                .subresourceRange = VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = m_Texture.getImage().getCreateInfo().mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = m_Texture.getImage().getCreateInfo().arrayLayers,
                },
            }
        };
        m_TextureSampler = OwningWrapper<VkSampler>{
            core::helpers::createSampler(
                m_Device,
                VkSamplerCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                    .magFilter = VK_FILTER_LINEAR,
                    .minFilter = VK_FILTER_LINEAR,
                    .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                    .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                    .mipLodBias = 0.0f,
                    .anisotropyEnable = VK_TRUE,
                    .maxAnisotropy = PhysicalDevice::getProperties(m_PhysicalDevice).limits.maxSamplerAnisotropy,
                    .compareEnable = VK_FALSE,
                    .compareOp = VK_COMPARE_OP_ALWAYS,
                    .minLod = 0.0f,
                    .maxLod = 1.0f,
                    .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                    .unnormalizedCoordinates = VK_FALSE,
                }
            ),
            core::deleters::VkSamplerDeleter{ m_Device }
        };
    }

    void initVulkanCore()
    {
        m_Instance = createInstance();
        m_DebugMessenger = setupDebugMessenger(m_Instance);
        m_Surface = OwningWrapper{
            createSurface(m_Instance, m_Window),
            core::deleters::VkSurfaceKHRDeleter{ m_Instance },
        };

        m_PhysicalDevice = PhysicalDevice{ pickPhysicalDevice(m_Instance, m_Surface) };
        m_Device = createLogicalDevice(m_PhysicalDevice, m_Surface);

        m_Swapchain = createSwapchain(
            m_Window,
            m_Device,
            m_PhysicalDevice,
            m_Surface
        );
    }

    void initColorDepthTargets()
    {
        m_ColorImage = ImageResource{
            m_Device,
            m_PhysicalDevice,
            VkImageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = m_Swapchain.getImageFormat(),
                .extent = VkExtent3D{
                    .width = m_Swapchain.getExtent().width,
                    .height = m_Swapchain.getExtent().height,
                    .depth = 1,
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VkImageUsageFlagBits(
                    VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT |
                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
                ),
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            },
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ImageResource::LinearBlitCheck::Enable,
        };
        m_ColorImageView = ImageView{
            m_Device,
            VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_ColorImage.getImage(),
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_ColorImage.getImage().getCreateInfo().format,
                .subresourceRange = VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0,
                    .levelCount = m_ColorImage.getImage().getCreateInfo().mipLevels,
                    .baseArrayLayer = 0,
                    .layerCount = m_ColorImage.getImage().getCreateInfo().arrayLayers,
                },
            }
        };
        m_DepthImage = ImageResource{
            m_Device,
            m_PhysicalDevice,
            VkImageCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                .imageType = VK_IMAGE_TYPE_2D,
                .format = PhysicalDevice::findDepthFormat(m_PhysicalDevice),
                .extent = VkExtent3D{
                    .width = m_Swapchain.getExtent().width,
                    .height = m_Swapchain.getExtent().height,
                    .depth = 1,
                },
                .mipLevels = 1,
                .arrayLayers = 1,
                .samples = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                .tiling = VK_IMAGE_TILING_OPTIMAL,
                .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            },
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            ImageResource::LinearBlitCheck::Disable,
        };
        m_DepthImageView = ImageView{
            m_Device,
            VkImageViewCreateInfo{
                .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                .image = m_DepthImage.getImage(),
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = m_DepthImage.getImage().getCreateInfo().format,
                .components = VkComponentMapping{
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                },
                .subresourceRange = VkImageSubresourceRange{
                    .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
            }
        };
    }

    void resetColorDepthTargets()
    {
        // Destroy old views & images (if they were constructed)
        std::destroy_at(&m_ColorImageView);
        std::destroy_at(&m_ColorImage);
        std::destroy_at(&m_DepthImageView);
        std::destroy_at(&m_DepthImage);

        // Recreate with current swapchain extent/format
        std::construct_at(
            &m_ColorImage,
            ImageResource{
                m_Device,
                m_PhysicalDevice,
                VkImageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .imageType = VK_IMAGE_TYPE_2D,
                    .format = m_Swapchain.getImageFormat(),
                    .extent = { m_Swapchain.getExtent().width, m_Swapchain.getExtent().height, 1 },
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                    .tiling = VK_IMAGE_TILING_OPTIMAL,
                    .usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                },
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                ImageResource::LinearBlitCheck::Enable
            }
        );

        std::construct_at(
            &m_ColorImageView,
            ImageView{
                m_Device,
                VkImageViewCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = m_ColorImage.getImage(),
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = m_ColorImage.getImage().getCreateInfo().format,
                    .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, m_ColorImage.getImage().getCreateInfo().mipLevels, 0, 1 },
                }
            }
        );

        std::construct_at(
            &m_DepthImage,
            ImageResource{
                m_Device,
                m_PhysicalDevice,
                VkImageCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
                    .imageType = VK_IMAGE_TYPE_2D,
                    .format = PhysicalDevice::findDepthFormat(m_PhysicalDevice),
                    .extent = { m_Swapchain.getExtent().width, m_Swapchain.getExtent().height, 1 },
                    .mipLevels = 1,
                    .arrayLayers = 1,
                    .samples = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                    .tiling = VK_IMAGE_TILING_OPTIMAL,
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                },
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                ImageResource::LinearBlitCheck::Disable
            }
        );

        std::construct_at(
            &m_DepthImageView,
            ImageView{
                m_Device,
                VkImageViewCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                    .image = m_DepthImage.getImage(),
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = m_DepthImage.getImage().getCreateInfo().format,
                    .components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                    VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY },
                    .subresourceRange = { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 },
                }
            }
        );
    }

    void initFramebuffers()
    {
        m_Framebuffers = FramebufferSet{
            m_Device,
            m_Swapchain.getRenderPass(),
            m_Swapchain.getImageViews(),
            m_ColorImageView,
            m_DepthImageView,
            m_Swapchain.getExtent()
        };
    }

    void resetFramebuffers()
    {
        std::destroy_at(&m_Framebuffers);
        std::construct_at(
            &m_Framebuffers,
            FramebufferSet{
                m_Device,
                m_Swapchain.getRenderPass(),
                m_Swapchain.getImageViews(),
                m_ColorImageView,
                m_DepthImageView,
                m_Swapchain.getExtent()
            }
        );
    }

    void initDescriptors()
    {
        m_DescriptorSetLayout = OwningWrapper<VkDescriptorSetLayout>{
            core::helpers::createDescriptorSetLayout(
                m_Device,
                std::vector{
                    VkDescriptorSetLayoutBinding{
                        .binding = 0,
                        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    },
                    VkDescriptorSetLayoutBinding{
                        .binding = 1,
                        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                        .descriptorCount = 1,
                        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
                    }
                }
            ),
            core::deleters::VkDescriptorSetLayoutDeleter{ m_Device }
        };
        m_DescriptorPool = OwningWrapper<VkDescriptorPool>{
            core::helpers::createDescriptorPool(
                m_Device,
                std::vector{
                        VkDescriptorPoolSize{
                            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            .descriptorCount = 2 * kMaxFramesInFlight
                        },
                        VkDescriptorPoolSize{
                            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            .descriptorCount = 2 * kMaxFramesInFlight
                        },
                },
                2 * kMaxFramesInFlight
            ),
            core::deleters::VkDescriptorPoolDeleter{ m_Device }
        };
    }

    void resetDescriptorPool()
    {
        std::destroy_at(&m_DescriptorPool);
        std::construct_at(
            &m_DescriptorPool,
            OwningWrapper<VkDescriptorPool>{
                core::helpers::createDescriptorPool(
                    m_Device,
                    {
                        VkDescriptorPoolSize{
                            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                            2 * kMaxFramesInFlight
                        },
                        VkDescriptorPoolSize{
                            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                            2 * kMaxFramesInFlight
                        },
                    },
                    2 * kMaxFramesInFlight
                ),
                core::deleters::VkDescriptorPoolDeleter{ m_Device }
            }
        );
    }

    void initPipelines()
    {
        m_GraphicsPipeline = createGraphicsPipeline(
            m_Device,
            m_Swapchain.getExtent(),
            m_Swapchain.getRenderPass()
        );

        m_LinePipeline = createLinePipeline(
            m_Device,
            m_Swapchain.getExtent(),
            m_Swapchain.getRenderPass()
        );
    }

    void resetPipelines()
    {
        std::destroy_at(&m_GraphicsPipeline);
        std::construct_at(
            &m_GraphicsPipeline,
            createGraphicsPipeline(
                m_Device,
                m_Swapchain.getExtent(),
                m_Swapchain.getRenderPass()
            )
        );

        std::destroy_at(&m_LinePipeline);
        std::construct_at(
            &m_LinePipeline,
            createLinePipeline(
                m_Device,
                m_Swapchain.getExtent(),
                m_Swapchain.getRenderPass()
            )
        );
    }

    void initCommandPool()
    {
        m_CommandPool = OwningWrapper<VkCommandPool>{
            core::helpers::createCommandPool(
                m_Device,
                VkCommandPoolCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
                }
            ),
            core::deleters::VkCommandPoolDeleter{ m_Device }
        };
    }

    void initTransferManager()
    {
        m_JobManager = JobManager{
            m_Device,
            core::helpers::createCommandPool(
                m_Device,
                VkCommandPoolCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                    .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
                }
            ),
            m_Device.getGraphicsQueue(),
        };
    }

    void initChunkManager()
    {
        auto vertexBufferSize = ChunkManagerType::ComputeVertexBufferSize(
            &m_Camera,
            ChunkManagerType::ChunkResolution{ kChunkResolution },
            ChunkManagerType::VoxelSize{ kVoxelSize }
		);
        auto indexBufferSize = ChunkManagerType::ComputeIndexBufferSize(
            &m_Camera,
            ChunkManagerType::ChunkResolution{ kChunkResolution },
            ChunkManagerType::VoxelSize{ kVoxelSize }
        );
        std::println(
            "Initial chunk vertex buffer size: {:L} elements, {:L} MB",
            vertexBufferSize / sizeof(ChunkManagerType::VertexType),
            vertexBufferSize / (1024ull * 1024ull)
        );
        std::println(
            "Initial chunk index buffer size: {:L} elements, {:L} MB",
            vertexBufferSize / sizeof(ChunkManagerType::IndexType),
            indexBufferSize / (1024ull * 1024ull)
        );
        m_ChunkManager = ChunkManagerType{
            &m_Camera,
            ChunkManagerType::ChunkResolution{ kChunkResolution },
            ChunkManagerType::VoxelSize{ kVoxelSize },
            ChunkManagerType::VertexBufferType{
                m_Device,
                m_PhysicalDevice,
                VkBufferCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .size = vertexBufferSize,
                    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                },
            },
            ChunkManagerType::IndexBufferType{
                m_Device,
                m_PhysicalDevice,
                VkBufferCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .size = indexBufferSize,
                    .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                },
            },
        };
    }

    void initFrames()
    {
        m_InFlightFrames = {
            createFrame(
                m_Device,
                m_PhysicalDevice,
                m_CommandPool,
                m_DescriptorPool,
                m_DescriptorSetLayout,
                m_TextureImageView,
                m_TextureSampler
            ),
            createFrame(
                m_Device,
                m_PhysicalDevice,
                m_CommandPool,
                m_DescriptorPool,
                m_DescriptorSetLayout,
                m_TextureImageView,
                m_TextureSampler
            )
        };
    }

    void resetFrames()
    {
        for (auto& frame : m_InFlightFrames) {
            std::destroy_at(&frame);
            std::construct_at(
                &frame,
                createFrame(
                    m_Device,
                    m_PhysicalDevice,
                    m_CommandPool,
                    m_DescriptorPool,
                    m_DescriptorSetLayout,
                    m_TextureImageView,
                    m_TextureSampler
                )
            );
        }
    }

    void initMainBuffers()
    {
        m_VertexBuffer = decltype(m_VertexBuffer){
            m_Device,
                m_PhysicalDevice,
                VkBufferCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                },
        };
        m_IndexBuffer = decltype(m_IndexBuffer){
            m_Device,
                m_PhysicalDevice,
                VkBufferCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = 0,
                    .usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                },
        };
    }

    void initGizmoBuffer()
    {
        m_GizmoVB = StagedObjectBuffer<LineVertexType>{
            m_Device,
            m_PhysicalDevice,
            VkBufferCreateInfo{
                .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size = 1 * 1024 * 1024 * sizeof(LineVertexType),   // 1 MB for lines; grow if needed
                .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            },
        };
    }

    void initThreadPool()
    {
        m_ThreadPool.init(std::thread::hardware_concurrency() - 1);
    }

    void init()
    {
        initInputController();
        initWindow();
        initCamera();
        initVulkanCore();
        initTexture();
        initColorDepthTargets();
        initFramebuffers();
        initDescriptors();
        initPipelines();
        initCommandPool();
        initTransferManager();
        initChunkManager();
        initFrames();
        initMainBuffers();
        initGizmoBuffer();
        initThreadPool();
    }

    void cleanup()
    {
        vkDeviceWaitIdle(m_Device);
        if constexpr (kEnableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(m_Instance, *m_DebugMessenger, nullptr);
        }
    }

    [[nodiscard]] bool acquireNextImage(u32& imageIndex)
    {
        auto& frame = m_InFlightFrames[m_CurrentFrameIndex];
        const auto inFlightFence = frame.getInFlightFence();
        vkWaitForFences(m_Device, 1, &inFlightFence, VK_TRUE, std::numeric_limits<u64>::max());

        auto result = vkAcquireNextImageKHR(
            m_Device,
            m_Swapchain,
            std::numeric_limits<u64>::max(),
            frame.getImageAvailableSemaphore(),
            VK_NULL_HANDLE,
            &imageIndex
        );

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            return false;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error{ "Failed to acquire swap chain image!" };
        }

        vkResetFences(m_Device, 1, &inFlightFence);
        return true;
    }

    void submitCommandBuffer(u32 imageIndex)
    {
        auto& frame = m_InFlightFrames[m_CurrentFrameIndex];
        const auto imageAvailableSemaphore = frame.getImageAvailableSemaphore();
        VkCommandBuffer cmd = frame.getCommandBuffer();

        const VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        const VkSemaphore imageRenderFinishedSemaphore = m_Swapchain.getImageRenderFinishedSemaphores()[imageIndex];

        VkSubmitInfo submitInfo{
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &imageAvailableSemaphore,
            .pWaitDstStageMask = &waitStage,
            .commandBufferCount = 1,
            .pCommandBuffers = &cmd,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &imageRenderFinishedSemaphore,
        };

        if (vkQueueSubmit(
            m_Device.getGraphicsQueue(),
            1,
            &submitInfo,
            frame.getInFlightFence()
        ) != VK_SUCCESS)
        {
            throw std::runtime_error{ "Failed to submit command buffer" };
        }
    }

    [[nodiscard]] bool presentImage(u32 imageIndex)
    {
        const VkSwapchainKHR swapchain = m_Swapchain;
        const VkSemaphore imageRenderFinishedSemaphore = m_Swapchain.getImageRenderFinishedSemaphores()[imageIndex];
        VkPresentInfoKHR presentInfo{
            .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
            .waitSemaphoreCount = 1,
            .pWaitSemaphores = &imageRenderFinishedSemaphore,
            .swapchainCount = 1,
            .pSwapchains = &swapchain,
            .pImageIndices = &imageIndex,
        };

        VkResult result = vkQueuePresentKHR(m_Device.getPresentQueue(), &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.hasResized()) {
            m_Window.resetResized();
            return false;
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to present swap chain image!" };
        }
        return true;
    }

    [[nodiscard]] VkInstance createInstance()
    {

        if constexpr (kEnableValidationLayers) {
            if (!checkValidationLayerSupport(
                std::vector(c_ValidationLayers.begin(), c_ValidationLayers.end()))
                ) {
                throw std::runtime_error{ "Validation layers requested, but not available!" };
            }
        }

        u32 extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

        auto extensions = getRequiredExtensions(kEnableValidationLayers);

        VkApplicationInfo appInfo {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Hello Triangle",
            .applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_API_VERSION(0, 1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0,
        };

        VkInstanceCreateInfo createInfo{
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &appInfo,
            .enabledExtensionCount = static_cast<u32>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if constexpr (kEnableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<u32>(c_ValidationLayers.size());
            createInfo.ppEnabledLayerNames = c_ValidationLayers.data();
            populateDebugMessengerCreateInfo(debugCreateInfo, debugCallback);
            createInfo.pNext = &debugCreateInfo;
        }
        else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        VkInstance instance{};
        if (const auto result = vkCreateInstance(
            &createInfo,
            nullptr,
            &instance
        ); result != VK_SUCCESS) {
            switch (result) {
            case VK_ERROR_OUT_OF_HOST_MEMORY:
                throw std::runtime_error{ "Vulkan error: Out of host memory." };
            case VK_ERROR_OUT_OF_DEVICE_MEMORY:
                throw std::runtime_error{ "Vulkan error: Out of device memory." };
            case VK_ERROR_INITIALIZATION_FAILED:
                throw std::runtime_error{ "Vulkan error: Initialization failed." };
            case VK_ERROR_LAYER_NOT_PRESENT:
                throw std::runtime_error{ "Vulkan error: Requested layer is not present." };
            case VK_ERROR_EXTENSION_NOT_PRESENT:
                throw std::runtime_error{ "Vulkan error: Requested extension is not available." };
            case VK_ERROR_INCOMPATIBLE_DRIVER:
                throw std::runtime_error{ "Vulkan error: Incompatible driver." };
            case VK_ERROR_TOO_MANY_OBJECTS:
                throw std::runtime_error{ "Vulkan error: Too many objects." };
            case VK_ERROR_DEVICE_LOST:
                throw std::runtime_error{ "Vulkan error: Device lost." };
            default:
                throw std::runtime_error{ "Vulkan error: Unknown error during instance creation." };
            }
        }

        return instance;
    }

    [[nodiscard]] std::optional<VkDebugUtilsMessengerEXT> setupDebugMessenger(VkInstance instance)
    {
        if constexpr (!kEnableValidationLayers) {
            return std::nullopt;
        }

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo, debugCallback);

        VkDebugUtilsMessengerEXT debugMessenger{};
        if (CreateDebugUtilsMessengerEXT(
            instance,
            "vkCreateDebugUtilsMessengerEXT",
            &createInfo,
            nullptr,
            &debugMessenger
        ) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to set up debug messenger!" };
        }
        return debugMessenger;
    }

    [[nodiscard]] VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window)
    {
        VkSurfaceKHR surface{};
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to create window surface!" };
        }
		return surface;
    }

    [[nodiscard]] VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
    {
        u32 deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, VK_NULL_HANDLE);
        if (deviceCount == 0) {
            throw std::runtime_error{ "Failed to find GPUs with Vulkan support!" };
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        VkPhysicalDevice suitable_device{};
        bool found_device = true;
        for (const auto& device : devices) {
            if (isDeviceSuitable(device, surface)) {
                suitable_device = device;
                found_device = true;
                break;
            }
        }

        if (!found_device) {
            throw std::runtime_error{ "Failed to find a suitable GPU!" };
        }
        return suitable_device;
    }

    [[nodiscard]] Device createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface)
    {
        const auto& indices = findQueueFamilies(physicalDevice, surface);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<u32> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (u32 queueFamily : uniqueQueueFamilies) {
            queueCreateInfos.emplace_back(
                VkDeviceQueueCreateInfo {
                    .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                    .pNext = nullptr, // Optional
                    .flags = 0, // Optional
                    .queueFamilyIndex = queueFamily,
                    .queueCount = 1,
                    .pQueuePriorities = &queuePriority,
                }
            );
        }

        VkPhysicalDeviceFeatures deviceFeatures{
            .sampleRateShading = VK_TRUE,
            .samplerAnisotropy = VK_TRUE,
        };
        VkDeviceCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
            .pNext = nullptr, // Optional
            .flags = 0, // Optional
            .queueCreateInfoCount = static_cast<u32>(queueCreateInfos.size()),
            .pQueueCreateInfos = queueCreateInfos.data(),
            .enabledLayerCount = (kEnableValidationLayers ? static_cast<u32>(c_ValidationLayers.size()) : 0u),
            .ppEnabledLayerNames = (kEnableValidationLayers ? c_ValidationLayers.data() : nullptr),
            .enabledExtensionCount = static_cast<u32>(c_DeviceExtensions.size()),
            .ppEnabledExtensionNames = c_DeviceExtensions.data(),
            .pEnabledFeatures = &deviceFeatures,
        };

        VkDevice device{};
        if (vkCreateDevice(
            physicalDevice,
            &createInfo,
            VK_NULL_HANDLE,
            &device) != VK_SUCCESS
        ) {
            throw std::runtime_error{ "Failed to create logical device!" };
        }

        VkQueue graphicsQueue{};
        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);

        VkQueue presentQueue{};
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);

        return { device, graphicsQueue, presentQueue };
    }

    [[nodiscard]] Swapchain createSwapchain(
        GLFWwindow* window,
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
		VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE
    )
    {
        const auto& swapChainSupport = querySwapChainSupport(physicalDevice, surface);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(window, swapChainSupport.capabilities);

        u32 imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        const auto& indices = findQueueFamilies(physicalDevice, surface);
        std::array<u32, 2> queueFamilyIndices = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        const bool graphicsFamilyNotPresentFamily = indices.graphicsFamily != indices.presentFamily;
        VkSwapchainCreateInfoKHR createInfo{
            .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.pNext = nullptr, // Optional
			.flags = 0, // Optional
            .surface = surface,
            .minImageCount = imageCount,
            .imageFormat = surfaceFormat.format,
            .imageColorSpace = surfaceFormat.colorSpace,
            .imageExtent = extent,
            .imageArrayLayers = 1,
            .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .imageSharingMode = (graphicsFamilyNotPresentFamily ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE),
            .queueFamilyIndexCount = (graphicsFamilyNotPresentFamily ? static_cast<u32>(queueFamilyIndices.size()) : 0u),
            .pQueueFamilyIndices = (graphicsFamilyNotPresentFamily ? queueFamilyIndices.data() : VK_NULL_HANDLE),
            .preTransform = swapChainSupport.capabilities.currentTransform,
            .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            .presentMode = presentMode,
            .clipped = VK_TRUE,
            .oldSwapchain = oldSwapchain,
        };

        VkSwapchainKHR swapChain{};
        if (auto result = vkCreateSwapchainKHR(
            device,
            &createInfo,
            VK_NULL_HANDLE,
            &swapChain); result != VK_SUCCESS
        ) {
            throw std::runtime_error{ "Failed to create swap chain!" };
        }
        
        return Swapchain{
            device,
            extent,
            surfaceFormat.format,
            swapChain,
            createRenderPass(surfaceFormat.format)
        };
    }

    [[nodiscard]] VkRenderPass createRenderPass(VkFormat swapChainImageFormat)
    {
        return RenderPassBuilder{ m_Device }
            .addAttachment(
                VkAttachmentDescription{
                    .format = swapChainImageFormat,
                    .samples = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                }
                )
            .addAttachment(
                VkAttachmentDescription{
                    .format = swapChainImageFormat,
                    .samples = VK_SAMPLE_COUNT_1_BIT,
                    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
                }
                )
            .addAttachment(
                VkAttachmentDescription{
                    .format = PhysicalDevice::findDepthFormat(m_PhysicalDevice),
                    .samples = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                }
                )
            .beginSubpass(
                VK_PIPELINE_BIND_POINT_GRAPHICS
            )
            .addColorAttachmentRef(
                VkAttachmentReference{
                    .attachment = 0,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                }
                )
            .addResolveAttachmentRef(
                VkAttachmentReference{
                    .attachment = 1,
                    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                }
                )
            .setDepthStencilAttachmentRef(
                VkAttachmentReference{
                    .attachment = 2,
                    .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                }
                )
            .endSubpass()
            .addDependency(
                VkSubpassDependency{
                    .srcSubpass = VK_SUBPASS_EXTERNAL,
                    .dstSubpass = 0,
                    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                    .srcAccessMask = 0,
                    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                    .dependencyFlags = 0 // Optional
                }
            )
            .build();
    }

    [[nodiscard]] bool drawFrame()
    {
        u32 imageIndex{};
        if (!acquireNextImage(imageIndex)) {
            return false;
        }

        auto& frame = m_InFlightFrames[m_CurrentFrameIndex];
        vkResetCommandBuffer(frame.getCommandBuffer(), 0);

        updateUniformBuffer(frame);
        recordCommandBuffer(
            frame.getCommandBuffer(),
            m_GraphicsPipeline,
            frame,
            m_Swapchain,
            m_Framebuffers,
            imageIndex
        );
        submitCommandBuffer(imageIndex);

        if (!presentImage(imageIndex)) {
            return false;
        }

        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % kMaxFramesInFlight;
        return true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_PTR debugCallback(
        [[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        [[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageType,
        [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        [[maybe_unused]] void* pUserData
    )
    {
        std::cerr << "[Validation layer]: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    [[nodiscard]] Pipeline createGraphicsPipeline(
        VkDevice device,
        VkExtent2D swapChainExtent,
        VkRenderPass renderPass
    )
    {
        auto vertexAttributeDescriptions = ShapeVertexType::GetAttributeDescriptions();
        return GraphicsPipelineBuilder<>{ device, renderPass }
            .withShaders()
            .addShader<VK_SHADER_STAGE_VERTEX_BIT>(
            joinPaths("shaders", "shader.vert.spv")
            ).addShader<VK_SHADER_STAGE_FRAGMENT_BIT>(
            joinPaths("shaders", "shader.frag.spv")
            ).addVertexInput(
                std::vector{ ShapeVertexType::GetBindingDescription() },
                std::vector(vertexAttributeDescriptions.begin(), vertexAttributeDescriptions.end())
            ).withLayout(
                std::vector<VkDescriptorSetLayout>{ m_DescriptorSetLayout }
            ).addInputAssemblyState(
                VkPipelineInputAssemblyStateCreateInfo{
                    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .pNext                  = nullptr,
                    .flags                  = 0,
                    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
                    .primitiveRestartEnable = VK_FALSE,
                }
            ).addViewportState(
                std::vector<VkViewport>{
                    {
                        .x          = 0.0f,
                        .y          = 0.0f,
                        .width      = static_cast<f32>(swapChainExtent.width),
                        .height     = static_cast<f32>(swapChainExtent.height),
                        .minDepth   = 0.0f,
                        .maxDepth   = 1.0f,
                    },
                },
                std::vector<VkRect2D>{
                    {
                        .offset = { 0, 0 },
                        .extent = swapChainExtent,
                    },
                }
            ).addDynamicState(
                std::vector{
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR,
                }
            ).addRasterizationState(
                VkPipelineRasterizationStateCreateInfo{
                    .sType                      = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                    .pNext                      = nullptr,
                    .flags                      = 0,
                    .depthClampEnable           = VK_FALSE,
                    .rasterizerDiscardEnable    = VK_FALSE,
                    .polygonMode                = VK_POLYGON_MODE_FILL,
                    .cullMode                   = RenderConventions::kCullMode,
                    .frontFace                  = RenderConventions::kFrontFace,
                    .depthBiasEnable            = VK_FALSE,
                    .depthBiasConstantFactor    = 0.0f, // Optional
                    .depthBiasClamp             = 0.0f, // Optional
                    .depthBiasSlopeFactor       = 0.0f, // Optional
                    .lineWidth                  = 1.0f,
                }
            ).addMultisampleState(
                VkPipelineMultisampleStateCreateInfo{
                    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                    .rasterizationSamples   = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                    .sampleShadingEnable    = VK_TRUE,
                    .minSampleShading       = 0.2f, // closer to 1.0f is smooth
                    .pSampleMask            = nullptr,
                    .alphaToCoverageEnable  = VK_FALSE,
                    .alphaToOneEnable       = VK_FALSE,
                }
            ).addColorBlendState(
                std::vector{
                    VkPipelineColorBlendAttachmentState{
                        .blendEnable            = VK_TRUE,
                        .srcColorBlendFactor    = VK_BLEND_FACTOR_SRC_ALPHA,
                        .dstColorBlendFactor    = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                        .colorBlendOp           = VK_BLEND_OP_ADD,
                        .srcAlphaBlendFactor    = VK_BLEND_FACTOR_ONE,
                        .dstAlphaBlendFactor    = VK_BLEND_FACTOR_ZERO,
                        .alphaBlendOp           = VK_BLEND_OP_ADD,
                        .colorWriteMask         = (
                            VK_COLOR_COMPONENT_R_BIT
                            | VK_COLOR_COMPONENT_G_BIT
                            | VK_COLOR_COMPONENT_B_BIT
                            | VK_COLOR_COMPONENT_A_BIT
                        ),
                    },
                }
            ).addDepthStencilState(
                VkPipelineDepthStencilStateCreateInfo{
                    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                    .pNext                  = nullptr,
                    .flags                  = 0,
                    .depthTestEnable        = VK_TRUE,
                    .depthWriteEnable       = VK_TRUE,
                    .depthCompareOp         = VK_COMPARE_OP_LESS,
                    .depthBoundsTestEnable  = VK_FALSE,
                    .stencilTestEnable      = VK_FALSE,
                    .front                  = {}, // Optional
                    .back                   = {}, // Optional
                    .minDepthBounds         = 0.0f, // Optional
                    .maxDepthBounds         = 1.0f, // Optional
                }
            ).build();
    }

    [[nodiscard]] Pipeline createLinePipeline(
        VkDevice device,
        VkExtent2D swapChainExtent,
        VkRenderPass renderPass
    )
    {
        auto attr = LineVertexType::GetAttributeDescriptions();
        return GraphicsPipelineBuilder<>{ device, renderPass }
        .withShaders()
            .addShader<VK_SHADER_STAGE_VERTEX_BIT>(joinPaths("shaders", "line_shader.vert.spv"))
            .addShader<VK_SHADER_STAGE_FRAGMENT_BIT>(joinPaths("shaders", "line_shader.frag.spv"))
            .addVertexInput(
                std::vector{ LineVertexType::GetBindingDescription() },
                std::vector(attr.begin(), attr.end())
            )
            .withLayout(
                std::vector<VkDescriptorSetLayout>{ m_DescriptorSetLayout }
            )
            .addInputAssemblyState(
                VkPipelineInputAssemblyStateCreateInfo{
                    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
                    .topology               = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
                    .primitiveRestartEnable = VK_FALSE,
                }
            )
            .addViewportState(
                std::vector<VkViewport>{
                    VkViewport{
                        .x          = 0.0f,
                        .y          = 0.0f,
                        .width      = static_cast<f32>(swapChainExtent.width),
                        .height     = static_cast<f32>(swapChainExtent.height),
                        .minDepth   = 0.0f,
                        .maxDepth   = 1.0f,
                    }
                },
                std::vector<VkRect2D>{
                    VkRect2D{
                        .offset = { 0, 0 },
                        .extent = swapChainExtent,
                    }
                }
            )
            .addDynamicState(
                std::vector{
                    VK_DYNAMIC_STATE_VIEWPORT,
                    VK_DYNAMIC_STATE_SCISSOR,
                }
            )
            .addRasterizationState(
                VkPipelineRasterizationStateCreateInfo{
                    .sType          = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
                    .polygonMode    = VK_POLYGON_MODE_FILL,
                    .cullMode       = VK_CULL_MODE_NONE,
                    .frontFace      = RenderConventions::kFrontFace,
                    .lineWidth      = 1.0f,
                }
            )
            .addMultisampleState(
                VkPipelineMultisampleStateCreateInfo{
                    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
                    .rasterizationSamples   = PhysicalDevice::getMaxUsableSampleCount(m_PhysicalDevice),
                    .sampleShadingEnable    = VK_FALSE,
                }
            )
            .addColorBlendState(
                std::vector{
                    VkPipelineColorBlendAttachmentState{
                        .blendEnable    = VK_FALSE,
                        .colorWriteMask = (
                            VK_COLOR_COMPONENT_R_BIT
                            | VK_COLOR_COMPONENT_G_BIT
                            | VK_COLOR_COMPONENT_B_BIT
                            | VK_COLOR_COMPONENT_A_BIT
                        ),
                    }
                }
            )
            .addDepthStencilState(
                VkPipelineDepthStencilStateCreateInfo{
                    .sType              = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
                    .depthTestEnable    = VK_TRUE,
                    .depthWriteEnable   = VK_FALSE,
                    .depthCompareOp     = VK_COMPARE_OP_LESS,
                }
            )
            .build();
    }

    [[nodiscard]] Frame createFrame(
        VkDevice device,
        VkPhysicalDevice physicalDevice,
        VkCommandPool commandPool,
        VkDescriptorPool descriptorPool,
        VkDescriptorSetLayout descriptorSetLayout,
        VkImageView imageView,
        VkSampler sampler
    )
    {
        const std::vector<scene::ModelViewProjectionUBO> transformData{
            scene::ModelViewProjectionUBO{},
        };

        resources::DirectObjectBuffer<scene::ModelViewProjectionUBO> transformBuffer{
            device,
            physicalDevice,
            transformData,
            VkBufferCreateInfo {
                .sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .pNext          = nullptr,
                .size           = transformData.size() * sizeof(decltype(transformData)::value_type),
                .usage          = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                .sharingMode    = VK_SHARING_MODE_EXCLUSIVE,
            }
        };

        resources::DirectObjectBuffer<scene::ModelViewProjectionUBO> gizmoTransformBuffer{
            device,
            physicalDevice,
            std::vector<scene::ModelViewProjectionUBO>{
                scene::ModelViewProjectionUBO{}
            },
            VkBufferCreateInfo{
                .sType          = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                .size           = sizeof(scene::ModelViewProjectionUBO),
                .usage          = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                .sharingMode    = VK_SHARING_MODE_EXCLUSIVE,
            }
        };


        const auto descriptorSets = core::helpers::createDescriptorSets(
            device,
            descriptorPool,
            std::vector{
                descriptorSetLayout,
                descriptorSetLayout,
            }
        );

        auto sceneDS = descriptorSets[0];
        auto gizmoDS = descriptorSets[1];

        std::vector<core::helpers::DescriptorWriteInfo> descriptorWrites = {
            core::helpers::DescriptorWriteInfo{
                .type       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .binding    = 0,
                .info = VkDescriptorBufferInfo{
                    .buffer = transformBuffer.getBuffer(),
                    .offset = 0,
                    .range  = transformBuffer.getSize(),
                }
            },
            core::helpers::DescriptorWriteInfo{
                .type       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .binding    = 1,
                .info = VkDescriptorImageInfo{
                    .sampler        = sampler,
                    .imageView      = imageView,
                    .imageLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                },
            },
        };

        std::vector<core::helpers::DescriptorWriteInfo> gizmoWrites = {
            {
                .type       = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .binding    = 0,
                .info = VkDescriptorBufferInfo{
                    .buffer = gizmoTransformBuffer.getBuffer(),
                    .offset = 0,
                    .range  = gizmoTransformBuffer.getSize(),
                },
            },
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .binding = 1,
                .info = VkDescriptorImageInfo{
                    .sampler        = sampler,
                    .imageView      = imageView,
                    .imageLayout    = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                },
            },
        };

        return Frame{
            device,
            core::helpers::createCommandBuffers(
                device,
                VkCommandBufferAllocateInfo{
                    .sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
                    .pNext              = nullptr,
                    .commandPool        = commandPool,
                    .level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = 1,
                }
            )[0],
            core::helpers::createSemaphore(
                device,
                VkSemaphoreCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
                    .pNext = nullptr, 
                    .flags = 0
                }
            ),
            core::helpers::createFence(
                device,
                VkFenceCreateInfo{
                    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
                    .pNext = nullptr,
                    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
                }
            ),
            std::move(transformBuffer),
			std::move(gizmoTransformBuffer),
            sceneDS,
            gizmoDS,
            descriptorWrites
        };
    }

    void drawPendingChunks(VkCommandBuffer cmd)
    {
        VkBuffer vb = m_ChunkManager.getVertexBuffer().getBuffer();
        VkBuffer ib = m_ChunkManager.getIndexBuffer().getBuffer();

        m_ChunkManager.draw(
            [&](const ChunkId&, std::shared_ptr<ChunkInfo> info)
            {
                if (!info || info->indexCount == 0) {
                    return;
                }

                VkDeviceSize vtxOff = info->vertexOffset;
                VkDeviceSize idxOff = info->indexOffset;

                vkCmdBindVertexBuffers(cmd, 0, 1, &vb, &vtxOff);
                vkCmdBindIndexBuffer(cmd, ib, idxOff, RenderConventions::IndexType<IndexT>::kVkIndexType);

                // Because we bound at the chunk’s start, indices are read from there.
                // Use element offsets = 0.
                vkCmdDrawIndexed(cmd, info->indexCount, 1, /*firstIndex*/0, /*vertexOffset*/0, /*firstInstance*/0);
            }
        );
    }

	void recordCommandBuffer(
		VkCommandBuffer commandBuffer,
		Pipeline& graphicsPipeline,
        Frame& frame,
        Swapchain& swapChain,
        FramebufferSet& frameBuffers,
        u32 imageIndex
	)
    {
        const auto& swapChainExtent = swapChain.getExtent();
		VkCommandBufferBeginInfo beginInfo {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.pNext = nullptr,
			.flags = 0,
			.pInheritanceInfo = nullptr,
		};

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to begin recording command buffer!" };
		}

        std::array<VkClearValue, 3> clearValues{
            {
                VkClearValue{
                    .color = { 0.0f, 0.0f, 0.0f, 1.0f },
                },
                VkClearValue{
                    .color = { 0.0f, 0.0f, 0.0f, 1.0f },
                },
                VkClearValue{
                    .depthStencil = { 1.0f, 0 },
                },
            }
        };
        VkRenderPassBeginInfo renderPassInfo {
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .pNext = nullptr,
            .renderPass = swapChain.getRenderPass(),
            .framebuffer = frameBuffers.getFramebuffers()[imageIndex],
            .renderArea = {
                .offset = { 0, 0 },
                .extent = swapChainExtent,
            },
            .clearValueCount = static_cast<u32>(clearValues.size()),
            .pClearValues = clearValues.data(),
        };

        uploadBarriers(commandBuffer);
        
		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		
        VkViewport viewport {
            .x = 0.0f,
            .y = 0.0f,
            .width = static_cast<f32>(swapChainExtent.width),
            .height = static_cast<f32>(swapChainExtent.height),
            .minDepth = 0.0f,
            .maxDepth = 1.0f,
        };
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor {
			.offset = { 0, 0 },
			.extent = swapChainExtent,
        };
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        const auto frameDescriptorSet = frame.getDescriptorSet();
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            graphicsPipeline.getPipelineLayout(),
            0,
            1,
            &frameDescriptorSet,
            0,
            nullptr
        );

        drawPendingChunks(commandBuffer);

        drawGizmos(commandBuffer, m_LinePipeline, frame);

		vkCmdEndRenderPass(commandBuffer);
		
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error{ "Failed to record command buffer!" };
		}
	}

	void recreateSwapchain()
    {
        int width = 0;
        int height = 0;
		glfwGetFramebufferSize(m_Window, &width, &height);
        while (width == 0 || height == 0) {
            glfwGetFramebufferSize(m_Window, &width, &height);
            glfwWaitEvents();
        }
        m_Camera.setProjection(
            m_Camera.getFovRadians(),
            static_cast<f32>(width) / static_cast<f32>(height),
            m_Camera.getNear(),
            m_Camera.getFar()
        );
        m_ThreadPool.stop().waitIdle().init().start();
		vkDeviceWaitIdle(m_Device);
        m_ChunkManager.rebuild(&m_Camera);
		m_Swapchain = createSwapchain(
			m_Window,
			m_Device,
			m_PhysicalDevice,
			m_Surface,
			m_Swapchain
		);
        resetColorDepthTargets();
        resetFramebuffers();
        resetPipelines();
        resetDescriptorPool();
        vkResetCommandPool(m_Device, m_CommandPool, 0);
        resetFrames();
	}

    void updateUniformBuffer(Frame& frame)
    {
        scene::ModelViewProjectionUBO transform{
            .model = m_Model = glm::rotate(
                m_Model,
                m_DeltaTime.count() * glm::radians(0.0f),
                kVec3Up
            ),
			.view = m_Camera.getViewMatrix(),
			.projection = m_Camera.getProjectionMatrix(),
        };
        frame.getTransformBuffer().updateData(std::span(&transform, 1), 0);

        scene::ModelViewProjectionUBO gizmoTransform{
            .model = glm::mat4{ 1.0f },
            .view = m_Camera.getViewMatrix(),
            .projection = m_Camera.getProjectionMatrix(),
        };
        frame.getGizmoTransformBuffer().updateData(std::span(&gizmoTransform, 1), 0);
    }

    void uploadBarriers(VkCommandBuffer commandBuffer) const
    {
        static constexpr auto makeBarrier = [](
            VkBuffer buffer,
            VkDeviceSize offset,
            VkDeviceSize size,
            VkAccessFlags dstAccess
        )
        {
            return VkBufferMemoryBarrier{
                .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
                .pNext = nullptr,
                .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                .dstAccessMask = dstAccess,
                .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                .buffer = buffer,
                .offset = offset,
                .size = size,
            };
        };

        std::array<VkBufferMemoryBarrier, 3> bufBarriers{
            {
                makeBarrier(
                    m_ChunkManager.getVertexBuffer().getBuffer(),
                    0,
                    VK_WHOLE_SIZE,
                    VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
                ),
                makeBarrier(
                    m_ChunkManager.getIndexBuffer().getBuffer(),
                    0,
                    VK_WHOLE_SIZE,
                    VK_ACCESS_INDEX_READ_BIT
                ),
                makeBarrier(
                    m_GizmoVB.getBuffer(),
                    0,
                    VK_WHOLE_SIZE,
                    VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT
                ),
            }
        };

        // One barrier for all three buffers
        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT,         // srcStage
            VK_PIPELINE_STAGE_VERTEX_INPUT_BIT,     // dstStage (covers attrib + index fetch)
            0,
            0,
            nullptr,
            static_cast<u32>(bufBarriers.size()),
            bufBarriers.data(),
            0,
            nullptr
        );
    }

    ResultQueue m_ResultQueue{};
    InputController m_InputController{};
    Window m_Window{};
    CameraFirstPerson m_Camera{};
    CameraController m_CameraController{};
    ImageData m_ImageData{};
    OwningWrapper<VkInstance> m_Instance{};
    std::optional<VkDebugUtilsMessengerEXT> m_DebugMessenger{};
    OwningWrapper<VkSurfaceKHR> m_Surface{};
    PhysicalDevice m_PhysicalDevice{};
    Device m_Device{};
    ImageResource m_Texture{};
    ImageView m_TextureImageView{};
    OwningWrapper<VkSampler> m_TextureSampler{};
    Swapchain m_Swapchain{};
    ImageResource m_ColorImage{};
    ImageView m_ColorImageView{};
    ImageResource m_DepthImage{};
    ImageView m_DepthImageView{};
    FramebufferSet m_Framebuffers{};
    OwningWrapper<VkDescriptorPool> m_DescriptorPool{};
    OwningWrapper<VkDescriptorSetLayout> m_DescriptorSetLayout{};
    Pipeline m_GraphicsPipeline{};
    Pipeline m_LinePipeline{};                // line list pipeline
    StagedObjectBuffer<LineVertexType> m_GizmoVB{};     // one big line-vertex buffer
    std::vector<LineVertexType> m_GizmoVertices{};      // rebuilt each frame
    OwningWrapper<VkCommandPool> m_CommandPool{};
	JobManager m_JobManager{};
	ChunkManagerType m_ChunkManager{};
	std::array<Frame, kMaxFramesInFlight> m_InFlightFrames{};
    std::size_t m_CurrentFrameIndex{};
    std::vector<ShapeVertexType> m_Vertices{};
    std::vector<u32> m_Indices{};
    StagedObjectBuffer<decltype(m_Vertices)::value_type> m_VertexBuffer{};
    StagedObjectBuffer<decltype(m_Indices)::value_type> m_IndexBuffer{};
    std::chrono::time_point<std::chrono::steady_clock> m_LastOrigin{};
    std::chrono::time_point<std::chrono::steady_clock> m_LastFrameTime{};
    std::chrono::duration<f32> m_DeltaTime{};
    glm::mat4 m_Model{ 1.0f };

    ThreadPool m_ThreadPool{};
};

int main()
{
    glfwInit();
    App{}.run();
    glfwTerminate();
    return EXIT_SUCCESS;
}
