#pragma once

#include "rendering/vulkan/pipeline/vulkan_compute_pipeline.h"
#include "rendering/vulkan/vulkan.h"

#include "engine/queue.h"
#include <GLFW/glfw3.h>
#include <memory>

class EngineInstance;

class AbstractRenderer {};

namespace Vulkan {

	constexpr unsigned int FRAME_OVERLAP = 2;

	struct Frame {
		VkCommandPool _commandPool = VK_NULL_HANDLE;
		VkCommandBuffer _mainCommandBuffer = VK_NULL_HANDLE;

		VkSemaphore _swapchainSemaphore = VK_NULL_HANDLE;
		VkFence _renderFence = VK_NULL_HANDLE;

		// per-frame deletion queue: flushed after the frame's fence so we can safely
		// delete objects created for that specific frame
		DeletionQueue _deletionQueue;
	};

	class Renderer : AbstractRenderer {
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
		VkDescriptorSet draw_image_descriptors = VK_NULL_HANDLE;
		VkDescriptorSetLayout draw_image_descriptor_layout = VK_NULL_HANDLE;

		std::unique_ptr<Vulkan::pipeline::ComputePipeline> gradient_pipeline;

		void init_vulkan(EngineInstance &engine, GLFWwindow *glfw_window);
		void create_swapchain(uint32_t width, uint32_t height);
		void init_commands();
		void init_sync_structures();
		void init_descriptors();
		void init_pipelines();
		void init_background_pipeline();
		void init_imgui(GLFWwindow *glfw_window);

		void draw_pipeline(VkCommandBuffer vk_buffer, Vulkan::pipeline::ComputePipeline pipeline);
		void draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view);

		Frame _frames[FRAME_OVERLAP];
		Frame &get_current_frame() { return _frames[frame_number % FRAME_OVERLAP]; };

	  public:
		Renderer(EngineInstance &engine, GLFWwindow *glfw_window);
		~Renderer();

		void draw();
	};
}