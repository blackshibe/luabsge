#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/pipeline/vulkan_compute_pipeline.h"
#include "rendering/vulkan/pipeline/vulkan_graphics_pipeline.h"
#include "vulkan/vulkan_core.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
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

	// a separate command pool + buffer for immediate_submit, used to upload mesh
	// data to the GPU outside of the per-frame command buffers
	VK_CHECK(vkCreateCommandPool(device.vk_device, &commandPoolInfo, nullptr, &imm_command_pool));

	VkCommandBufferAllocateInfo immAllocInfo = {};
	immAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	immAllocInfo.pNext = nullptr;
	immAllocInfo.commandPool = imm_command_pool;
	immAllocInfo.commandBufferCount = 1;
	immAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VK_CHECK(vkAllocateCommandBuffers(device.vk_device, &immAllocInfo, &imm_command_buffer));
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

	// fence used by immediate_submit to block the CPU until a one-off upload finishes
	VK_CHECK(vkCreateFence(device.vk_device, &fenceCreateInfo, nullptr, &imm_fence));
}

void Vulkan::Renderer::init_descriptors() {
	// a descriptor pool that will hold 10 sets, each with 1 storage image (the type
	// used for an image a compute shader can write to)
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes = {{VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};
	global_descriptor_allocator.init_pool(device.vk_device, 10, sizes);

	// // the descriptor set layout for our compute draw: a single binding 0 of type
	// // storage image, visible to the compute stage
	// DescriptorLayoutBuilder builder;
	// builder.add_binding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
	// auto new_layout = builder.build(device.vk_device, VK_SHADER_STAGE_COMPUTE_BIT);

	// // allocate a descriptor set of that layout and point it at our draw image
	// auto draw_image_descriptors = global_descriptor_allocator.allocat e(device.vk_device, new_layout);

	// VkDescriptorImageInfo imgInfo = {};
	// imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	// imgInfo.imageView = draw_image.vk_view;

	// VkWriteDescriptorSet drawImageWrite = {};
	// drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	// drawImageWrite.pNext = nullptr;
	// drawImageWrite.dstBinding = 0;
	// drawImageWrite.dstSet = draw_image_descriptors;
	// drawImageWrite.descriptorCount = 1;
	// drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	// drawImageWrite.pImageInfo = &imgInfo;

	// vkUpdateDescriptorSets(device.vk_device, 1, &drawImageWrite, 0, nullptr);

	// lifetime_deletion_queue.push_function([this]() {
	// 	global_descriptor_allocator.destroy_pool(device.vk_device);
	// 	vkDestroyDescriptorSetLayout(device.vk_device, draw_image_descriptor_layout, nullptr);
	// });

	// each frame gets its own pool for the transient combined-image-sampler sets we
	// allocate while recording draws; it is reset at the start of every frame
	std::vector<DescriptorAllocator::PoolSizeRatio> frame_sizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
	                                                               {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1}};
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		_frames[i].frame_descriptors.init_pool(device.vk_device, 1000, frame_sizes);
	}

	// nearest-filter sampler shared by every texture bind
	VkSamplerCreateInfo sampler_info = {.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	sampler_info.magFilter = VK_FILTER_NEAREST;
	sampler_info.minFilter = VK_FILTER_NEAREST;
	VK_CHECK(vkCreateSampler(device.vk_device, &sampler_info, nullptr, &_defaultSamplerNearest));

	// a 16x16 magenta/black checkerboard used as the fallback texture
	uint32_t black = 0x00000000;
	uint32_t magenta = 0xFFFF00FF;
	std::array<uint32_t, 16 * 16> checkerboard;
	for (int y = 0; y < 16; y++) {
		for (int x = 0; x < 16; x++) {
			checkerboard[y * 16 + x] = ((x % 2) ^ (y % 2)) ? magenta : black;
		}
	}

	_errorCheckerboardImage = create_image(checkerboard.data(), VkExtent3D{16, 16, 1}, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_SAMPLED_BIT);

	lifetime_deletion_queue.push_function([this]() {
		vkDestroySampler(device.vk_device, _defaultSamplerNearest, nullptr);
	});
}

void Vulkan::Renderer::init_pipelines() {
	// Now we call this function from our main init_pipelines() function.
	init_mesh_pipeline();

	// Passes 1 sampler to shader
	DescriptorLayoutBuilder prepass_builder;
	VkDescriptorSetLayout prepass_image_layout = prepass_builder
	                                                 .add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	                                                 .build(device.vk_device, VK_SHADER_STAGE_FRAGMENT_BIT);

	// binding 0: sampled input texture, binding 1: storage image written by the shader
	DescriptorLayoutBuilder lighting_builder;
	VkDescriptorSetLayout lighting_image_layout = lighting_builder
	                                                  .add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	                                                  .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	                                                  .add_binding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	                                                  .build(device.vk_device, VK_SHADER_STAGE_COMPUTE_BIT);

	// We change the vertex shader to load colored_triangle_mesh.vert.spv, and we modify the pipeline layout to give
	// it the push constants struct we defined above. The vertex data is reached through a buffer device address in
	// the push constants, so the layout needs no descriptor sets, only the push-constant range.
	VkPushConstantRange buffer_range = Vulkan::init::push_constant_range<Vulkan::GPUDrawPrepassConstants>(VK_SHADER_STAGE_VERTEX_BIT, 0);
	VkPushConstantRange lighting_range = Vulkan::init::push_constant_range<Vulkan::GPUDrawLightingConstants>(VK_SHADER_STAGE_COMPUTE_BIT, 0);

	VkPipelineLayoutCreateInfo prepass_draw_geometry_layout = Vulkan::init::pipeline_layout_create_info();
	prepass_draw_geometry_layout.pPushConstantRanges = &buffer_range;
	prepass_draw_geometry_layout.pushConstantRangeCount = 1;
	prepass_draw_geometry_layout.pSetLayouts = &prepass_image_layout;
	prepass_draw_geometry_layout.setLayoutCount = 1;

	VkPipelineLayoutCreateInfo pass_draw_lighting_layout = Vulkan::init::pipeline_layout_create_info();
	pass_draw_lighting_layout.pPushConstantRanges = &lighting_range;
	pass_draw_lighting_layout.pushConstantRangeCount = 1;
	pass_draw_lighting_layout.pSetLayouts = &lighting_image_layout;
	pass_draw_lighting_layout.setLayoutCount = 1;

	VkExtent3D extent = draw_image.extent;

	// TODO draw image format dependent on pipeline
	pipelines = std::unique_ptr<Vulkan::EnginePipelines>(new Vulkan::EnginePipelines{
	    .image_depth = create_image(extent,
		                            VK_FORMAT_R8G8B8A8_UNORM,
		                            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		                            false),
	    .prepass_depth = Vulkan::pipeline::GraphicsPipeline(
	        "depth_prepass",
	        device,
	        prepass_draw_geometry_layout,
	        "shader/prepass/depth.vert.spv",
	        "shader/prepass/depth.frag.spv",
	        VK_FORMAT_R8G8B8A8_UNORM),

	    .image_albedo = create_image(extent,
		                             VK_FORMAT_R8G8B8A8_UNORM,
		                             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		                             false),

	    .prepass_albedo = Vulkan::pipeline::GraphicsPipeline(
	        "albedo_prepass",
	        device,
	        prepass_draw_geometry_layout,
	        "shader/prepass/albedo.vert.spv",
	        "shader/prepass/albedo.frag.spv",
	        VK_FORMAT_R8G8B8A8_UNORM),

	    .image_lighting = create_image(
	        extent,
	        VK_FORMAT_R8G8B8A8_UNORM,
	        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
	        false),

	    .pass_lighting = Vulkan::pipeline::ComputePipeline(
	        "lighting_pass",
	        device,
	        pass_draw_lighting_layout,
	        "shader/pass/lighting.comp.spv"),
	});

	// lifetime_deletion_queue.push_function([this]() {
	// 	Vulkan::pipeline::GraphicsPipeline *pipeline = this->mesh_pipeline.get();
	// 	if (pipeline != NULL) pipeline->destroy();
	// });
}

void Vulkan::Renderer::init_mesh_pipeline() {
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