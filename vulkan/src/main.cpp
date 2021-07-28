#include <common/common.hpp>
#include <common/math.hpp>

#define _AMD64_
#define UNICODE
#include <minwindef.h>
#include <WinDef.h>
#include <WinUser.h>
#include <WinBase.h>

#include <unordered_set>
template <typename T>
using Set = std::unordered_set<T>;

#define VOLK_IMPLEMENTATION
#define VK_NO_PROTOTYPES
#define VK_USE_PLATFORM_WIN32_KHR
#include "volk.h"


LRESULT CALLBACK main_window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
    if(message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    else if(message == WM_ERASEBKGND) {
        return 1;
    }
    else {
        return DefWindowProcW(window, message, w_param, l_param);
    }
}


static VKAPI_ATTR VkBool32 VKAPI_CALL vk_debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData
) {
    UNUSED(messageSeverity);
    UNUSED(messageType);
    UNUSED(pUserData);
    printf("\n[Validation]: %s\n\n", pCallbackData->pMessage);
    return VK_FALSE;
}


int main() {
    HeapSetInformation(nullptr, HeapEnableTerminationOnCorruption, nullptr, 0);

    auto win32_instance = GetModuleHandle(nullptr);


    // Vulkan setup.

    auto volk_init_result = volkInitialize();
    assert(volk_init_result == VK_SUCCESS);

    // print available layers.
    {
        auto layer_count = Uint32(0);
        auto result = vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
        assert(result == VK_SUCCESS);

        auto layers = List<VkLayerProperties>(layer_count);
        result = vkEnumerateInstanceLayerProperties(&layer_count, layers.data());
        assert(result == VK_SUCCESS);

        printf("Layers:\n");
        for(Uint32 i = 0; i < layer_count; i += 1) {
            auto& layer = layers[i];
            printf("%s\n", layer.layerName);
        }
        printf("\n");
    }


    auto enabled_layers = List<const char*> {
        "VK_LAYER_KHRONOS_validation",
    };


    auto vk_instance = VkInstance();
    {
        auto enabled_extensions = List<const char*> {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        };

        auto app_info = VkApplicationInfo();
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pNext = nullptr;
        app_info.pApplicationName = "";
        app_info.pEngineName = "";
        app_info.apiVersion = VK_MAKE_VERSION(1, 0, 0);

        auto create_info = VkInstanceCreateInfo();
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledLayerCount = Uint32(enabled_layers.size());
        create_info.ppEnabledLayerNames = enabled_layers.data();
        create_info.enabledExtensionCount = Uint32(enabled_extensions.size());
        create_info.ppEnabledExtensionNames = enabled_extensions.data();

        auto result = vkCreateInstance(&create_info, nullptr, &vk_instance);
        assert(result == VK_SUCCESS);

        volkLoadInstanceOnly(vk_instance);
    }
    defer { vkDestroyInstance(vk_instance, nullptr); };


    auto vk_debug_messenger = VkDebugUtilsMessengerEXT();
    {
        auto create_info = VkDebugUtilsMessengerCreateInfoEXT();
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;

        create_info.messageSeverity =
              0&VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
            | 0&VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

        create_info.messageType =
              VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
            | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

        create_info.pfnUserCallback = vk_debug_callback;
        create_info.pUserData = nullptr;

        auto result = vkCreateDebugUtilsMessengerEXT(vk_instance, &create_info, nullptr, &vk_debug_messenger);
        assert(result == VK_SUCCESS);
    }
    defer { vkDestroyDebugUtilsMessengerEXT(vk_instance, vk_debug_messenger, nullptr); };


    auto win32_window = HWND();
    {
        auto window_class = WNDCLASSW {};
        window_class.style         = CS_HREDRAW | CS_VREDRAW;
        window_class.lpfnWndProc   = main_window_proc;
        window_class.hCursor       = LoadCursorW(0, IDC_ARROW);
        window_class.hIcon         = LoadIconW(0, IDI_APPLICATION);
        window_class.hInstance     = win32_instance;
        window_class.lpszClassName = L"raster_main_window_class";

        auto window_class_atom = RegisterClassW(&window_class);
        assert(window_class_atom != 0);


        win32_window = CreateWindowExW(
            0,
            MAKEINTATOM(window_class_atom),
            L"Raster",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            nullptr, nullptr, win32_instance, nullptr
        );
        assert(win32_window != 0);
    }


    auto vk_surface = VkSurfaceKHR();
    {
        auto create_info = VkWin32SurfaceCreateInfoKHR();
        create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        create_info.hwnd = win32_window;
        create_info.hinstance = win32_instance;

        auto result = vkCreateWin32SurfaceKHR(vk_instance, &create_info, nullptr, &vk_surface);
        assert(result == VK_SUCCESS);
    }
    defer { vkDestroySurfaceKHR(vk_instance, vk_surface, nullptr); };



    auto vk_surface_format = VkSurfaceFormatKHR();
    vk_surface_format.format = VK_FORMAT_B8G8R8A8_SRGB;
    vk_surface_format.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

    auto vk_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    auto vk_device = VkDevice();
    auto vk_graphics_queue_index = Uint32();
    auto vk_graphics_queue = VkQueue();
    auto vk_present_queue_index = Uint32();
    auto vk_present_queue = VkQueue();
    auto vk_surface_extent = VkExtent2D();
    auto vk_swapchain_image_count = Uint32();
    auto vk_swapchain_pre_transform = VkSurfaceTransformFlagBitsKHR();
    {
        // TODO: check support in device selection.
        auto enabled_extensions = List<const char*> {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        };

        auto device_count = Uint32(0);
        auto result = vkEnumeratePhysicalDevices(vk_instance, &device_count, nullptr);
        assert(result == VK_SUCCESS);
        assert(device_count >= 1);

        auto devices = List<VkPhysicalDevice>(device_count);
        result = vkEnumeratePhysicalDevices(vk_instance, &device_count, devices.data());
        assert(result == VK_SUCCESS);

        auto best_device_index = Uint32(-1);
        auto graphics_queue_index = Uint32(-1);
        auto present_queue_index = Uint32(-1);

        printf("Devices:\n");
        for(Uint32 device_index = 0; device_index < device_count; device_index++) {
            auto device = devices[device_index];

            auto properties = VkPhysicalDeviceProperties();
            vkGetPhysicalDeviceProperties(device, &properties);

            // Choose first discrete or last gpu.
            if(best_device_index == Uint32(-1)) {
                auto is_discrete_gpu = properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
                auto is_last_gpu = device_index == (device_count - 1);
                if(is_discrete_gpu || is_last_gpu) {
                    best_device_index = device_index;
                }
            }

            if(best_device_index == device_index) {
                printf("(chosen device)\n");
            }

            printf("Device Name:    %s\n", properties.deviceName);
            printf("Device Type:    %d\n", properties.deviceType);
            printf("Driver Version: %d\n", properties.driverVersion);
            printf("API Version:    %d.%d.%d\n",
                VK_VERSION_MAJOR(properties.apiVersion),
                VK_VERSION_MINOR(properties.apiVersion),
                VK_VERSION_PATCH(properties.apiVersion)
            );


            auto queue_family_count = Uint32(0);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

            auto queue_families = List<VkQueueFamilyProperties>(queue_family_count);
            vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

            printf("Queue families:\n");
            for(Uint32 queue_index = 0; queue_index < queue_family_count; queue_index += 1) {
                auto flags = queue_families[queue_index].queueFlags;

                if(device_index == best_device_index) {
                    if(graphics_queue_index == Uint32(-1)) {
                        if((flags & VK_QUEUE_GRAPHICS_BIT) != 0) {
                            printf("(chosen graphics queue)\n");
                            graphics_queue_index = queue_index;
                        }
                    }


                    if(present_queue_index == Uint32(-1)) {
                        auto supports_present = VkBool32(false);
                        auto result = vkGetPhysicalDeviceSurfaceSupportKHR(device, queue_index, vk_surface, &supports_present);
                        assert(result == VK_SUCCESS);

                        if(supports_present) {
                            printf("(chosen present queue)\n");
                            present_queue_index = queue_index;
                        }
                    }
                }


                printf("%d ", queue_index);
                if((flags & VK_QUEUE_GRAPHICS_BIT) != 0) { printf("Graphics "); }
                if((flags & VK_QUEUE_COMPUTE_BIT) != 0) { printf("Compute "); }
                if((flags & VK_QUEUE_TRANSFER_BIT) != 0) { printf("Transfer "); }
                if((flags & VK_QUEUE_SPARSE_BINDING_BIT) != 0) { printf("Sparse-Binding "); }
                if((flags & VK_QUEUE_PROTECTED_BIT) != 0) { printf("Protected "); }
                printf("\n");
            }

            auto extension_count = Uint32();
            auto result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
            assert(result == VK_SUCCESS);

            auto extensions = List<VkExtensionProperties>(extension_count);
            result = vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data());

            #if 0
            printf("Extensions:\n");
            for(auto& extension : extensions) {
                printf("%s\n", extension.extensionName);
            }
            #endif

            printf("\n");
        }
        printf("\n");

        assert(best_device_index != Uint32(-1));
        assert(graphics_queue_index != Uint32(-1));
        assert(present_queue_index != Uint32(-1));

        vk_graphics_queue_index = graphics_queue_index;
        vk_present_queue_index  = present_queue_index;


        auto queue_priorities = Float32(1.0f);
        auto queue_create_infos = List<VkDeviceQueueCreateInfo>();

        auto unique_queue_families = Set<Uint32>{graphics_queue_index, present_queue_index};
        for(auto family_index : unique_queue_families) {
            auto create_info = VkDeviceQueueCreateInfo();
            create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            create_info.queueFamilyIndex = family_index;
            create_info.queueCount = 1;
            create_info.pQueuePriorities = &queue_priorities;
            queue_create_infos.push_back(create_info);
        }

        auto device_features = VkPhysicalDeviceFeatures();

        auto create_info = VkDeviceCreateInfo();
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = Uint32(queue_create_infos.size());
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.enabledLayerCount = Uint32(enabled_layers.size());
        create_info.ppEnabledLayerNames = enabled_layers.data();
        create_info.enabledExtensionCount = Uint32(enabled_extensions.size());
        create_info.ppEnabledExtensionNames = enabled_extensions.data();
        create_info.pEnabledFeatures = &device_features;

        auto vk_physical_device = devices[best_device_index];

        result = vkCreateDevice(vk_physical_device, &create_info, nullptr, &vk_device);
        assert(result == VK_SUCCESS);

        volkLoadDevice(vk_device);

        vkGetDeviceQueue(vk_device, graphics_queue_index, 0, &vk_graphics_queue);
        vkGetDeviceQueue(vk_device, present_queue_index, 0, &vk_present_queue);


        auto surface_capabilities = VkSurfaceCapabilitiesKHR();
        result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
            vk_physical_device, vk_surface, &surface_capabilities
        );
        assert(result == VK_SUCCESS);

        assert(surface_capabilities.currentExtent.width  != UINT32_MAX);
        assert(surface_capabilities.currentExtent.height != UINT32_MAX);
        vk_surface_extent = surface_capabilities.currentExtent;

        vk_swapchain_image_count = surface_capabilities.minImageCount + 1;
        if(surface_capabilities.maxImageCount > 0) {
            vk_swapchain_image_count = min(vk_swapchain_image_count, surface_capabilities.maxImageCount);
        }

        vk_swapchain_pre_transform = surface_capabilities.currentTransform;


        auto surface_format_count = Uint32();
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &surface_format_count, nullptr);
        assert(result == VK_SUCCESS);
        assert(surface_format_count > 0);

        auto surface_formats = List<VkSurfaceFormatKHR>(surface_format_count);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, vk_surface, &surface_format_count, surface_formats.data());
        assert(result == VK_SUCCESS);


        auto present_mode_count = Uint32();
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count, nullptr);
        assert(result == VK_SUCCESS);
        assert(present_mode_count > 0);

        auto present_modes = List<VkPresentModeKHR>(present_mode_count);
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(vk_physical_device, vk_surface, &present_mode_count, present_modes.data());
        assert(result == VK_SUCCESS);


        auto found_format = false;
        for(auto format : surface_formats) {
            if(    format.format     == vk_surface_format.format
                && format.colorSpace == vk_surface_format.colorSpace
            ) {
                found_format = true;
                break;
            }
        }
        assert(found_format);

        auto found_present_mode = false;
        for(auto present_mode : present_modes) {
            if(present_mode == vk_present_mode) {
                found_present_mode = true;
                break;
            }
        }
        assert(found_present_mode);

    }
    defer { vkDestroyDevice(vk_device, nullptr); };


    auto vk_swapchain = VkSwapchainKHR();
    auto vk_swapchain_images = List<VkImage>();
    auto vk_swapchain_image_views = List<VkImageView>();
    {
        auto queue_family_indices = std::array<Uint32, 2> {
            vk_graphics_queue_index,
            vk_present_queue_index,
        };

        auto create_info = VkSwapchainCreateInfoKHR();
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = vk_surface;
        create_info.minImageCount = vk_swapchain_image_count;
        create_info.imageFormat = vk_surface_format.format;
        create_info.imageColorSpace = vk_surface_format.colorSpace;
        create_info.imageExtent = vk_surface_extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        if(vk_graphics_queue_index != vk_present_queue_index) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = (Uint32)queue_family_indices.size();
            create_info.pQueueFamilyIndices = &queue_family_indices[0];
        }
        else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = nullptr;
        }
        create_info.preTransform = vk_swapchain_pre_transform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = vk_present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = VK_NULL_HANDLE;

        auto result = vkCreateSwapchainKHR(vk_device, &create_info, nullptr, &vk_swapchain);
        assert(result == VK_SUCCESS);

        auto image_count = Uint32();
        result = vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &image_count, nullptr);
        assert(result == VK_SUCCESS);

        vk_swapchain_images.resize(image_count);
        result = vkGetSwapchainImagesKHR(vk_device, vk_swapchain, &image_count, vk_swapchain_images.data());
        assert(result == VK_SUCCESS);

        vk_swapchain_image_views.resize(image_count);
        for(Uint32 i = 0; i < image_count; i += 1) {
            auto create_info = VkImageViewCreateInfo();
            create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            create_info.image = vk_swapchain_images[i];
            create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            create_info.format = vk_surface_format.format;
            create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            create_info.subresourceRange.baseMipLevel = 0;
            create_info.subresourceRange.levelCount = 1;
            create_info.subresourceRange.baseArrayLayer = 0;
            create_info.subresourceRange.layerCount = 1;

            auto result = vkCreateImageView(vk_device, &create_info, nullptr, &vk_swapchain_image_views[i]);
            assert(result == VK_SUCCESS);
        }
    }
    defer { vkDestroySwapchainKHR(vk_device, vk_swapchain, nullptr); };
    defer {
        for(auto image_view : vk_swapchain_image_views) {
            vkDestroyImageView(vk_device, image_view, nullptr);
        }
    };


    auto vk_pipeline_layout = VkPipelineLayout();
    auto vk_pipeline = VkPipeline();
    {
        #include "shaders.inl"

        auto vertex_shader_module = VkShaderModule();
        {
            auto create_info = VkShaderModuleCreateInfo();
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = vertex_shader.size();
            create_info.pCode = (const Uint32*)vertex_shader.data();

            auto result = vkCreateShaderModule(vk_device, &create_info, nullptr, &vertex_shader_module);
            assert(result == VK_SUCCESS);
        }
        defer { vkDestroyShaderModule(vk_device, vertex_shader_module, nullptr); };

        auto vertex_shader_stage_create_info = VkPipelineShaderStageCreateInfo();
        vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertex_shader_stage_create_info.module = vertex_shader_module;
        vertex_shader_stage_create_info.pName = "main";


        auto fragment_shader_module = VkShaderModule();
        {
            auto create_info = VkShaderModuleCreateInfo();
            create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            create_info.codeSize = fragment_shader.size();
            create_info.pCode = (const Uint32*)fragment_shader.data();

            auto result = vkCreateShaderModule(vk_device, &create_info, nullptr, &fragment_shader_module);
            assert(result == VK_SUCCESS);
        }
        defer { vkDestroyShaderModule(vk_device, fragment_shader_module, nullptr); };

        auto fragment_shader_stage_create_info = VkPipelineShaderStageCreateInfo();
        fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragment_shader_stage_create_info.module = fragment_shader_module;
        fragment_shader_stage_create_info.pName = "main";


        VkPipelineShaderStageCreateInfo shader_stages[] = {
            vertex_shader_stage_create_info,
            fragment_shader_stage_create_info,
        };


        auto vertex_input_create_info = VkPipelineVertexInputStateCreateInfo();
        vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_create_info.vertexBindingDescriptionCount = 0;
        vertex_input_create_info.pVertexBindingDescriptions = nullptr;
        vertex_input_create_info.vertexAttributeDescriptionCount = 0;
        vertex_input_create_info.pVertexAttributeDescriptions = nullptr;


        auto input_assembly_create_info = VkPipelineInputAssemblyStateCreateInfo();
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;


        auto viewport = VkViewport();
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width  = (Float32)vk_surface_extent.width;
        viewport.height = (Float32)vk_surface_extent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        auto scissor = VkRect2D();
        scissor.offset = {0, 0};
        scissor.extent = vk_surface_extent;

        auto viewport_create_info = VkPipelineViewportStateCreateInfo();
        viewport_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_create_info.viewportCount = 1;
        viewport_create_info.pViewports = &viewport;
        viewport_create_info.scissorCount = 1;
        viewport_create_info.pScissors = &scissor;


        auto rasterizer_create_info = VkPipelineRasterizationStateCreateInfo();
        rasterizer_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer_create_info.depthClampEnable = VK_FALSE;
        rasterizer_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterizer_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer_create_info.depthBiasEnable = VK_FALSE;
        rasterizer_create_info.depthBiasConstantFactor = 0.0f;
        rasterizer_create_info.depthBiasClamp = 0.0f;
        rasterizer_create_info.depthBiasSlopeFactor = 0.0f;
        rasterizer_create_info.lineWidth = 1.0f;


        auto multisample_create_info = VkPipelineMultisampleStateCreateInfo();
        multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisample_create_info.sampleShadingEnable = VK_FALSE;
        multisample_create_info.minSampleShading = 1.0f;
        multisample_create_info.pSampleMask = nullptr;
        multisample_create_info.alphaToCoverageEnable = VK_FALSE;
        multisample_create_info.alphaToOneEnable = VK_FALSE;


        auto color_blending = VkPipelineColorBlendAttachmentState();
        color_blending.blendEnable = VK_FALSE;
        color_blending.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blending.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blending.colorBlendOp = VK_BLEND_OP_ADD;
        color_blending.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blending.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blending.alphaBlendOp = VK_BLEND_OP_ADD;
        color_blending.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


        auto blend_create_info = VkPipelineColorBlendStateCreateInfo();
        blend_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend_create_info.logicOpEnable = VK_FALSE;
        blend_create_info.logicOp = VK_LOGIC_OP_COPY;
        blend_create_info.attachmentCount = 1;
        blend_create_info.pAttachments = &color_blending;
        blend_create_info.blendConstants[0] = 0.0f;
        blend_create_info.blendConstants[1] = 0.0f;
        blend_create_info.blendConstants[2] = 0.0f;
        blend_create_info.blendConstants[3] = 0.0f;


        auto dynamic_state = std::array<VkDynamicState, 1>{
            VK_DYNAMIC_STATE_VIEWPORT
        };

        auto dynamic_state_create_info = VkPipelineDynamicStateCreateInfo();
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = (Uint32)dynamic_state.size();
        dynamic_state_create_info.pDynamicStates = &dynamic_state[0];


        auto pipeline_layout_create_info = VkPipelineLayoutCreateInfo();
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.setLayoutCount = 0;
        pipeline_layout_create_info.pSetLayouts = nullptr;
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;

        auto result = vkCreatePipelineLayout(vk_device, &pipeline_layout_create_info, nullptr, &vk_pipeline_layout);
        assert(result == VK_SUCCESS);

    }
    defer { vkDestroyPipelineLayout(vk_device, vk_pipeline_layout, nullptr); };
    defer { vkDestroyPipeline(vk_device, vk_pipeline, nullptr); };


    ShowWindow(win32_window, SW_SHOWDEFAULT);
    UpdateWindow(win32_window);


    // Main loop.

    auto exit_code = 0;

    while(true) {
        auto msg = MSG();
        auto res = GetMessageW(&msg, 0, 0, 0);

        if(res == 0) {
            exit_code = (int)msg.wParam;
            break;
        }
        else if(res == -1) {
            printf("Error: GetMessage failed (%d)\n", GetLastError());
            ExitProcess(1);
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return exit_code;
}
