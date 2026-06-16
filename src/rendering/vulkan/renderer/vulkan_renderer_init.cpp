#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/vulkan_renderer.h"

#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_vulkan.h"
#include "rendering/vulkan/pipeline/compute/vulkan_compute_pipeline.h"
#include "rendering/vulkan/pipeline/geometry/vulkan_geometry_pipeline.h"
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
	std::vector<DescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1},
	                                                                       {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1},
	                                                                       {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}};
	for (int i = 0; i < FRAME_OVERLAP; i++) {
		_frames[i].frame_descriptors.init(device.vk_device, 1000, frame_sizes);
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

	// binding 0/1: sampled albedo+depth prepass, binding 2: storage image written by the
	// shader, binding 3: directional lights buffer, binding 4: point lights buffer
	DescriptorLayoutBuilder lighting_builder;
	VkDescriptorSetLayout lighting_image_layout = lighting_builder
	                                                  .add_binding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	                                                  .add_binding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
	                                                  .add_binding(2, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE)
	                                                  .add_binding(3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
	                                                  .add_binding(4, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER)
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

	// for prepasses:
	// 	vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_pipeline);

	// // bind a texture: use the first imported texture if there is one, otherwise fall
	// // back to the magenta/black checkerboard
	// VkImageView texture_view = gpu_textures.empty() ? _errorCheckerboardImage.vk_view : gpu_textures[0].vk_view;

	// VkDescriptorSet imageSet = get_current_frame().frame_descriptors.allocate(device.vk_device, pipeline.vk_descriptor_layout);
	// {
	// 	Vulkan::DescriptorWriter writer;
	// 	writer.write_image(0, texture_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);

	// 	writer.update_set(device.vk_device, imageSet);
	// }

	// vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.vk_layout, 0, 1, &imageSet, 0, nullptr);

	// Vulkan::image::transition_image(vk_command_buffer, pipelines.get()->image_depth.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	// Vulkan::Renderer::draw_geometry_pipeline(vk_command_buffer, pipelines.get()->image_depth.vk_view, pipelines.get()->prepass_depth);

	// for main

	// pipelines->pass_lighting.descriptor.write_image(0, pipelines.get()->image_albedo.vk_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	// pipelines->pass_lighting.descriptor.write_image(1, pipelines.get()->image_depth.vk_view, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
	// pipelines->pass_lighting.descriptor.write_image(2, draw_image.vk_view, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);

	// Vulkan::image::transition_image(vk_command_buffer, pipelines.get()->image_albedo.vk_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	// Vulkan::image::transition_image(vk_command_buffer, pipelines.get()->image_depth.vk_image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	// Vulkan::image::transition_image(vk_command_buffer, draw_image.vk_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// TODO draw image format dependent on pipeline
	pipelines = std::unique_ptr<Vulkan::EnginePipelines>(new Vulkan::EnginePipelines{
	    .prepass_depth = Vulkan::pipeline::GraphicsPipeline(
	        "depth_prepass",
	        device,
	        prepass_draw_geometry_layout,
	        "shader/prepass/depth.vert.spv",
	        "shader/prepass/depth.frag.spv",
	        create_image(extent,
			             VK_FORMAT_R8G8B8A8_UNORM,
			             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			             false),
	        create_image(extent,
			             VK_FORMAT_D32_SFLOAT,
			             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			             false)

	            ),

	    .prepass_albedo = Vulkan::pipeline::GraphicsPipeline(
	        "albedo_prepass",
	        device,
	        prepass_draw_geometry_layout,
	        "shader/prepass/albedo.vert.spv",
	        "shader/prepass/albedo.frag.spv",
	        create_image(extent,
			             VK_FORMAT_R8G8B8A8_UNORM,
			             VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			             false),
	        create_image(extent,
			             VK_FORMAT_D32_SFLOAT,
			             VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
			             false)),

	    .pass_lighting = Vulkan::pipeline::ComputePipeline(
	        "lighting_pass",
	        device,
	        pass_draw_lighting_layout,
	        "shader/pass/lighting.comp.spv"),
	});

	pipelines->pass_lighting.bindings = {
	    Vulkan::pipeline::ImageBinding(0, &pipelines->prepass_albedo.color_image, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
	    Vulkan::pipeline::ImageBinding(1, &pipelines->prepass_depth.depth_image, _defaultSamplerNearest, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER),
	    Vulkan::pipeline::ImageBinding(2, &draw_image, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_GENERAL, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE),
	};

	// configure sampling for the lighting pass

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