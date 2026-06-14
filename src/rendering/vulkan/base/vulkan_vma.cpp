// VMA holds both the header and the implementation of its functions in the same
// header file. VMA_IMPLEMENTATION must be defined in exactly one .cpp of the project.
//
// Because the project builds with VK_NO_PROTOTYPES and resolves entrypoints through
// volk at runtime, VMA must not reference statically-linked vulkan functions. We turn
// off static functions and let VMA fetch what it needs dynamically from the
// vkGetInstanceProcAddr / vkGetDeviceProcAddr pointers we hand it at allocator creation.
#include <volk.h>

#define VMA_IMPLEMENTATION
#define VMA_STATIC_VULKAN_FUNCTIONS 0
#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1
#include <vk_mem_alloc.h>
