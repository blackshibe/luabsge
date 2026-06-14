#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/pipeline/vulkan_compute_pipeline.h"
#include "rendering/vulkan/pipeline/vulkan_graphics_pipeline.h"
#include <cstddef>
#include <span>
#include <vector>

void Vulkan::Renderer::init_commands() {
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.queueFamilyIndex = queue_family;

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateCommandPool(device.vk_device, &commandPoolInfo, nullptr, &_frames[i]._commandPool));

		VkCommandBufferAllocateInfo cmdAllocInfo = {};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.pNext = nullptr;
		cmdAllocInfo.commandPool = _frames[i]._commandPool;
		cmdAllocInfo.commandBufferCount = 1;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

		VK_CHECK(vkAllocateCommandBuffers(device.vk_device, &cmdAllocInfo, &_frames[i]._mainCommandBuffer));
	}
}

void Vulkan::Renderer::init_sync_structures() {
	// one fence to control when the gpu has finished rendering the frame, and 2
	// semaphores to syncronize rendering with the swapchain. The fence starts
	// signalled (VK_FENCE_CREATE_SIGNALED_BIT) so the first frame's wait doesn't block.
	VkFenceCreateInfo fenceCreateInfo = Vulkan::init::fence_create_info(VK_FENCE_CREATE_SIGNALED_BIT);
	VkSemaphoreCreateInfo semaphoreCreateInfo = Vulkan::init::semaphore_create_info();

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		VK_CHECK(vkCreateFence(device.vk_device, &fenceCreateInfo, nullptr, &_frames[i]._renderFence));

		VK_CHECK(vkCreateSemaphore(device.vk_device, &semaphoreCreateInfo, nullptr, &_frames[i]._swapchainSemaphore));
	}

	// one present-wait semaphore per swapchain image, indexed by the acquired image
	render_semaphores.resize(swapchain.images.size());
	for (size_t i = 0; i < render_semaphores.size(); i++) {
		VK_CHECK(vkCreateSemaphore(device.vk_device, &semaphoreCreateInfo, nullptr, &render_semaphores[i]));
	}
}

void Vulkan::Renderer::init_descriptors() {
	// a descriptor pool that will hold 10 sets, each with 1 storage image (the type
	// used for an image a compute shader can write to)
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};
	global_descriptor_allocator.init_pool(device.vk_device, 10, sizes);

	// the descriptor set layout for our compute draw: a single binding 0 of type
	// storage image, visible to the compute stage
	{
		DescriptorLayoutBuilder builder;
		builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		draw_image_descriptor_layout = builder.build(device.vk_device, VK_SHADER_STAGE_COMPUTE_BIT);
	}

	// allocate a descriptor set of that layout and point it at our draw image
	draw_image_descriptors = global_descriptor_allocator.allocate(device.vk_device, draw_image_descriptor_layout);

	VkDescriptorImageInfo imgInfo = {};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = draw_image.imageView;

	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;
	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = draw_image_descriptors;
	drawImageWrite.descriptorCount = 1;
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(device.vk_device, 1, &drawImageWrite, 0, nullptr);

	lifetime_deletion_queue.push_function([this]() {
		global_descriptor_allocator.destroy_pool(device.vk_device);
		vkDestroyDescriptorSetLayout(device.vk_device, draw_image_descriptor_layout, nullptr);
	});
}

void Vulkan::Renderer::init_pipelines() {
	init_background_pipeline();
	init_triangle_pipeline();
}

void Vulkan::Renderer::init_background_pipeline() {
	// the pipeline layout only needs the draw-image descriptor set layout; this shader
	// has no push constants
	VkPipelineLayoutCreateInfo layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	layout_info.pNext = nullptr;
	layout_info.pSetLayouts = &draw_image_descriptor_layout;
	layout_info.setLayoutCount = 1;

	gradient_pipeline = std::make_unique<Vulkan::pipeline::ComputePipeline>(device, layout_info, "shader/gradient.comp.spv");

	// idk how else to do this
	lifetime_deletion_queue.push_function([this]() {
		Vulkan::pipeline::ComputePipeline *pipeline = this->gradient_pipeline.get();
		if (pipeline != NULL) pipeline->destroy();
	});
}

void Vulkan::Renderer::init_triangle_pipeline() {
	// build the pipeline layout that controls the inputs/outputs of the shader. we are
	// not using descriptor sets or other systems yet, so no need to use anything other
	// than empty default.
	VkPipelineLayoutCreateInfo layout_info = Vulkan::init::pipeline_layout_create_info();

	// connect the image format we will draw into (the draw image) so the graphics
	// pipeline targets the same off-swapchain image the compute background wrote to
	triangle_pipeline = std::make_unique<Vulkan::pipeline::GraphicsPipeline>(
	    device, layout_info, "shader/colored_triangle.vert.spv", "shader/colored_triangle.frag.spv", draw_image.imageFormat);

	lifetime_deletion_queue.push_function([this]() {
		Vulkan::pipeline::GraphicsPipeline *pipeline = this->triangle_pipeline.get();
		if (pipeline != NULL) pipeline->destroy();
	});
}

void Vulkan::Renderer::init_imgui(GLFWwindow *glfw_window) {
	{
		// 1: create descriptor pool for IMGUI
		//  the size of the pool is very oversize, but it's copied from imgui demo
		//  itself.
		VkDescriptorPoolSize pool_sizes[] = {{VK_DESCRIPTOR_TYPE_SAMPLER, 1000},
		                                     {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
		                                     {VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000},
		                                     {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000},
		                                     {VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000},
		                                     {VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000},
		                                     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000},
		                                     {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000},
		                                     {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000},
		                                     {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000},
		                                     {VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000}};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 1000;
		pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		VkDescriptorPool imguiPool;
		VK_CHECK(vkCreateDescriptorPool(device.vk_device, &pool_info, nullptr, &imguiPool));

		// 2: initialize imgui library

		// this initializes the core structures of imgui
		ImGui::CreateContext();

		// this initializes imgui for SDL
		ImGui_ImplGlfw_InitForVulkan(glfw_window, true);

		// this initializes imgui for Vulkan
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.ApiVersion = VK_API_VERSION_1_3;
		init_info.Instance = instance.instance;
		init_info.PhysicalDevice = physical_device;
		init_info.Device = device.vk_device;
		init_info.QueueFamily = queue_family;
		init_info.Queue = device.graphics_queue;
		init_info.DescriptorPool = imguiPool;
		init_info.MinImageCount = 3;
		init_info.ImageCount = 3;
		init_info.UseDynamicRendering = true;

		// dynamic rendering parameters for imgui to use
		ImGui_ImplVulkan_PipelineInfo pipeline_info = {};
		pipeline_info.PipelineRenderingCreateInfo = {.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO};
		pipeline_info.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
		pipeline_info.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchain.image_format;
		pipeline_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.PipelineInfoMain = pipeline_info;

		ImGui_ImplVulkan_Init(&init_info);

		// add the destroy the imgui created structures
		lifetime_deletion_queue.push_function([=, this]() {
			ImGui_ImplVulkan_Shutdown();
			vkDestroyDescriptorPool(device.vk_device, imguiPool, nullptr);
		});
	}
}