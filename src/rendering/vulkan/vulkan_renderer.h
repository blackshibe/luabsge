#pragma once

#include "rendering/vulkan/base/vulkan_types.h"
#include "rendering/vulkan/pipeline/vulkan_compute_pipeline.h"
#include "rendering/vulkan/pipeline/vulkan_graphics_pipeline.h"
#include "rendering/vulkan/vulkan.h"
#include "resource/mesh/mesh.h"

#include "engine/queue.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <memory>
#include <vector>

class EngineInstance;

class AbstractRenderer {};

namespace Vulkan {

	constexpr unsigned int FRAME_OVERLAP = 2;

	// todo move
	struct EnginePipelines {
	  public:
		AllocatedImage image_depth;
		Vulkan::pipeline::GraphicsPipeline prepass_depth;
		// Vulkan::pipeline::GraphicsPipeline prepass_position;
		// Vulkan::pipeline::GraphicsPipeline prepass_normal;

		AllocatedImage image_albedo;
		Vulkan::pipeline::GraphicsPipeline prepass_albedo;

		AllocatedImage image_lighting;
		Vulkan::pipeline::ComputePipeline pass_lighting;
	};

	struct Frame {
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		VkCommandBuffer _mainCommandBuffer = VK_NULL_HANDLE;

		VkSemaphore _swapchainSemaphore = VK_NULL_HANDLE;
		VkFence _renderFence = VK_NULL_HANDLE;

		// per-frame deletion queue: flushed after the frame's fence so we can safely
		// delete objects created for that specific frame
		DeletionQueue _deletionQueue;

		// per-frame descriptor pool, reset at the start of the frame so transient sets
		// (like the texture bound for a draw) don't accumulate across frames
		DescriptorAllocator frame_descriptors;
	};

	class Renderer : AbstractRenderer {
		// the engine owns the ECS registry and the mesh resource bank
		EngineInstance *engine = nullptr;

		Vulkan::Instance instance;
		Vulkan::Device device;
		Vulkan::Swapchain swapchain;

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;

		uint32_t queue_family = 0;
		uint64_t frame_number = 0;

		// present-wait semaphores, one per swapchain image and indexed by the acquired
		// image index. The present that waits on a semaphore isn't tied to our frame
		// fence, so a per-frame semaphore could be reused before its present completes.
		// TODO clautard decided this needs a vector despite having size driven by a constant
		std::vector<VkSemaphore> render_semaphores;

		VmaAllocator allocator = nullptr;
		DeletionQueue lifetime_deletion_queue;

		// we render into this off-swapchain image (RGBA 16-bit float for extra precision)
		// and then copy it to the swapchain image before presenting
		AllocatedImage draw_image;
		VkExtent2D draw_extent = {};

		DescriptorAllocator global_descriptor_allocator;

		// nearest-filter sampler used when binding textures for mesh draws
		VkSampler _defaultSamplerNearest = VK_NULL_HANDLE;

		// magenta/black checkerboard bound when a mesh has no texture to sample
		AllocatedImage _errorCheckerboardImage;

		std::unique_ptr<EnginePipelines> pipelines;
		// std::unique_ptr<Vulkan::pipeline::ComputePipeline> gradient_pipeline;
		// std::unique_ptr<Vulkan::pipeline::GraphicsPipeline> triangle_pipeline;

		// immediate-submit structures: a one-off command buffer + fence used to run
		// GPU work (like staging-buffer copies) synchronously, outside the frame loop
		VkFence imm_fence = VK_NULL_HANDLE;
		VkCommandPool imm_command_pool = VK_NULL_HANDLE;
		VkCommandBuffer imm_command_buffer = VK_NULL_HANDLE;

		// GPU buffers for every mesh in EngineInstance::resources.meshes, kept in the
		// same order/index so a MeshComponent's mesh_index maps straight into here
		std::vector<Vulkan::GPUMeshBuffers> gpu_meshes;

		// GPU images for every texture in EngineInstance::resources.images, kept
		// index-aligned with the resource bank just like gpu_meshes
		std::vector<Vulkan::AllocatedImage> gpu_textures;

		// creation
		void init_vulkan(EngineInstance &engine, GLFWwindow *glfw_window);
		void create_swapchain(uint32_t width, uint32_t height);
		void init_commands();
		void init_sync_structures();
		void init_descriptors();
		void init_pipelines();
		void init_background_pipeline();
		void init_triangle_pipeline();
		void init_mesh_pipeline();
		void init_imgui(GLFWwindow *glfw_window);

		// drawing
		void draw_pipeline(VkCommandBuffer vk_buffer, Vulkan::pipeline::ComputePipeline pipeline, VkDescriptorSet descriptor_set);
		void draw_geometry_pipeline(VkCommandBuffer vk_buffer, VkImageView target_view, Vulkan::pipeline::GraphicsPipeline pipeline);
		void draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view);

		// buffer
		AllocatedBuffer allocate_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
		void destroy_buffer(const Vulkan::AllocatedBuffer &buffer);

		// image
		AllocatedImage create_image(VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		AllocatedImage create_image(void *data, VkExtent3D size, VkFormat format, VkImageUsageFlags usage, bool mipmapped = false);
		void destroy_image(const AllocatedImage &img);

		// mesh
		Vulkan::GPUMeshBuffers upload_mesh(std::span<uint32_t> indices, std::span<MeshVertex> vertices);

		// run a function on the GPU and block until it finishes
		void immediate_submit(std::function<void(VkCommandBuffer cmd)> &&function);

		// upload any resource-bank meshes that don't have GPU buffers yet
		void upload_pending_meshes();

		// upload any resource-bank textures that don't have GPU images yet
		void upload_pending_textures();

		// swapchain?
		Frame _frames[FRAME_OVERLAP];
		Frame &get_current_frame() { return _frames[frame_number % FRAME_OVERLAP]; };

	  public:
		Renderer(EngineInstance &engine, GLFWwindow *glfw_window);
		~Renderer();

		void draw();
	};
}