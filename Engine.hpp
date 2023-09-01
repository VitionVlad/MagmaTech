#include <iostream>
#include <fstream>
#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include <string>
#include <vector>

#ifdef _WIN32
#include "GLFW/glfw3.h"
#elif __ANDROID__
#include <android_native_app_glue.h>
#include <android/native_activity.h>
#include "android/native_window.h"
#include "android/native_window_jni.h"
#include "vulkan/vulkan_android.h"
#endif

class Engine {
private:
	const int MAX_FRAMES_IN_FLIGHT = 2;
	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}
	void createInstance(std::string appname) {
		std::cout << "log: instance creating began" << std::endl;
		VkApplicationInfo appinfo{};
		appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appinfo.pApplicationName = appname.c_str();
		appinfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appinfo.pEngineName = "ShadowVK";
		appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appinfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appinfo;

		#ifdef _WIN32
				uint32_t glfwExtensionCount = 0;
				const char** glfwExtensions;

				glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

				createInfo.enabledExtensionCount = glfwExtensionCount;
				createInfo.ppEnabledExtensionNames = glfwExtensions;
        #elif __ANDROID__
            uint32_t extensions_count = 0;
            vkEnumerateInstanceExtensionProperties( nullptr, &extensions_count, nullptr );
            std::vector<VkExtensionProperties> extensionProperties(extensions_count);
            vkEnumerateInstanceExtensionProperties(nullptr, &extensions_count, extensionProperties.data());
            std::vector<char*> names(extensions_count);
            for(int i = 0; i != extensions_count; i++){
                names[i] = extensionProperties[i].extensionName;
            }
            createInfo.enabledExtensionCount = extensions_count;
            createInfo.ppEnabledExtensionNames = names.data();
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
			std::cout << "log: Enabling Instance Layer:" << lprop[i].layerName << std::endl;
		}

		createInfo.ppEnabledLayerNames = layerNames.data();
		createInfo.enabledLayerCount = layercont;

		if (!uselayer) {
			createInfo.enabledLayerCount = 0;
		}

		vkCreateInstance(&createInfo, nullptr, &instance);
		std::cout << "log: instance created" << std::endl;
	}
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDeviceQueueCreateInfo queueinfo{};
	uint32_t queueFamilyIndex;
	void getDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("error: failed to find GPUs with Vulkan support!");
		}
		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
		physicalDevice = devices[deviceCount - 1];
		VkPhysicalDeviceProperties physprop{};
		vkGetPhysicalDeviceProperties(physicalDevice, &physprop);
		std::cout << "log: device name = " << physprop.deviceName << std::endl;
		std::cout << "log: device type = " << physprop.deviceType << std::endl;

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		queueFamilyIndex = queueFamilies[queueFamilyCount - 1].queueCount;

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

		uint32_t extensioncount = 0;
		std::vector<VkExtensionProperties> extensionproprites{};
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensioncount, nullptr);
		extensionproprites.resize(extensioncount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensioncount, extensionproprites.data());
		std::vector<char*> extensionNames{};
		extensionNames.resize(extensioncount);
		for (int i = 0; i != extensioncount; i++) {
			extensionNames[i] = extensionproprites[i].extensionName;
			std::cout << "log:Enabling Device Extension:" << extensionproprites[i].extensionName << std::endl;
		}

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueinfo;
		createInfo.queueCreateInfoCount = 1;

		VkPhysicalDeviceFeatures deviceFeatures{};

		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = extensioncount;
		createInfo.ppEnabledExtensionNames = extensionNames.data();
		vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);

		vkGetDeviceQueue(device, queueinfo.queueFamilyIndex, 0, &graphicsQueue);
		vkGetDeviceQueue(device, queueinfo.queueFamilyIndex, 0, &presentQueue);

		std::cout << "log: device created" << std::endl;
	}
	VkSurfaceKHR surface{};
	void createSurface() {
		#ifdef _WIN32
		glfwCreateWindowSurface(instance, window, nullptr, &surface);
        #elif __ANDROID__
        VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
        surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
        surfaceCreateInfo.window = window;
        vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, &surface);
		#endif
	}
	VkSwapchainKHR swapChain;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkImage> swapChainImages;
	void createswapchain() {
		VkSurfaceCapabilitiesKHR capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);
		uint32_t imageCount = capabilities.minImageCount + 1;
		VkSwapchainCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;
		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
		createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		createInfo.imageExtent.height = resolution.y;
		createInfo.imageExtent.width = resolution.x;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		createInfo.minImageCount = capabilities.minImageCount;
		createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
		createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.oldSwapchain = VK_NULL_HANDLE;
		vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
		std::cout << "log: swapchain created" << std::endl;
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
		swapChainImages.resize(imageCount);
		vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

		swapChainImageViews.resize(swapChainImages.size());
		for (size_t i = 0; i < swapChainImages.size(); i++) {
			VkImageViewCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			createInfo.image = swapChainImages[i];
			createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			createInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
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
		std::cout << "log: swapchain images created" << std::endl;
	}
	VkRenderPassCreateInfo renderPassInfo{};
	VkSubpassDependency dependency{};
	void createrepass() {
		std::vector < VkAttachmentDescription> colorAttachment(2);
		colorAttachment[0].format = VK_FORMAT_B8G8R8A8_SRGB;
		colorAttachment[0].samples = VK_SAMPLE_COUNT_1_BIT;
		colorAttachment[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorAttachment[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorAttachment[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		colorAttachment[1].format = VK_FORMAT_D32_SFLOAT;
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
		colorAttachmentRef[1].layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
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
		renderPassInfo.dependencyCount = 1;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		renderPassInfo.pDependencies = &dependency;
		vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
		std::cout << "log: main renderpass created" << std::endl;
	}
	std::vector<VkFramebuffer> swapChainFramebuffers;
	void createdepthbuffer() {
		VkImageCreateInfo imagecreate{};
		imagecreate.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imagecreate.imageType = VK_IMAGE_TYPE_2D;
		imagecreate.arrayLayers = 1;
		imagecreate.extent.width = resolution.x;
		imagecreate.extent.height = resolution.y;
		imagecreate.extent.depth = 1;
		imagecreate.format = VK_FORMAT_D32_SFLOAT;
		imagecreate.tiling = VK_IMAGE_TILING_OPTIMAL;
		imagecreate.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imagecreate.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imagecreate.mipLevels = 1;
		imagecreate.samples = VK_SAMPLE_COUNT_1_BIT;
		imagecreate.queueFamilyIndexCount = 1;
		imagecreate.pQueueFamilyIndices = &queueFamilyIndex;
		imagecreate.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		vkCreateImage(device, &imagecreate, nullptr, &depthImage);

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(device, depthImage, &memRequirements);
		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		vkAllocateMemory(device, &allocInfo, nullptr, &depthImageMemory);
		vkBindImageMemory(device, depthImage, depthImageMemory, 0);

		std::cout << "log: depth image created" << std::endl;
		VkImageViewCreateInfo viewinfo{};
		viewinfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewinfo.image = depthImage;
		viewinfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewinfo.format = VK_FORMAT_D32_SFLOAT;
		viewinfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewinfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewinfo.subresourceRange.baseMipLevel = 0;
		viewinfo.subresourceRange.levelCount = 1;
		viewinfo.subresourceRange.baseArrayLayer = 0;
		viewinfo.subresourceRange.layerCount = 1;
		vkCreateImageView(device, &viewinfo, nullptr, &depthImageView);
		std::cout << "log: image view created" << std::endl;
	}
	void createswfrm() {
		swapChainFramebuffers.resize(swapChainImageViews.size());
		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			VkImageView attachments[] = {
				swapChainImageViews[i],
				depthImageView
			};
			VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = 2;
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.width = resolution.x;
			framebufferInfo.height = resolution.y;
			framebufferInfo.layers = 1;

			vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]);
			std::cout << "log: swapchain framebuffer created" << std::endl;
		}
	}
	void createcomandpoolbuffer() {
		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		VkCommandBufferAllocateInfo allocInfo{};
		vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool);
		std::cout << "log: command pool created" << std::endl;
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = commandPool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;
		allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
		vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data());
		std::cout << "log: command buffer created" << std::endl;
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
	VkQueue graphicsQueue{};
	void recreateswap() {
		vkDeviceWaitIdle(device);
		for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
			vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
		}

		for (size_t i = 0; i < swapChainImageViews.size(); i++) {
			vkDestroyImageView(device, swapChainImageViews[i], nullptr);
		}
		vkDestroyImageView(device, depthImageView, nullptr);
		vkDestroyImage(device, depthImage, nullptr);
		vkFreeMemory(device, depthImageMemory, nullptr);

		createdepthbuffer();
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		createswapchain();
		createswfrm();
	}
public:
	uint32_t currentFrame = 0;
	VkCommandPool commandPool{};
	std::vector<VkCommandBuffer> commandBuffers;
	VkInstance instance{};
	VkQueue presentQueue{};
	VkDevice device{};
	VkRenderPass renderPass{};
    #ifdef _WIN32
	GLFWwindow *window;
    #elif __ANDROID__
    ANativeWindow *window;
	ANativeWindow_Buffer windowbuffer;
    #endif
	glm::ivec2 resolution = glm::ivec2(800, 600);
	glm::ivec2 oldres = glm::ivec2(800, 600);
	bool uselayer = false;
    #ifdef __ANDROID__
	void init(std::string appname, JNIEnv* env, jobject& surf) {
    #else
    void init(std::string appname) {
    #endif
		#ifdef _WIN32
			std::cout << "log: platform = WIN32" << std::endl;
			glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			window = glfwCreateWindow(resolution.x, resolution.y, (appname + " - Windows").c_str(), nullptr, nullptr);
			std::cout << "log: window created" << std::endl;
        #elif __ANDROID__
		window = ANativeWindow_fromSurface(env, surf);
        resolution.x = ANativeWindow_getWidth(window);
		resolution.y = ANativeWindow_getHeight(window);
		#endif
		createInstance(appname);
		getDevice();
		createSurface();
		createswapchain();
		createdepthbuffer();
		createrepass();
		createswfrm();
		createcomandpoolbuffer();
		createsync();
		std::cout << "log: engine initied with success" << std::endl;
	}
	bool shouldterminate() {
		#ifdef _WIN32
			return !glfwWindowShouldClose(window);
        #else
        return true;
		#endif
	}
	void beginmainpass() {
		#ifdef _WIN32
		glfwGetFramebufferSize(window, &resolution.x, &resolution.y);
		#endif
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
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent.width = resolution.x;
		renderPassInfo.renderArea.extent.height = resolution.y;
		std::vector<VkClearValue> clearColor(2);
		clearColor[0] = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
		clearColor[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = clearColor.data();

		vkCmdBeginRenderPass(commandBuffers[currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	void endmainpass() {
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

		VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]);

		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;

		VkResult res = vkQueuePresentKHR(presentQueue, &presentInfo);

		if (res == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateswap();
		}else if (resolution.x != oldres.x || resolution.y != oldres.y) {
			recreateswap();
		}

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		oldres.x = resolution.x;
		oldres.y = resolution.y;

		#ifdef _WIN32
		glfwPollEvents();
        #elif __ANDROID__
		ANativeWindow_lock(window, &windowbuffer, NULL);
		memcpy(windowbuffer.bits, &swapChainImages[imageIndex],  resolution.x*resolution.y*4); // ARGB_8888
		ANativeWindow_unlockAndPost(window);
		#endif
	}
	void terminate() {
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
			vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
			vkDestroyFence(device, inFlightFences[i], nullptr);
		}
		vkDestroyCommandPool(device, commandPool, nullptr);
		for (auto framebuffer : swapChainFramebuffers) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
		}
		vkDestroyRenderPass(device, renderPass, nullptr);
		for (auto imageView : swapChainImageViews) {
			vkDestroyImageView(device, imageView, nullptr);
		}
		vkDestroySwapchainKHR(device, swapChain, nullptr);
		vkDestroySurfaceKHR(instance, surface, nullptr);
		vkDestroyDevice(device, nullptr);
		vkDestroyInstance(instance, nullptr);
        #ifdef _WIN32
        glfwDestroyWindow(window);
		glfwTerminate();
        #elif __ANDROID__
        #endif
	}
};

class Mesh {
private:
	static std::vector<char> loadbin(const std::string& filename) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) {
			throw std::runtime_error("Error:Failed to open file");
		}
		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}
	VkShaderModule createShaderModule(const std::vector<char>& code, Engine eng) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
		VkShaderModule shaderModule;
		if (vkCreateShaderModule(eng.device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}
		return shaderModule;
	}
	VkPipeline graphicsPipeline{};
	void createPipeline(Engine eng, std::string vertshader, std::string fragshader) {
		auto vertShaderCode = loadbin(vertshader);
		auto fragShaderCode = loadbin(fragshader);

		VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, eng);
		VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, eng);

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
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)eng.resolution.x;
		viewport.height = (float)eng.resolution.y;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent.width = eng.resolution.x;
		scissor.extent.height = eng.resolution.y;

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
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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

		VkPipelineLayout pipelineLayout;
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		vkCreatePipelineLayout(eng.device, &pipelineLayoutInfo, nullptr, &pipelineLayout);

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
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
		pipelineInfo.renderPass = eng.renderPass;
		pipelineInfo.subpass = 0;
		vkCreateGraphicsPipelines(eng.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline);
		std::cout << "log: pipeline created" << std::endl;
	}
	std::string vs{};
	std::string fs{};
public:
	void create(Engine eng, std::string vertshader, std::string fragshader) {
		createPipeline(eng, vertshader, fragshader);
		vs = vertshader;
		fs = fragshader;
	}
	void Draw(Engine& eng) {
		if (eng.resolution.x != eng.oldres.x || eng.resolution.y != eng.oldres.y) {
			vkDestroyPipeline(eng.device, graphicsPipeline, nullptr);
			createPipeline(eng, vs, fs);
		}
		vkCmdBindPipeline(eng.commandBuffers[eng.currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(eng.resolution.x);
		viewport.height = static_cast<float>(eng.resolution.y);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(eng.commandBuffers[eng.currentFrame], 0, 1, &viewport);
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent.width = eng.resolution.x;
		scissor.extent.height = eng.resolution.y;
		vkCmdSetScissor(eng.commandBuffers[eng.currentFrame], 0, 1, &scissor);

		vkCmdDraw(eng.commandBuffers[eng.currentFrame], 3, 1, 0, 0);
	}
};