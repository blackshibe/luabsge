#include "rendering/vulkan/vulkan_renderer.h"

#include "rendering/vulkan/vulkan_init.h"
#include "rendering/vulkan/vulkan_images.h"
#include "rendering/vulkan/pipeline/vulkan_pipeline.h"
#include "engine/engine.h"
#include "include/imgui/imgui_impl_glfw.h"
#include "include/imgui/imgui_impl_vulkan.h"
#include "vulkan/vulkan_core.h"

#include <cmath>
#include <cstddef>
#include <span>
#include <vector>
#include <stdexcept>

Vulkan::Renderer::Renderer(EngineInstance &engine, GLFWwindow *glfw_window) {
	init_vulkan(engine, glfw_window);
}

Vulkan::Renderer::~Renderer() {
	if (device.vk_device != VK_NULL_HANDLE) vkDeviceWaitIdle(device.vk_device);

	for (int i = 0; i < FRAME_OVERLAP; i++) {
		// free any per-frame resources before destroying the frame data
		_frames[i]._deletionQueue.flush();

		if (_frames[i]._commandPool != VK_NULL_HANDLE)
			vkDestroyCommandPool(device.vk_device, _frames[i]._commandPool, nullptr);

		if (_frames[i]._renderFence != VK_NULL_HANDLE)
			vkDestroyFence(device.vk_device, _frames[i]._renderFence, nullptr);
		if (_frames[i]._swapchainSemaphore != VK_NULL_HANDLE)
			vkDestroySemaphore(device.vk_device, _frames[i]._swapchainSemaphore, nullptr);
	}

	for (VkSemaphore semaphore : render_semaphores) {
		if (semaphore != VK_NULL_HANDLE) vkDestroySemaphore(device.vk_device, semaphore, nullptr);
	}

	// flush the global deletion queue (draw image, descriptors, pipelines, allocator).
	// This must run while the device is still alive but before it is destroyed.
	lifetime_deletion_queue.flush();

	Vulkan::Bootstrap::destroy_swapchain(swapchain);
	Vulkan::Bootstrap::destroy_device(device);

	if (surface != VK_NULL_HANDLE) vkDestroySurfaceKHR(instance.instance, surface, nullptr);

	Vulkan::Bootstrap::destroy_instance(instance);
}

void Vulkan::Renderer::init_vulkan(EngineInstance &engine, GLFWwindow *glfw_window) {
	Vulkan::Bootstrap::InstanceBuilder builder;

    // glfw requires some vulkan extensions
    uint32_t glfw_ext_count = 0;
    const char **glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_ext_count);
		 
    auto instance_return = builder.set_app_name(Engine::VERSION)
        .request_validation_layers(true)
        .use_default_debug_messenger()
        .require_api_version(VK_API_VERSION_1_3)
        .enable_extensions(glfw_extensions, glfw_ext_count)
        .build();

    if (!instance_return.has_value()) throw std::runtime_error("failed to create Vulkan instance");
    instance = instance_return.value();

    if (glfwCreateWindowSurface(instance.instance, glfw_window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }

	VkPhysicalDeviceVulkan13Features features_1_3{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	features_1_3.dynamicRendering = true;
	features_1_3.synchronization2 = true;

	VkPhysicalDeviceVulkan12Features features_1_2{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
	features_1_2.bufferDeviceAddress = true;
	features_1_2.descriptorIndexing = true;

	Vulkan::Bootstrap::PhysicalDeviceSelector selector(instance, surface);
	auto physical_device_return = selector.prefer_discrete(true).select();
	if (!physical_device_return.has_value()) throw std::runtime_error("failed to select a Vulkan physical device");
	Vulkan::PhysicalDevice selected = physical_device_return.value();

	Vulkan::Bootstrap::DeviceBuilder device_builder(selected);
	auto device_return = device_builder
		.add_pnext(&features_1_3)
		.add_pnext(&features_1_2)
		.build();
	if (!device_return.has_value()) throw std::runtime_error("failed to create Vulkan device");

	device = device_return.value();
	physical_device = selected.vk_device;
	queue_family = selected.graphics_queue_family;

	// initialize the memory allocator. We give VMA the volk-loaded
	// vkGetInstanceProcAddr/vkGetDeviceProcAddr so it can resolve the rest of the
	// functions it needs (we build with VK_NO_PROTOTYPES). The BUFFER_DEVICE_ADDRESS
	// flag lets us use GPU pointers later.
	VmaVulkanFunctions vulkan_functions = {};
	vulkan_functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
	vulkan_functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

	VmaAllocatorCreateInfo allocator_info = {};
	allocator_info.physicalDevice = physical_device;
	allocator_info.device = device.vk_device;
	allocator_info.instance = instance.instance;
	allocator_info.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
	allocator_info.pVulkanFunctions = &vulkan_functions;
	vmaCreateAllocator(&allocator_info, &allocator);

	lifetime_deletion_queue.push_function([this]() { vmaDestroyAllocator(allocator); });

	int fb_width = 0;
	int fb_height = 0;
	glfwGetFramebufferSize(glfw_window, &fb_width, &fb_height);
	create_swapchain((uint32_t)fb_width, (uint32_t)fb_height);

	init_commands();
	init_sync_structures();
	init_descriptors();
	init_pipelines();
	init_imgui(glfw_window);
}

void Vulkan::Renderer::create_swapchain(uint32_t width, uint32_t height) {
	Vulkan::Bootstrap::SwapchainBuilder swapchain_builder(device, surface);

	auto swapchain_return = swapchain_builder
		.set_desired_format(VkSurfaceFormatKHR{ .format = VK_FORMAT_B8G8R8A8_UNORM, .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
		.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
		.set_desired_extent(width, height)
		.set_image_usage_flags(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		.build();

	if (!swapchain_return.has_value()) throw std::runtime_error("failed to create Vulkan swapchain");
	swapchain = swapchain_return.value();

	// draw image size matches the window. We hardcode RGBA 16-bit float for the extra
	// precision that helps with lighting calculations and avoids banding.
	VkExtent3D drawImageExtent = {width, height, 1};
	draw_image.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
	draw_image.imageExtent = drawImageExtent;

	// TransferSRC/DST to copy from and into it, Storage so a compute shader can write
	// to it, ColorAttachment so graphics pipelines can draw into it.
	VkImageUsageFlags drawImageUsages = 0;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_STORAGE_BIT;
	drawImageUsages |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkImageCreateInfo rimg_info = Vulkan::init::image_create_info(draw_image.imageFormat, drawImageUsages, drawImageExtent);

	// allocate the draw image from gpu local memory (VRAM) for fastest access
	VmaAllocationCreateInfo rimg_allocinfo = {};
	rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vmaCreateImage(allocator, &rimg_info, &rimg_allocinfo, &draw_image.image, &draw_image.allocation, nullptr);

	// build the default image view we pair with the image to access it
	VkImageViewCreateInfo rview_info = Vulkan::init::imageview_create_info(draw_image.imageFormat, draw_image.image, VK_IMAGE_ASPECT_COLOR_BIT);
	VK_CHECK(vkCreateImageView(device.vk_device, &rview_info, nullptr, &draw_image.imageView));

	lifetime_deletion_queue.push_function([this]() {
		vkDestroyImageView(device.vk_device, draw_image.imageView, nullptr);
		vmaDestroyImage(allocator, draw_image.image, draw_image.allocation);
	});
}


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

void Vulkan::Renderer::draw() {
	// wait until the gpu has finished rendering the last frame. Timeout of 1 second
	// (in nanoseconds). Fences have to be reset between uses.
	VK_CHECK(vkWaitForFences(device.vk_device, 1, &get_current_frame()._renderFence, true, 1000000000));

	// the fence is signalled, so the gpu finished this frame's work: safe to flush any
	// per-frame resources now.
	get_current_frame()._deletionQueue.flush();

	VK_CHECK(vkResetFences(device.vk_device, 1, &get_current_frame()._renderFence));

	// request image from the swapchain. We send the _swapchainSemaphore so we can
	// sync our render commands to the swapchain image being ready.
	uint32_t swapchainImageIndex;
	VK_CHECK(vkAcquireNextImageKHR(device.vk_device, swapchain.swapchain, 1000000000,
								   get_current_frame()._swapchainSemaphore, nullptr, &swapchainImageIndex));

	// naming it cmd for shorter writing. Vulkan handles are just 64-bit pointers,
	// so copying them around is fine.
	VkCommandBuffer cmd = get_current_frame()._mainCommandBuffer;

	// now that the commands finished executing (we waited on the fence), we can
	// safely reset the command buffer to begin recording again.
	VK_CHECK(vkResetCommandBuffer(cmd, 0));

	// the draw region matches our off-swapchain draw image
	draw_extent.width = draw_image.imageExtent.width;
	draw_extent.height = draw_image.imageExtent.height;

	// we will use this command buffer exactly once, so tell vulkan that for a
	// possible small speedup in command encoding.
	VkCommandBufferBeginInfo cmdBeginInfo = Vulkan::init::command_buffer_begin_info(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

	// transition our main draw image into general layout so we can write into it. We
	// will overwrite it all so we dont care about the older layout.
	Vulkan::util::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

	// record the actual draw commands into the draw image
	// boy i sure hope this shit isn't null!
	draw_pipeline(cmd, *gradient_pipeline.get());

	// transition the draw image and the swapchain image into their transfer layouts,
	// then blit (copy) the draw image into the swapchain image.
	Vulkan::util::transition_image(cmd, draw_image.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
	Vulkan::util::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	Vulkan::util::copy_image_to_image(cmd, draw_image.image, swapchain.images[swapchainImageIndex], draw_extent, swapchain.extent);

	// draw imgui directly onto the swapchain image. We need it in color-attachment
	// layout for dynamic rendering, then transition to present.
	Vulkan::util::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	draw_imgui(cmd, swapchain.image_views[swapchainImageIndex]);

	// make the swapchain image into presentable mode. PRESENT_SRC_KHR is the only
	// layout the swapchain allows for presenting to screen.
	Vulkan::util::transition_image(cmd, swapchain.images[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

	// finalize the command buffer (we can no longer add commands, but it can now be executed)
	VK_CHECK(vkEndCommandBuffer(cmd));

	// prepare the submission to the queue. We wait on the _swapchainSemaphore, as it
	// is signaled when the swapchain image is ready, and we signal the _renderSemaphore
	// to indicate that rendering has finished.
	VkCommandBufferSubmitInfo cmdinfo = Vulkan::init::command_buffer_submit_info(cmd);
	VkSemaphoreSubmitInfo waitInfo =
		Vulkan::init::semaphore_submit_info(VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR, get_current_frame()._swapchainSemaphore);
	VkSemaphoreSubmitInfo signalInfo =
		Vulkan::init::semaphore_submit_info(VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT, render_semaphores[swapchainImageIndex]);
	VkSubmitInfo2 submit = Vulkan::init::submit_info(&cmdinfo, &signalInfo, &waitInfo);

	// submit command buffer to the queue and execute it. _renderFence will now block
	// until the graphic commands finish execution. This is how we sync gpu to cpu.
	VK_CHECK(vkQueueSubmit2(device.graphics_queue, 1, &submit, get_current_frame()._renderFence));

	// prepare present. This puts the image we just rendered into the visible window.
	// We wait on the _renderSemaphore so drawing finishes before the image is shown.
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.pSwapchains = &swapchain.swapchain;
	presentInfo.swapchainCount = 1;
	presentInfo.pWaitSemaphores = &render_semaphores[swapchainImageIndex];
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pImageIndices = &swapchainImageIndex;

	VK_CHECK(vkQueuePresentKHR(device.graphics_queue, &presentInfo));

	// increase the number of frames drawn
	frame_number++;
}

void Vulkan::Renderer::draw_pipeline(VkCommandBuffer vk_buffer, Vulkan::pipeline::ComputePipeline pipeline) {
	// bind the gradient compute pipeline and the descriptor set holding the draw image,
	// then dispatch. The shader uses a 16x16 workgroup, so we divide the draw resolution
	// by 16 and round up.
	vkCmdBindPipeline(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_pipeline);
	vkCmdBindDescriptorSets(vk_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.vk_layout, 0, 1, &draw_image_descriptors, 0, nullptr);
	vkCmdDispatch(vk_buffer, (uint32_t)std::ceil(draw_extent.width / 16.0), (uint32_t)std::ceil(draw_extent.height / 16.0), 1);
}

void Vulkan::Renderer::draw_imgui(VkCommandBuffer cmd, VkImageView target_image_view) {
	// dynamic rendering pass that loads the existing swapchain contents (the blitted
	// gradient) and draws the imgui draw data on top of it.
	VkRenderingAttachmentInfo colorAttachment = Vulkan::init::attachment_info(target_image_view, nullptr, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	VkRenderingInfo renderInfo = Vulkan::init::rendering_info(swapchain.extent, &colorAttachment, nullptr);

	vkCmdBeginRendering(cmd, &renderInfo);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	vkCmdEndRendering(cmd);
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

void Vulkan::Renderer::init_pipelines() { init_background_pipeline(); }

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
		Vulkan::pipeline::ComputePipeline* pipeline = this->gradient_pipeline.get();
		if (pipeline != NULL) pipeline->destroy();
	});
}

void Vulkan::Renderer::init_imgui(GLFWwindow *glfw_window) { {
	// 1: create descriptor pool for IMGUI
	//  the size of the pool is very oversize, but it's copied from imgui demo
	//  itself.
	VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 } };

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

	//dynamic rendering parameters for imgui to use
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