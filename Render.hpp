#include <iostream>
#include <fstream>
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <string>
#include <vector>
#include <array>
#include <thread>

#if defined(__ANDROID__)
#include <android/native_activity.h>
#include "android/native_window.h"
#include "android/native_window_jni.h"
#include "android/hardware_buffer_jni.h"
#include "vulkan/vulkan_android.h"
#elif defined(_WIN32) || defined(__linux__)
#include "GLFW/glfw3.h"
#endif

#include "ResourceLoader.hpp"

struct vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 tangent;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};

        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(vertex, position);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(vertex, uv);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(vertex, normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(vertex, tangent);

        return attributeDescriptions;
    }
};

struct UniformBufferObject {
    alignas(16) glm::vec4 useLookAt;
    alignas(16) glm::vec4 resolution;
    alignas(16) glm::vec4 cameraPosition;
    alignas(16) glm::mat4 projection;
    alignas(16) glm::mat4 translate;
    alignas(16) glm::mat4 rotx;
    alignas(16) glm::mat4 roty;

    alignas(16) glm::mat4 mtranslate;
    alignas(16) glm::mat4 mroty;
    alignas(16) glm::mat4 mrotx;
    alignas(16) glm::mat4 mrotz;
    alignas(16) glm::mat4 mscale;

    alignas(16) glm::mat4 sprojection;
    alignas(16) glm::mat4 stranslate;
    alignas(16) glm::mat4 srotx;
    alignas(16) glm::mat4 sroty;
    alignas(16) glm::vec4 lightPos[10];
    alignas(16) glm::vec4 lightColor[10];
};

class Render {
private:
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkImage ShadowRenderImage;
    VkDeviceMemory ShadowRenderImageMemory;
    VkImageView ShadowRenderImageView;

    VkImage MainImage;
    VkDeviceMemory MainImageMemory;
    VkImageView MainImageView;

    VkImage MaindImage;
    VkDeviceMemory MaindImageMemory;
    VkImageView MaindImageView;

    int writeapiver = 0;

    bool cfw = false;
    void createInstance(std::string appname) {
        std::ifstream readcfg{};
        std::ofstream writecfg{};
        readcfg.open(pathprefix + "eng/cfg/Render.cfg");

        if (!readcfg.is_open()) {
            std::cout << "log:\u001b[31m failed to read Render configuration, creating a new one...\u001b[37m" << std::endl;
            cfw = true;
            writecfg.open(pathprefix + "eng/cfg/Render.cfg");
        }

        std::cout << "log:\u001b[36m instance creating began\u001b[37m" << std::endl;
        VkApplicationInfo appinfo{};
        appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appinfo.pApplicationName = appname.c_str();
        appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appinfo.pEngineName = "MagmaTech";
        appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appinfo.apiVersion = VK_API_VERSION_1_0;

        if (!cfw) {
            std::string nm{};
            int argument;
            while (readcfg >> nm >> argument) {
                if (nm == "vkver") {
                    appinfo.apiVersion = argument;
                    writeapiver = argument;
                }
            }
        }
        else {
            writecfg << "vkver " << VK_API_VERSION_1_3 << std::endl;
            writeapiver = VK_API_VERSION_1_3;
        }


        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appinfo;

#if defined(__ANDROID__)
        std::vector<const char*> instance_extensions;

        instance_extensions.push_back("VK_KHR_surface");
        instance_extensions.push_back("VK_KHR_android_surface");

        createInfo.enabledExtensionCount = instance_extensions.size();
        createInfo.ppEnabledExtensionNames = instance_extensions.data();
#elif defined(_WIN32) || defined(__linux__)
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
#endif
        uint32_t layercont = 0;
        std::vector<VkLayerProperties> lprop{};
        vkEnumerateInstanceLayerProperties(&layercont, nullptr);
        lprop.resize(layercont);
        vkEnumerateInstanceLayerProperties(&layercont, lprop.data());
        std::vector<char*>layerNames{};
        layerNames.resize(layercont);
        for (int i = 0; i != layercont; i++) {
            layerNames[i] = lprop[i].layerName;
            std::cout << "log:\u001b[36m Enabling Instance Layer:" << lprop[i].layerName << "\u001b[37m" << std::endl;
        }

        createInfo.ppEnabledLayerNames = layerNames.data();
        createInfo.enabledLayerCount = layerNames.size();

        if (!uselayer) {
            createInfo.enabledLayerCount = 0;
        }

        VkResult res = vkCreateInstance(&createInfo, nullptr, &instance);
        std::cout << "log:\u001b[32m instance created with code \u001b[37m" << res << std::endl;
    }
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDeviceQueueCreateInfo queueinfo{};
    uint32_t queueFamilyIndex;
    int choseddevice;
    void getDevice() {
        std::fstream cfgwork{};
        cfgwork.open(pathprefix + "eng/cfg/Render.cfg", std::ios_base::app);

        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0) {
            std::cout << "error: cannot find an vulkan capable device" << std::endl;
        }
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        choseddevice = deviceCount - 1;

        if (!cfw) {
            std::string param;
            int arg;
            while (cfgwork >> param >> arg) {
                if (param == "vkphysdev") {
                    choseddevice = arg;
                }
            }
        }
        else {
            cfgwork << "vkphysdev " << choseddevice << std::endl;
            cfgwork << "wsizex 800" << std::endl;
            cfgwork << "wsizey 600" << std::endl;
            cfgwork << "wfull 0" << std::endl;
        }

        physicalDevice = devices[choseddevice];

        VkPhysicalDeviceProperties physprop{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physprop);
        std::cout << "log:\u001b[36m device name = " << physprop.deviceName << "\u001b[37m" << std::endl;
        std::cout << "log:\u001b[36m device type = " << physprop.deviceType << "\u001b[37m" << std::endl;
        devicename = physprop.deviceName;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

        for (int i = 0; i != queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                queueFamilyIndex = i;
                break;
            }
        }

        const float prior = 1.0f;
        queueinfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueinfo.pNext = NULL;
        queueinfo.queueCount = 1;
        queueinfo.pQueuePriorities = &prior;
        queueinfo.queueFamilyIndex = queueFamilyIndex;

        VkDeviceCreateInfo createInfo{};

        std::vector<const char*> device_extensions;

        device_extensions.push_back("VK_KHR_swapchain");

        createInfo.enabledExtensionCount = device_extensions.size();
        createInfo.ppEnabledExtensionNames = device_extensions.data();

        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.pQueueCreateInfos = &queueinfo;
        createInfo.queueCreateInfoCount = 1;

        VkPhysicalDeviceFeatures deviceFeatures{};

        createInfo.pEnabledFeatures = &deviceFeatures;
        vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

        vkGetDeviceQueue(device, queueinfo.queueFamilyIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(device, queueinfo.queueFamilyIndex, 0, &presentQueue);

        std::cout << "log:\u001b[32m device created\u001b[37m" << std::endl;
    }
    VkSurfaceKHR surface{};
#if defined(__ANDROID__)
    void createSurface(ANativeWindow* window) {
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window = window;
        vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);
    }

#elif defined(_WIN32) || defined(__linux__)
    void createSurface() {
        glfwCreateWindowSurface(instance, window, nullptr, &surface);
    }
#endif
    VkSwapchainKHR swapChain;
    std::vector<VkImageView> swapChainImageViews;
    std::vector<VkImage> swapChainImages;
    std::vector<VkSurfaceFormatKHR> surform{};
    uint32_t choseform;
    void createswapchain() {
        VkSurfaceCapabilitiesKHR capabilities{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

        uint32_t surfcnt = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfcnt, nullptr); // problema e aici

        surform.resize(surfcnt);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfcnt, surform.data());

        for (int i = 0; i != surfcnt; i++) {
            if (surform[i].format == VK_FORMAT_R8G8B8A8_UNORM) {
                choseform = i;
                break;
            }
        }

        uint32_t imageCount = capabilities.minImageCount + 1;
        MAX_FRAMES_IN_FLIGHT = imageCount;
#if defined(__ANDROID__)
        MAX_FRAMES_IN_FLIGHT++;
#endif
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surform[choseform].format;
        createInfo.imageColorSpace = surform[choseform].colorSpace;
        createInfo.imageExtent.height = resolution.y;
        createInfo.imageExtent.width = resolution.x;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        createInfo.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
        createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
        std::cout << "log:\u001b[32m swapchain created\u001b[37m" << std::endl;
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = surform[choseform].format;
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;
            vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]);
        }
        std::cout << "log:\u001b[32m swapchain images created\u001b[37m" << std::endl;
    }
    VkRenderPassCreateInfo renderPassInfo{};
    VkRenderPassCreateInfo renderswPassInfo{};
    VkSubpassDependency dependency{};
    VkFormat depthformat = VK_FORMAT_D32_SFLOAT;
    void createshadowimages() {
        createImage(ShadowMapResolution, ShadowMapResolution, 1, surform[choseform].format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ShadowRenderImage, ShadowRenderImageMemory, 1);
        createImage(ShadowMapResolution, ShadowMapResolution, 1, depthformat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, ShadowImage, ShadowImageMemory, 1);
        createImageView(ShadowRenderImageView, ShadowRenderImage, VK_IMAGE_VIEW_TYPE_2D, 1, 1, surform[choseform].format, VK_IMAGE_ASPECT_COLOR_BIT);
        createImageView(ShadowImageView, ShadowImage, VK_IMAGE_VIEW_TYPE_2D, 1, 1, depthformat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    void createmainimages() {
        createImage((int)resolution.x * resolutionscale, (int)resolution.y * resolutionscale, 1, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MainImage, MainImageMemory, 1);
        createImage((int)resolution.x * resolutionscale, (int)resolution.y * resolutionscale, 1, depthformat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, MaindImage, MaindImageMemory, 1);
        createImageView(MainImageView, MainImage, VK_IMAGE_VIEW_TYPE_2D, 1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, VK_IMAGE_ASPECT_COLOR_BIT);
        createImageView(MaindImageView, MaindImage, VK_IMAGE_VIEW_TYPE_2D, 1, 1, depthformat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }
    void createswrepass() {
        std::vector < VkAttachmentDescription> colorAttachment(1);
        colorAttachment[0].format = surform[choseform].format;
        colorAttachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        std::vector<VkAttachmentReference> colorAttachmentRef(1);
        colorAttachmentRef[0].attachment = 0;
        colorAttachmentRef[0].layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = colorAttachmentRef.data();
        renderswPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderswPassInfo.attachmentCount = 1;
        renderswPassInfo.pAttachments = colorAttachment.data();
        renderswPassInfo.subpassCount = 1;
        renderswPassInfo.pSubpasses = &subpass;
        renderswPassInfo.dependencyCount = 0;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        renderswPassInfo.dependencyCount = 1;
        renderswPassInfo.pDependencies = &dependency;
        vkCreateRenderPass(device, &renderswPassInfo, nullptr, &swapChainrenderPass);
        std::cout << "log:\u001b[32m main renderpass created\u001b[37m" << std::endl;
    }
    void createrepass() {
        createswrepass();
        std::vector < VkAttachmentDescription> colorAttachment(2);
        colorAttachment[0].format = surform[choseform].format;
        colorAttachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachment[1].format = depthformat;
        colorAttachment[1].samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::vector<VkAttachmentReference> colorAttachmentRef(2);
        colorAttachmentRef[0].attachment = 0;
        colorAttachmentRef[0].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        colorAttachmentRef[1].attachment = 1;
        colorAttachmentRef[1].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef[0];
        subpass.pDepthStencilAttachment = &colorAttachmentRef[1];
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 2;
        renderPassInfo.pAttachments = colorAttachment.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 0;
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcAccessMask = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
        colorAttachment[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &mainPass);
        colorAttachment[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        colorAttachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        colorAttachment[0].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        vkCreateRenderPass(device, &renderPassInfo, nullptr, &mainPasss);
        std::cout << "log:\u001b[32m main renderpass created\u001b[37m" << std::endl;
    }
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkFramebuffer ShadowFramebuffer;
    VkFramebuffer MainFramebuffer;
    void createswfrm() {
        swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            VkImageView attachments[] = {
                    swapChainImageViews[i],
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = swapChainrenderPass;
            framebufferInfo.attachmentCount = 1;
            framebufferInfo.pAttachments = attachments;
            framebufferInfo.width = resolution.x;
            framebufferInfo.height = resolution.y;
            framebufferInfo.layers = 1;

            vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);
            std::cout << "log:\u001b[32m swapchain framebuffer created\u001b[37m" << std::endl;
        }
    }
    void createshadowfrm() {
        VkImageView attachments[] = {
                ShadowRenderImageView,
                ShadowImageView
        };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = ShadowMapResolution;
        framebufferInfo.height = ShadowMapResolution;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(device, &framebufferInfo, nullptr, &ShadowFramebuffer);
    }
    void createmainfrm() {
        VkImageView attachments[] = {
                MainImageView,
                MaindImageView
        };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = mainPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = (int)resolution.x * resolutionscale;
        framebufferInfo.height = (int)resolution.y * resolutionscale;
        framebufferInfo.layers = 1;

        vkCreateFramebuffer(device, &framebufferInfo, nullptr, &MainFramebuffer);
    }
    void createcomandpoolbuffer() {
        commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndex;
        VkCommandBufferAllocateInfo allocInfo{};
        vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
        std::cout << "log:\u001b[32m command pool created\u001b[37m" << std::endl;
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
        vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
        std::cout << "log:\u001b[32m command buffer created\u001b[37m" << std::endl;
    }
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    void createsync() {
        imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]);
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]);
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]);
        }
    }
    uint32_t imageIndex;
    void recreateswap() {
        vkDeviceWaitIdle(device);

        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        }
        vkDestroyFramebuffer(device, MainFramebuffer, nullptr);

        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            vkDestroyImageView(device, swapChainImageViews[i], nullptr);
        }
        vkDestroyImageView(device, depthImageView, nullptr);
        vkDestroyImage(device, depthImage, nullptr);
        vkFreeMemory(device, depthImageMemory, nullptr);

        vkDestroyImageView(device, MainImageView, nullptr);
        vkDestroyImage(device, MainImage, nullptr);
        vkFreeMemory(device, MainImageMemory, nullptr);

        vkDestroyImageView(device, MaindImageView, nullptr);
        vkDestroyImage(device, MaindImage, nullptr);
        vkFreeMemory(device, MaindImageMemory, nullptr);

        //vkDestroyPipeline(device, graphicsPipeline, nullptr);
        //vkDestroyPipelineLayout(device, pipelineLayout, nullptr);

        createmainimages();
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        createswapchain();
        createswfrm();
        createmainfrm();
        createDescriptorSetLayout();
        //createPipeline(PostProcessVertexPath, PostProcessFragmentPath, graphicsPipeline, pipelineLayout, &descriptorSetLayout, 1, false, false, VK_CULL_MODE_NONE);
        createUniformBuffers(uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, descriptorPool, descriptorSets, descriptorSetLayout);
    }
    static std::vector<char> loadbin(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {
            throw std::runtime_error("error:\u001b[31m Failed to open file\u001b[37m");
        }
        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
    }
    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("error:\u001b[31m failed to create shader module!\u001b[37m");
        }
        return shaderModule;
    }
    bool alreadyran = false;
    VkDescriptorSetLayout descriptorSetLayout{};
    VkPipelineLayout pipelineLayout{};
    void createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutBinding shadowLayoutBinding{};
        shadowLayoutBinding.binding = 2;
        shadowLayoutBinding.descriptorCount = 1;
        shadowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadowLayoutBinding.pImmutableSamplers = nullptr;
        shadowLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, samplerLayoutBinding, shadowLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
    }
    VkPipeline graphicsPipeline{};
    void createUniformBuffers(std::vector<VkBuffer>& uniformBuffers, std::vector<VkDeviceMemory>& uniformBuffersMemory, std::vector<void*>& uniformBuffersMapped, VkDescriptorPool& descriptorPool, std::vector<VkDescriptorSet>& descriptorSets, VkDescriptorSetLayout& descriptorSetLayout) {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
            vkMapMemory(device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
        }

        std::array<VkDescriptorPoolSize, 3> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

        vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data());

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo colorinfo{};
            colorinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            colorinfo.imageView = MainImageView;
            colorinfo.sampler = RenderSampler;

            VkDescriptorImageInfo depthinfo{};
            depthinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            depthinfo.imageView = MaindImageView;
            depthinfo.sampler = RenderSampler;

            std::array<VkWriteDescriptorSet, 3> descriptorWrite;
            descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[0].dstSet = descriptorSets[i];
            descriptorWrite[0].dstBinding = 0;
            descriptorWrite[0].dstArrayElement = 0;
            descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite[0].descriptorCount = 1;
            descriptorWrite[0].pBufferInfo = &bufferInfo;
            descriptorWrite[0].pNext = nullptr;

            descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[1].dstSet = descriptorSets[i];
            descriptorWrite[1].dstBinding = 1;
            descriptorWrite[1].dstArrayElement = 0;
            descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[1].descriptorCount = 1;
            descriptorWrite[1].pImageInfo = &colorinfo;
            descriptorWrite[1].pNext = nullptr;

            descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[2].dstSet = descriptorSets[i];
            descriptorWrite[2].dstBinding = 2;
            descriptorWrite[2].dstArrayElement = 0;
            descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[2].descriptorCount = 1;
            descriptorWrite[2].pImageInfo = &depthinfo;
            descriptorWrite[2].pNext = nullptr;

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data(), 0, nullptr);
        }
    }
    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};
    std::vector<void*> uniformBuffersMapped{};
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets{};
public:
    std::string ShadowFragmentPath = "data/raw/smf.spv";
    std::string ShadowVertexPath = "data/raw/smv.spv";
    std::string PostProcessFragmentPath = "data/raw/postf.spv";
    std::string PostProcessVertexPath = "data/raw/postv.spv";

    VkImage ShadowImage{};
    VkDeviceMemory ShadowImageMemory{};
    VkImageView ShadowImageView{};
    VkSampler RenderSampler{};

    bool shadowpass = false;
    int MAX_FRAMES_IN_FLIGHT = 2;
    VkQueue graphicsQueue{};
    UniformBufferObject ubo{};

    int ShadowMapResolution = 4000;
    int oldShadowMapResolution = 4000;
    glm::vec3 ShadowPos = glm::vec3(0, 0, 0);
    glm::vec3 ShadowLookAt = glm::vec3(0, 0, 0);
    glm::vec2 ShadowRot = glm::vec2(0, 0);
    bool useShadowLookAt = true;
    bool ShadowOrtho = true;
    float sFov = 10;
    float szNear = 0.1f;
    float szFar = 100.0f;

    bool useOrthographic = false;
    float fov = 90.0f;
    float zNear = 0.1f;
    float zFar = 100.0f;
    glm::vec2 rot = glm::vec2(0, 0);
    glm::vec3 pos = glm::vec3(0, 0, 0);
    std::string pathprefix = "";
    std::string devicename;
    uint32_t currentFrame = 0;
    VkCommandPool commandPool{};
    std::vector<VkCommandBuffer> commandBuffers;
    VkInstance instance{};
    VkQueue presentQueue{};
    VkDevice device{};
    VkRenderPass swapChainrenderPass{};
    VkRenderPass renderPass{};
    VkRenderPass mainPass{};
    VkRenderPass mainPasss{};
#if defined(__ANDROID__)
#elif defined(_WIN32) || defined(__linux__)
    GLFWwindow* window;
#endif
    float resolutionscale = 1.0f;
    float oldresolutionscale = 1.0f;
    glm::ivec2 resolution = glm::ivec2(800, 600);
    glm::ivec2 oldres = glm::ivec2(800, 600);
    bool uselayer = false;
    bool shadowrecreated = false;
    bool fullscreen;
    bool uimat = false;
    bool clear = true;
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("error:\u001b[31m failed to create buffer!\u001b[37m");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("error:\u001b[31m failed to allocate buffer memory!\u001b[37m");
        }

        vkBindBufferMemory(device, buffer, bufferMemory, 0);
    }
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("error:\u001b[31m failed to find suitable memory type!\u001b[37m");
    }
    void createPipeline(std::string vertshader, std::string fragshader, VkPipeline& graphicsPipeline, VkPipelineLayout& pipelineLayout, VkDescriptorSetLayout* descriptorSetLayout, int descriptorcnt, bool shadowusage, bool useresolutionscale, VkCullModeFlagBits cullmode) {
        auto vertShaderCode = loadbin(vertshader);
        auto fragShaderCode = loadbin(fragshader);

        VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";

        VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        auto bindingDescription = vertex::getBindingDescription();
        auto attributeDescriptions = vertex::getAttributeDescriptions();

        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float)resolution.x;
        viewport.height = (float)resolution.y;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent.width = resolution.x;
        scissor.extent.height = resolution.y;

        if (useresolutionscale) {
            viewport.width = (float)resolution.x * resolutionscale;
            viewport.height = (float)resolution.y * resolutionscale;
            scissor.extent.width = (int)resolution.x * resolutionscale;
            scissor.extent.height = (int)resolution.y * resolutionscale;
        }

        if (shadowusage) {
            viewport.width = (float)ShadowMapResolution;
            viewport.height = (float)ShadowMapResolution;
            scissor.extent.width = ShadowMapResolution;
            scissor.extent.height = ShadowMapResolution;
        }

        std::vector<VkDynamicState> dynamicStates = {
                VK_DYNAMIC_STATE_VIEWPORT,
                VK_DYNAMIC_STATE_SCISSOR
        };

        VkPipelineDynamicStateCreateInfo dynamicState{};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();

        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;

        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = cullmode;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;
        rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = 0.0f;

        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampling.minSampleShading = 1.0f;
        multisampling.pSampleMask = nullptr;
        multisampling.alphaToCoverageEnable = VK_FALSE;
        multisampling.alphaToOneEnable = VK_FALSE;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_TRUE;
        colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = descriptorcnt;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayout;
        vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

        VkPipelineDepthStencilStateCreateInfo depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.minDepthBounds = 0.0f;
        depthStencil.maxDepthBounds = 1.0f;
        depthStencil.stencilTestEnable = VK_FALSE;
        depthStencil.front = {};
        depthStencil.back = {};

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        if (useresolutionscale) {
            pipelineInfo.renderPass = mainPass;
        }
        if (!useresolutionscale && !shadowusage) {
            pipelineInfo.renderPass = swapChainrenderPass;
        }
        pipelineInfo.subpass = 0;
        vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
        std::cout << "log:\u001b[32m pipeline created\u001b[37m" << std::endl;
    }
    void createvertexbuf(vertex* vertices, int size, VkBuffer& vertexBuffer, VkDeviceMemory& vertexBufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = sizeof(vertices[0]) * size;
        bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer);

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory);
        vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);
    }
    void createDescriptorSetLayout(VkDescriptorSetLayout& descriptorSetLayout, unsigned int binding, VkDescriptorType descriptorType) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = binding;
        uboLayoutBinding.descriptorType = descriptorType;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = 1;
        layoutInfo.pBindings = &uboLayoutBinding;
        vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout);
    }
    void createImage(uint32_t width, uint32_t height, int miplevel, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, int imagearray) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = miplevel;
        imageInfo.arrayLayers = imagearray;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("error:\u001b[31m failed to create image!\u001b[37m");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("error:\u001b[31m failed to allocate image memory!\u001b[37m");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }
    void createImageView(VkImageView& imageview, VkImage& textureImage, VkImageViewType imageviewtype, int imagearray, int miplevels, VkFormat format, VkImageAspectFlags aspect) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = textureImage;
        viewInfo.viewType = imageviewtype;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspect;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = miplevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = imagearray;
        vkCreateImageView(device, &viewInfo, nullptr, &imageview);
    }
    void createImageSampler(VkFilter magfilter, VkFilter minfilter, VkSamplerAddressMode addresmode, VkSampler& textureSampler, int mipLevels) {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = magfilter;
        samplerInfo.minFilter = minfilter;
        samplerInfo.addressModeU = addresmode;
        samplerInfo.addressModeV = addresmode;
        samplerInfo.addressModeW = addresmode;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.anisotropyEnable = VK_FALSE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);
        vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler);
    }
    void recreateShadowResources() {
        vkDeviceWaitIdle(device);
        vkDestroyFramebuffer(device, ShadowFramebuffer, nullptr);

        vkDestroyImageView(device, ShadowRenderImageView, nullptr);
        vkDestroyImageView(device, ShadowImageView, nullptr);
        vkDestroyImage(device, ShadowRenderImage, nullptr);
        vkDestroyImage(device, ShadowImage, nullptr);
        vkFreeMemory(device, ShadowImageMemory, nullptr);
        vkFreeMemory(device, ShadowRenderImageMemory, nullptr);

        createshadowimages();
        createshadowfrm();
        shadowrecreated = true;
    }
#ifdef __ANDROID__
    void init(std::string appname, ANativeWindow* window) {
        resolution.x = ANativeWindow_getWidth(window);
        resolution.y = ANativeWindow_getHeight(window);
        createInstance(appname);
        createSurface(window);
        std::ifstream cfgwork{};
        cfgwork.open(pathprefix + "eng/cfg/Render.cfg");

        if (!cfw) {
            std::string param;
            float argument;
            while (cfgwork >> param >> argument) {
                if (param == "shadowres") {
                    ShadowMapResolution = (int)argument;
                    oldShadowMapResolution = (int)argument;
                }
                if (param == "renderscale") {
                    resolutionscale = argument;
                    oldresolutionscale = argument;
                }
            }
        }
#else
    void init(std::string appname) {
        std::ifstream cfgwork{};
        cfgwork.open(pathprefix + "eng/cfg/Render.cfg");

        if (!cfw) {
            std::string param;
            float argument;
            while (cfgwork >> param >> argument) {
                if (param == "wsizex") {
                    resolution.x = (int)argument;
                }
                if (param == "wsizey") {
                    resolution.y = (int)argument;
                }
                if (param == "wfull" && (int)argument == 1) {
                    fullscreen = (int)argument;
                }
                if (param == "shadowres") {
                    ShadowMapResolution = (int)argument;
                    oldShadowMapResolution = (int)argument;
                }
                if (param == "renderscale") {
                    resolutionscale = argument;
                    oldresolutionscale = argument;
                }
            }
        }

        std::string platformname = "UNKNOWN";

#if defined(_WIN32)
        platformname = "WIN32";
#elif defined(__linux__)
        platformname = "Linux";
#endif

        std::cout << "log:\u001b[36m platform = " << platformname << "\u001b[37m" << std::endl;
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        glfwWindowHint(GLFW_RED_BITS, mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
        if (fullscreen == true) {
            window = glfwCreateWindow(resolution.x, resolution.y, (appname + " - " + platformname).c_str(), glfwGetPrimaryMonitor(), nullptr);
        }
        else {
            window = glfwCreateWindow(resolution.x, resolution.y, (appname + " - " + platformname).c_str(), nullptr, nullptr);
        }
        std::cout << "log:\u001b[36m window created\u001b[37m" << std::endl;
        createInstance(appname);
        createSurface();
#endif
        getDevice();
        createswapchain();
        createshadowimages();
        createmainimages();
        createrepass();
        createswfrm();
        createshadowfrm();
        createmainfrm();
        createcomandpoolbuffer();
        createsync();
        createImageSampler(VK_FILTER_NEAREST, VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, RenderSampler, 1);
        for (int i = 0; i != 10; i++) {
            ubo.lightColor[i] = glm::vec4(0, 0, 0, 0);
            ubo.lightPos[i] = glm::vec4(0, 0, 0, 0);
        }
        ShadowVertexPath = pathprefix + ShadowVertexPath;
        ShadowFragmentPath = pathprefix + ShadowFragmentPath;
        PostProcessVertexPath = pathprefix + PostProcessVertexPath;
        PostProcessFragmentPath = pathprefix + PostProcessFragmentPath;
        createDescriptorSetLayout();
        createPipeline(PostProcessVertexPath, PostProcessFragmentPath, graphicsPipeline, pipelineLayout, &descriptorSetLayout, 1, false, false, VK_CULL_MODE_NONE);
        createUniformBuffers(uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, descriptorPool, descriptorSets, descriptorSetLayout);
        std::cout << "log:\u001b[32m Render initied with success\u001b[37m" << std::endl;
    }
    bool shouldterminate() {
#if defined(__ANDROID__)
        return true;
#elif defined(_WIN32) || defined(__linux__)
        return !glfwWindowShouldClose(window);
#endif
    }
#ifdef __ANDROID__
    void beginRender(ANativeWindow * window) {
        resolution.x = ANativeWindow_getWidth(window);
        resolution.y = ANativeWindow_getHeight(window);
#else
    void beginRender() {
        glfwGetFramebufferSize(window, &resolution.x, &resolution.y);
#endif
        if (ShadowMapResolution != oldShadowMapResolution) {
            recreateShadowResources();
        }

        vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        vkResetFences(device, 1, &inFlightFences[currentFrame]);

        VkResult res = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateswap();
        }

        vkResetCommandBuffer(commandBuffers[currentFrame], 0);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;
        vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo);
    }
    void beginMainPass() {
        alreadyran = true;
        if (shadowpass) {
            vkCmdEndRenderPass(commandBuffers[currentFrame]);
            shadowpass = false;
        }
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = mainPass;
        if (!clear) {
            renderPassInfo.renderPass = mainPasss;
        }
        renderPassInfo.framebuffer = MainFramebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent.width = (int)(resolution.x * resolutionscale);
        renderPassInfo.renderArea.extent.height = (int)(resolution.y * resolutionscale);
        std::vector<VkClearValue> clearColor(2);
        clearColor[0] = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        clearColor[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearColor.data();
        clear = false;

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    void beginShadowPass() {
        shadowpass = true;
        if (alreadyran) {
            vkCmdEndRenderPass(commandBuffers[currentFrame]);
        }
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = ShadowFramebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent.width = ShadowMapResolution;
        renderPassInfo.renderArea.extent.height = ShadowMapResolution;
        std::vector<VkClearValue> clearColor(2);
        clearColor[0] = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        clearColor[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = 2;
        renderPassInfo.pClearValues = clearColor.data();

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }
    bool renderswitch = false;
    void endRender() {
        vkCmdEndRenderPass(commandBuffers[currentFrame]);
#if !defined(__ANDROID__)
        glfwGetFramebufferSize(window, &resolution.x, &resolution.y);
#endif
        VkRenderPassBeginInfo passbeg{};
        passbeg.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        passbeg.renderPass = swapChainrenderPass;
        passbeg.framebuffer = swapChainFramebuffers[currentFrame];
        passbeg.renderArea.offset = { 0, 0 };
        passbeg.renderArea.extent.width = resolution.x;
        passbeg.renderArea.extent.height = resolution.y - 1;
        std::vector<VkClearValue> clearColor(1);
        clearColor[0] = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
        passbeg.clearValueCount = 1;
        passbeg.pClearValues = clearColor.data();
        memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));

        vkCmdBeginRenderPass(commandBuffers[currentFrame], &passbeg, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

        vkCmdBindDescriptorSets(commandBuffers[currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(resolution.x);
        viewport.height = static_cast<float>(resolution.y);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent.width = resolution.x;
        scissor.extent.height = resolution.y;
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);

        vkCmdDraw(commandBuffers[currentFrame], 6, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[currentFrame]);

        vkEndCommandBuffer(commandBuffers[currentFrame]);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &swapChain;
        presentInfo.pImageIndices = &imageIndex;

        VkResult res = vkQueuePresentKHR(presentQueue, &presentInfo);

        if (resolution.x != oldres.x || resolution.y != oldres.y || resolutionscale != oldresolutionscale || ShadowMapResolution != oldShadowMapResolution || res == VK_ERROR_OUT_OF_DATE_KHR) {
            std::ofstream cfgwork{};
            cfgwork.open(pathprefix + "eng/cfg/Render.cfg", std::ofstream::out | std::ofstream::trunc);
            cfgwork << "vkver " << writeapiver << std::endl;
            cfgwork << "vkphysdev " << choseddevice << std::endl;
            cfgwork << "wsizex " << resolution.x << std::endl;
            cfgwork << "wsizey " << resolution.y << std::endl;
            cfgwork << "wfull " << fullscreen << std::endl;
            cfgwork << "shadowres " << ShadowMapResolution << std::endl;
            cfgwork << "renderscale " << resolutionscale << std::endl;
            cfgwork.close();
            resolution.y -= 1;
            recreateswap();
            resolution.y += 1;
        }

        currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

        oldres.x = resolution.x;
        oldres.y = resolution.y;
        oldresolutionscale = resolutionscale;
        oldShadowMapResolution = ShadowMapResolution;
        alreadyran = false;
        shadowrecreated = false;
        clear = true;
        renderswitch = !renderswitch;

#if defined(__ANDROID__)
#elif defined(_WIN32) || defined(__linux__)
        glfwPollEvents();
#endif
    }
    void terminate() {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyDevice(device, nullptr);
        vkDestroyInstance(instance, nullptr);
#if defined(__ANDROID__)
#elif defined(_WIN32) || defined(__linux__)
        glfwDestroyWindow(window);
        glfwTerminate();
#endif
    }
    };

class Mesh {
private:
    std::string vs{};
    std::string fs{};
    VkPipeline graphicsPipeline{};
    VkPipeline graphicsPipelineShadow{};
    VkBuffer vertexBuffer{};
    VkDeviceMemory vertexBufferMemory{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkPipelineLayout pipelineLayout{};
    std::vector<VkBuffer> uniformBuffers{};
    std::vector<VkDeviceMemory> uniformBuffersMemory{};
    std::vector<void*> uniformBuffersMapped{};
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets{};
    VkBuffer stagingBuffer{};
    VkDeviceMemory stagingBufferMemory{};
    uint32_t mipLevels;
    VkImage textureImage{};
    VkDeviceMemory textureImageMemory{};
    VkImageView textureImageView{};
    VkSampler textureSampler;
    VkImage cubeImage{};
    VkDeviceMemory cubeImageMemory{};
    VkImageView cubeImageView{};
    VkSampler cubeSampler;

    VkPipelineStageFlags sourceStage{};
    VkPipelineStageFlags destinationStage{};

    void* vdata;

    VkCommandBuffer beginSingleTimeCommands(Render& eng) {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = eng.commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(eng.device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, Render& eng) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(eng.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(eng.graphicsQueue);

        vkFreeCommandBuffers(eng.device, eng.commandPool, 1, &commandBuffer);
    }
    void copyBuffer(VkBuffer srcBuffer, VkBuffer& dstBuffer, VkDeviceSize size, Render& eng) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(eng);

        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer, eng);
    }
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, Render& eng, int levelcount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(eng);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = mipLevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = levelcount;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        endSingleTimeCommands(commandBuffer, eng);
    }
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, Render& eng, int layercount) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(eng);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layercount;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
                width,
                height,
                1
        };
        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        endSingleTimeCommands(commandBuffer, eng);
    }
    void createDescriptorSetLayout(Render& eng) {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_ALL;

        VkDescriptorSetLayoutBinding shadowLayoutBinding{};
        shadowLayoutBinding.binding = 2;
        shadowLayoutBinding.descriptorCount = 1;
        shadowLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        shadowLayoutBinding.pImmutableSamplers = nullptr;
        shadowLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutBinding cubeLayoutBinding{};
        cubeLayoutBinding.binding = 3;
        cubeLayoutBinding.descriptorCount = 1;
        cubeLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        cubeLayoutBinding.pImmutableSamplers = nullptr;
        cubeLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        std::array<VkDescriptorSetLayoutBinding, 4> bindings = { uboLayoutBinding, samplerLayoutBinding, shadowLayoutBinding, cubeLayoutBinding };
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        vkCreateDescriptorSetLayout(eng.device, &layoutInfo, nullptr, &descriptorSetLayout);
    }
    int maxrep = 1;
    int rendert = 1;
    int createdbuff = 0;
    void createUniformBuffers(std::vector<VkBuffer>& uniformBuffers, std::vector<VkDeviceMemory>& uniformBuffersMemory, std::vector<void*>& uniformBuffersMapped, VkDescriptorPool& descriptorPool, std::vector<VkDescriptorSet>& descriptorSets, VkDescriptorSetLayout& descriptorSetLayout, VkSampler& textureSampler, VkImageView& textureImageView, Render& eng) {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        if (maxrep > createdbuff) {
            uniformBuffers.resize(eng.MAX_FRAMES_IN_FLIGHT * maxrep);
            uniformBuffersMemory.resize(eng.MAX_FRAMES_IN_FLIGHT * maxrep);
            uniformBuffersMapped.resize(eng.MAX_FRAMES_IN_FLIGHT * maxrep);

            for (size_t i = createdbuff * eng.MAX_FRAMES_IN_FLIGHT; i < eng.MAX_FRAMES_IN_FLIGHT * maxrep; i++) {
                eng.createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
                vkMapMemory(eng.device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
            }
            createdbuff = maxrep;
        }

        std::array<VkDescriptorPoolSize, 4> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = static_cast<uint32_t>(eng.MAX_FRAMES_IN_FLIGHT * maxrep);
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = static_cast<uint32_t>(eng.MAX_FRAMES_IN_FLIGHT * maxrep);
        poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[2].descriptorCount = static_cast<uint32_t>(eng.MAX_FRAMES_IN_FLIGHT * maxrep);
        poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[3].descriptorCount = static_cast<uint32_t>(eng.MAX_FRAMES_IN_FLIGHT * maxrep);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(eng.MAX_FRAMES_IN_FLIGHT * maxrep);

        vkCreateDescriptorPool(eng.device, &poolInfo, nullptr, &descriptorPool);

        std::vector<VkDescriptorSetLayout> layouts(eng.MAX_FRAMES_IN_FLIGHT * maxrep, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;

        allocInfo.descriptorSetCount = eng.MAX_FRAMES_IN_FLIGHT * maxrep;
        allocInfo.pSetLayouts = layouts.data();
        descriptorSets.resize(eng.MAX_FRAMES_IN_FLIGHT * maxrep);
        vkAllocateDescriptorSets(eng.device, &allocInfo, descriptorSets.data());

        for (size_t i = 0; i < eng.MAX_FRAMES_IN_FLIGHT * maxrep; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = textureImageView;
            imageInfo.sampler = textureSampler;

            VkDescriptorImageInfo shadowInfo{};
            shadowInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            shadowInfo.imageView = eng.ShadowImageView;
            shadowInfo.sampler = eng.RenderSampler;

            VkDescriptorImageInfo cubeInfo{};
            cubeInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            cubeInfo.imageView = cubeImageView;
            cubeInfo.sampler = cubeSampler;

            std::array<VkWriteDescriptorSet, 4> descriptorWrite;
            descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[0].dstSet = descriptorSets[i];
            descriptorWrite[0].dstBinding = 0;
            descriptorWrite[0].dstArrayElement = 0;
            descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite[0].descriptorCount = 1;
            descriptorWrite[0].pBufferInfo = &bufferInfo;
            descriptorWrite[0].pNext = nullptr;

            descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[1].dstSet = descriptorSets[i];
            descriptorWrite[1].dstBinding = 1;
            descriptorWrite[1].dstArrayElement = 0;
            descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[1].descriptorCount = 1;
            descriptorWrite[1].pImageInfo = &imageInfo;
            descriptorWrite[1].pNext = nullptr;

            descriptorWrite[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[2].dstSet = descriptorSets[i];
            descriptorWrite[2].dstBinding = 2;
            descriptorWrite[2].dstArrayElement = 0;
            descriptorWrite[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[2].descriptorCount = 1;
            descriptorWrite[2].pImageInfo = &shadowInfo;
            descriptorWrite[2].pNext = nullptr;

            descriptorWrite[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[3].dstSet = descriptorSets[i];
            descriptorWrite[3].dstBinding = 3;
            descriptorWrite[3].dstArrayElement = 0;
            descriptorWrite[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[3].descriptorCount = 1;
            descriptorWrite[3].pImageInfo = &cubeInfo;
            descriptorWrite[3].pNext = nullptr;

            vkUpdateDescriptorSets(eng.device, static_cast<uint32_t>(descriptorWrite.size()), descriptorWrite.data(), 0, nullptr);
        }
    }
    void generateMipmaps(VkImage& image, int32_t texWidth, int32_t texHeight, uint32_t imagecount, Render& eng) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(eng);

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.image = image;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = imagecount;
        barrier.subresourceRange.levelCount = 1;

        int32_t mipWidth = texWidth;
        int32_t mipHeight = texHeight;

        for (uint32_t i = 1; i < mipLevels; i++) {
            barrier.subresourceRange.baseMipLevel = i - 1;
            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            VkImageBlit blit{};
            blit.srcOffsets[0] = { 0, 0, 0 };
            blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
            blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.srcSubresource.mipLevel = i - 1;
            blit.srcSubresource.baseArrayLayer = 0;
            blit.srcSubresource.layerCount = imagecount;
            blit.dstOffsets[0] = { 0, 0, 0 };
            blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
            blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            blit.dstSubresource.mipLevel = i;
            blit.dstSubresource.baseArrayLayer = 0;
            blit.dstSubresource.layerCount = imagecount;

            vkCmdBlitImage(commandBuffer,
                image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &blit,
                VK_FILTER_LINEAR);

            barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
            barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);

            if (mipWidth > 1) mipWidth /= 2;
            if (mipHeight > 1) mipHeight /= 2;
        }

        barrier.subresourceRange.baseMipLevel = mipLevels - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier);

        endSingleTimeCommands(commandBuffer, eng);
    }
    void matoper(Render& eng) {
        if (enablePlayerMatrix) {
            if (eng.useOrthographic) {
                eng.ubo.projection = glm::ortho(-eng.fov, eng.fov, -eng.fov, eng.fov, eng.zNear, eng.zFar);
            }
            else {
                eng.ubo.projection = glm::perspective(eng.fov, (float)eng.resolution.x / eng.resolution.y, eng.zNear, eng.zFar);
            }
            eng.ubo.translate = glm::translate(glm::mat4(1.0f), glm::vec3(eng.pos.x, eng.pos.y, eng.pos.z));
            eng.ubo.rotx = glm::rotate(glm::mat4(1.0f), eng.rot.x, glm::vec3(1, 0, 0));
            eng.ubo.roty = glm::rotate(glm::mat4(1.0f), eng.rot.y, glm::vec3(0, 1, 0));
            if (eng.uimat) {
                eng.ubo.projection = glm::ortho(0.0f, (float)eng.resolution.x, 0.0f, (float)eng.resolution.y, 0.1f, 1000.0f);
            }
        }
        if (enableMeshMatrix) {
            eng.ubo.mtranslate = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, pos.z));
            eng.ubo.mrotx = glm::rotate(glm::mat4(1.0f), rot.x, glm::vec3(1, 0, 0));
            eng.ubo.mroty = glm::rotate(glm::mat4(1.0f), rot.y, glm::vec3(0, 1, 0));
            eng.ubo.mrotz = glm::rotate(glm::mat4(1.0f), rot.z, glm::vec3(0, 0, 1));
            eng.ubo.mscale = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z));
        }
        if (enableShadowMatrix) {
            if (eng.ShadowOrtho) {
                eng.ubo.sprojection = glm::ortho(-eng.sFov, eng.sFov, -eng.sFov, eng.sFov, eng.szNear, eng.szFar);
            }
            else {
                eng.ubo.sprojection = glm::perspective(eng.sFov, 1.0f, eng.szNear, eng.szFar);
            }
            if (eng.useShadowLookAt) {
                eng.ubo.stranslate = glm::lookAt(eng.ShadowPos, eng.ShadowLookAt, glm::vec3(0.0, 1.0, 0.0));
                eng.ubo.useLookAt.x = 1;
            }
            else {
                eng.ubo.useLookAt.x = 0;
                eng.ubo.stranslate = glm::translate(glm::mat4(1.0f), glm::vec3(eng.ShadowPos.x, eng.ShadowPos.y, eng.ShadowPos.z));
                eng.ubo.srotx = glm::rotate(glm::mat4(1.0f), eng.ShadowRot.x, glm::vec3(1, 0, 0));
                eng.ubo.sroty = glm::rotate(glm::mat4(1.0f), eng.ShadowRot.y, glm::vec3(0, 1, 0));
            }
        }
    }
    bool lmfr = true;
public:
    VkCullModeFlagBits cullmode = VK_CULL_MODE_BACK_BIT;
    VkCullModeFlagBits shadowcullmode = VK_CULL_MODE_FRONT_BIT;
    int totalvertex = 3;
    std::vector <vertex> vertexdata{};
    glm::vec3 pos = glm::vec3(0, 0, 0);
    glm::vec3 rot = glm::vec3(0, 0, 0);
    glm::vec3 scale = glm::vec3(1, 1, 1);
    bool enableMeshMatrix = true;
    bool enablePlayerMatrix = true;
    bool enableShadowMatrix = true;
    void create(Render& eng, std::string vertshader, std::string fragshader, glm::vec3* vertexes, glm::vec2* uv, glm::vec3* normals, int size, unsigned char* pixels, glm::ivec2 TexResolution, int imagecount, unsigned char* cpixels, glm::ivec2 cubeResolution, int cubecount) {
        vs = vertshader;
        fs = fragshader;
        vertexdata.resize(size);
        totalvertex = size;

        for (int i = 0; i != size; i++) {
            vertexdata[i].position.x = vertexes[i].x;
            vertexdata[i].position.y = vertexes[i].y;
            vertexdata[i].position.z = vertexes[i].z;

            vertexdata[i].uv.x = uv[i].x;
            vertexdata[i].uv.y = uv[i].y;

            vertexdata[i].normal.x = normals[i].x;
            vertexdata[i].normal.y = normals[i].y;
            vertexdata[i].normal.z = normals[i].z;
        }
        for (int i = 0; i != totalvertex; i += 3) {
            glm::vec3 v0 = vertexdata[i].position;
            glm::vec3 v1 = vertexdata[i + 1].position;
            glm::vec3 v2 = vertexdata[i + 2].position;

            glm::vec2 uv0 = glm::vec2(vertexdata[i].uv.x, -vertexdata[i].uv.y);
            glm::vec2 uv1 = glm::vec2(vertexdata[i + 1].uv.x, -vertexdata[i + 1].uv.y);
            glm::vec2 uv2 = glm::vec2(vertexdata[i + 2].uv.x, -vertexdata[i + 2].uv.y);

            glm::vec3 deltapos1 = glm::vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
            glm::vec3 deltapos2 = glm::vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);

            glm::vec2 deltaUV1 = glm::vec2(uv1.x - uv0.x, uv1.y - uv0.y);
            glm::vec2 deltaUV2 = glm::vec2(uv2.x - uv0.x, uv2.y - uv0.y);

            float r = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

            vertexdata[i].tangent.x = (deltapos1.x * deltaUV2.y - deltapos2.x * deltaUV1.y) * r;
            vertexdata[i].tangent.y = (deltapos1.y * deltaUV2.y - deltapos2.y * deltaUV1.y) * r;
            vertexdata[i].tangent.z = (deltapos1.z * deltaUV2.y - deltapos2.z * deltaUV1.y) * r;

            vertexdata[i + 1].tangent.y = (deltapos1.y * deltaUV2.y - deltapos2.y * deltaUV1.y) * r;
            vertexdata[i + 1].tangent.z = (deltapos1.z * deltaUV2.y - deltapos2.z * deltaUV1.y) * r;
            vertexdata[i + 1].tangent.x = (deltapos1.x * deltaUV2.y - deltapos2.x * deltaUV1.y) * r;

            vertexdata[i + 2].tangent.x = (deltapos1.x * deltaUV2.y - deltapos2.x * deltaUV1.y) * r;
            vertexdata[i + 2].tangent.y = (deltapos1.y * deltaUV2.y - deltapos2.y * deltaUV1.y) * r;
            vertexdata[i + 2].tangent.z = (deltapos1.z * deltaUV2.y - deltapos2.z * deltaUV1.y) * r;
        }

        VkDeviceSize imageSize = TexResolution.x * TexResolution.y * 4 * imagecount;
        std::cout << "log:\u001b[36m imagesize = " << imageSize << "\u001b[37m" << std::endl;
        eng.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        void* data;
        vkMapMemory(eng.device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(eng.device, stagingBufferMemory);
        mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(TexResolution.x, TexResolution.y)))) + 1;
        std::cout << "log:\u001b[36m " << mipLevels << " miplevels\u001b[37m" << std::endl;
        eng.createImage(TexResolution.x, TexResolution.y, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory, imagecount);
        transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, eng, imagecount);
        copyBufferToImage(stagingBuffer, textureImage, static_cast<uint32_t>(TexResolution.x), static_cast<uint32_t>(TexResolution.y), eng, imagecount);
        generateMipmaps(textureImage, TexResolution.x, TexResolution.y, imagecount, eng);
        eng.createImageView(textureImageView, textureImage, VK_IMAGE_VIEW_TYPE_2D_ARRAY, imagecount, mipLevels, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        eng.createImageSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, textureSampler, mipLevels);

        vkDestroyBuffer(eng.device, stagingBuffer, nullptr);
        vkFreeMemory(eng.device, stagingBufferMemory, nullptr);

        imageSize = cubeResolution.x * cubeResolution.y * 4 * cubecount * 6;
        std::cout << "log:\u001b[36m cube size = " << imageSize << "\u001b[37m" << std::endl;
        eng.createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        vkMapMemory(eng.device, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, cpixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(eng.device, stagingBufferMemory);
        eng.createImage(cubeResolution.x, cubeResolution.y, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cubeImage, cubeImageMemory, cubecount * 6);
        transitionImageLayout(cubeImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, eng, cubecount * 6);
        copyBufferToImage(stagingBuffer, cubeImage, static_cast<uint32_t>(cubeResolution.x), static_cast<uint32_t>(cubeResolution.y), eng, cubecount * 6);
        eng.createImageView(cubeImageView, cubeImage, VK_IMAGE_VIEW_TYPE_CUBE_ARRAY, cubecount * 6, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
        eng.createImageSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT, cubeSampler, 1);

        createDescriptorSetLayout(eng);
        eng.createPipeline(vertshader, fragshader, graphicsPipeline, pipelineLayout, &descriptorSetLayout, 1, false, true, cullmode);
        eng.createPipeline(eng.ShadowVertexPath, eng.ShadowFragmentPath, graphicsPipelineShadow, pipelineLayout, &descriptorSetLayout, 1, true, false, shadowcullmode);
        eng.createvertexbuf(vertexdata.data(), size, vertexBuffer, vertexBufferMemory);
        createUniformBuffers(uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, descriptorPool, descriptorSets, descriptorSetLayout, textureSampler, textureImageView, eng);

        vkMapMemory(eng.device, vertexBufferMemory, 0, sizeof(vertexdata[0]) * size, 0, &vdata);
        memcpy(vdata, vertexdata.data(), sizeof(vertexdata[0]) * size);
    }
    void applyChanges(Render& eng) {
        eng.createPipeline(vs, fs, graphicsPipeline, pipelineLayout, &descriptorSetLayout, 1, false, true, cullmode);
        eng.createPipeline(eng.ShadowVertexPath, eng.ShadowFragmentPath, graphicsPipelineShadow, pipelineLayout, &descriptorSetLayout, 1, true, false, shadowcullmode);
    }
    void Draw(Render& eng) {
        if (lmfr == eng.renderswitch) {
            rendert++;
            if (maxrep < rendert) {
                maxrep = rendert * rendert;
                vkFreeDescriptorSets(eng.device, descriptorPool, eng.MAX_FRAMES_IN_FLIGHT, descriptorSets.data());
                vkDestroyDescriptorPool(eng.device, descriptorPool, nullptr);
                createUniformBuffers(uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, descriptorPool, descriptorSets, descriptorSetLayout, textureSampler, textureImageView, eng);
            }
        }
        else {
            rendert = 1;
        }
        if (eng.shadowrecreated == true) {
            createUniformBuffers(uniformBuffers, uniformBuffersMemory, uniformBuffersMapped, descriptorPool, descriptorSets, descriptorSetLayout, textureSampler, textureImageView, eng);
        }
        matoper(eng);

        eng.ubo.cameraPosition.x = eng.pos.x;
        eng.ubo.cameraPosition.y = eng.pos.y;
        eng.ubo.cameraPosition.z = eng.pos.z;
        eng.ubo.cameraPosition.w = 0;
        eng.ubo.resolution.x = (int)eng.resolution.x * eng.resolutionscale;
        eng.ubo.resolution.y = (int)eng.resolution.y * eng.resolutionscale;

        memcpy(uniformBuffersMapped[eng.currentFrame + eng.MAX_FRAMES_IN_FLIGHT * (rendert - 1)], &eng.ubo, sizeof(eng.ubo));
        memcpy(vdata, vertexdata.data(), sizeof(vertexdata[0]) * totalvertex);

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };

        if (eng.shadowpass) {
            vkCmdBindPipeline(eng.commandBuffers[eng.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipelineShadow);
        }
        else {
            vkCmdBindPipeline(eng.commandBuffers[eng.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
        }

        vkCmdBindVertexBuffers(eng.commandBuffers[eng.currentFrame], 0, 1, vertexBuffers, offsets);

        vkCmdBindDescriptorSets(eng.commandBuffers[eng.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[eng.currentFrame + eng.MAX_FRAMES_IN_FLIGHT * (rendert - 1)], 0, nullptr);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>((int)eng.resolution.x * eng.resolutionscale);
        viewport.height = static_cast<float>((int)eng.resolution.y * eng.resolutionscale);
        if (eng.shadowpass) {
            viewport.width = static_cast<float>(eng.ShadowMapResolution);
            viewport.height = static_cast<float>(eng.ShadowMapResolution);
        }
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(eng.commandBuffers[eng.currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent.width = (int)eng.resolution.x * eng.resolutionscale;
        scissor.extent.height = (int)eng.resolution.y * eng.resolutionscale;
        if (eng.shadowpass) {
            scissor.extent.width = eng.ShadowMapResolution;
            scissor.extent.height = eng.ShadowMapResolution;
        }
        vkCmdSetScissor(eng.commandBuffers[eng.currentFrame], 0, 1, &scissor);

        vkCmdDraw(eng.commandBuffers[eng.currentFrame], totalvertex, 1, 0, 0);
        lmfr = eng.renderswitch;
    }
};
