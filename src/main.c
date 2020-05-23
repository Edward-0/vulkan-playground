#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <AL/al.h>
#include <AL/alc.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <sys/param.h>

struct Vec4 {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
	union {
		float z;
		float b;
	};
	union {
		float w;
		float a;
	};
};

struct Vec3 {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
	union {
		float z;
		float b;
	};
};

struct Vec2 {
	union {
		float x;
		float r;
	};
	union {
		float y;
		float g;
	};
};

struct Vertex {
	struct Vec2 position;
	struct Vec3 colour;
};

const struct Vertex vertices[3] = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};

VkVertexInputBindingDescription getVertexBindingDescription() {
	VkVertexInputBindingDescription bindingDescription = {};
	bindingDescription.binding = 0;
	bindingDescription.stride = sizeof(struct Vertex);
	bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return bindingDescription;
}

VkVertexInputAttributeDescription *getVertexAttributeBindingDescriptions() {
	VkVertexInputAttributeDescription *attributeDescriptions = malloc(sizeof(VkVertexInputAttributeDescription) * 2);

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	attributeDescriptions[0].offset = offsetof(struct Vertex, position);
	
	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	attributeDescriptions[1].offset = offsetof(struct Vertex, colour);

	return attributeDescriptions;
}

const int MAX_FRAMES_IN_FLIGHT = 2;

struct List {
	uint32_t count;
	void *data;
};

struct SyncObjects {
	VkSemaphore *imageAvailable;
	VkSemaphore *renderFinished;
	VkFence *inFlightFence;
	VkFence *imageInFlight;
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t formatCount;
	VkSurfaceFormatKHR *formats;
	uint32_t presentModeCount;
	VkPresentModeKHR *presentModes;
};


struct SwapChainSupportDetails getSwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
	struct SwapChainSupportDetails ret;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &ret.capabilities);

	
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &ret.formatCount, NULL);

	ret.formats = malloc(sizeof(VkSurfaceFormatKHR) * ret.formatCount);

	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &ret.formatCount, ret.formats);


	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &ret.presentModeCount, NULL);

	ret.presentModes = malloc(sizeof(VkPresentModeKHR) * ret.presentModeCount);

	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &ret.presentModeCount, ret.presentModes);
	
	return ret;
}

struct QueueFamilyIndices {
	bool isComplete;
	bool hasGraphicsFamily;
	uint32_t graphicsFamily;
	bool hasPresentFamily;
	uint32_t presentFamily;
};

const int layerNameCount = 1;
char const * layerNames[1] = {
	"VK_LAYER_KHRONOS_validation"
/*	"VK_LAYER_LUNARG_standard_validation",*/
/*	"VK_LAYER_LUNARG_vktrace",*/
};

const int extensionNameCount = 1;
char const * extensionNames[1] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

const bool enableValidationLayers = true;

bool checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, NULL);

	VkLayerProperties *availableLayers = malloc(sizeof(VkLayerProperties) * layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers);

	for (int i = 0; i < layerCount; i++) {
		fprintf(stderr, "Available Layer: %s\n", availableLayers[i].layerName);
	}

	bool allLayersFound = true;

	for (int i = 0; i < layerNameCount && allLayersFound; i++) {
		bool layerFound = false;
		fprintf(stderr, "Looking for layer %s\n", layerNames[i]);
		for (int j = 0; j < layerCount && !layerFound; j++) {	
			if (!strcmp(layerNames[i], availableLayers[j].layerName)) {
				fprintf(stderr, "Found %s\n", layerNames[i]);
				layerFound = true;
			}
		}
		if (!layerFound) {
			fprintf(stderr, "Failed to find layer %s\n", layerNames[i]);
		}
		allLayersFound = layerFound;
	}

	return allLayersFound;
}

VkApplicationInfo createAppInfo() {
	VkApplicationInfo ret;
	ret.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ret.pApplicationName = "Hello Triangle";
	ret.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	ret.pEngineName = "No Engine";
	ret.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	ret.apiVersion = VK_API_VERSION_1_0;
	ret.pNext = NULL;
	return ret;
}

char const ** getRequiredExtensions(uint32_t *count) {
	char const **ret;

	uint32_t glfwExtensionCount;
	char const **glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	if (enableValidationLayers) {
		*count = glfwExtensionCount + 1;
		ret = malloc(sizeof(char const *) * (*count));
		memcpy(ret, glfwExtensions, sizeof(char const *) * (*count));
		ret[glfwExtensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
	} else {
		*count = glfwExtensionCount + extensionNameCount;
		ret = glfwExtensions;
	}

	return ret;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	fprintf(stderr, "Validation layer: %s\n", pCallbackData->pMessage);

	return VK_FALSE;
}


VkDebugUtilsMessengerCreateInfoEXT createDebugUtilsMessengerCreateInfo() {
	VkDebugUtilsMessengerCreateInfoEXT ret = {};
	ret.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	ret.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	ret.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	ret.pfnUserCallback = debugCallback;
	ret.pUserData = NULL; // Optional	
	return ret;
}

VkInstance createInstance() {
	VkApplicationInfo appInfo = createAppInfo();

	VkInstanceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	uint32_t extensionCount = 0;
	char const **extensions;

	extensions = getRequiredExtensions(&extensionCount);

	createInfo.enabledExtensionCount = extensionCount;
	createInfo.ppEnabledExtensionNames = extensions;
	createInfo.flags = 0;

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = layerNameCount;
		createInfo.ppEnabledLayerNames = layerNames;
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo;
		debugMessengerCreateInfo = createDebugUtilsMessengerCreateInfo();
		createInfo.pNext = &debugMessengerCreateInfo;
	} else {
		createInfo.enabledLayerCount = 0;
	}
	VkInstance ret;
	if (vkCreateInstance(&createInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create Vulkan instance!");
		exit(EXIT_FAILURE);	
	}
	return ret;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    PFN_vkDestroyDebugUtilsMessengerEXT func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != NULL) {
        func(instance, debugMessenger, pAllocator);
    }
}

VkDebugUtilsMessengerEXT createDebugMessenger(VkInstance instance) {

	VkDebugUtilsMessengerEXT ret;

	VkDebugUtilsMessengerCreateInfoEXT createInfo = createDebugUtilsMessengerCreateInfo();
	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Unable to create debug messenger!\n");
	}
	return ret;
}

void enumerateExtensions() {
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, NULL);

	VkExtensionProperties *extensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
	vkEnumerateInstanceExtensionProperties(NULL, &extensionCount, extensions);
	for (int i = 0; i < extensionCount; i++) {
		fprintf(stderr, "Available Extension: %s\n", extensions[i].extensionName);
	}
	free(extensions);
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, NULL);

	VkExtensionProperties* availableExtensions = malloc(sizeof(VkExtensionProperties) * extensionCount);
	vkEnumerateDeviceExtensionProperties(device, NULL, &extensionCount, availableExtensions);

	bool allExtensionsFound = true;

	for (int i = 0; i < extensionNameCount && allExtensionsFound; i++) {
		bool extensionFound = false;
		fprintf(stderr, "Looking for extension %s\n", extensionNames[i]);
		for (int j = 0; j < extensionCount && !extensionFound; j++) {	
			if (!strcmp(extensionNames[i], availableExtensions[j].extensionName)) {
				fprintf(stderr, "Found %s\n", extensionNames[i]);
				extensionFound = true;
			}
		}
		if (!extensionFound) {
			fprintf(stderr, "Failed to find extension %s\n", extensionNames[i]);
		}
		allExtensionsFound = extensionFound;
	}

	return allExtensionsFound;
}


bool isSuitiblePysicalDevice(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	bool swapChainAdequate = false;

	return deviceFeatures.geometryShader
		&& checkDeviceExtensionSupport(device);
}

uint32_t ratePysicalDevice(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	int i = 0;
	uint32_t ret = 0;

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
		ret += 1024;
	}

	ret += deviceProperties.limits.maxImageDimension2D;

	if (!isSuitiblePysicalDevice(device)) {
		ret = 0;
	}

	return ret;
}

VkPhysicalDevice getPhysicalDevice(VkInstance instance) {
	uint32_t physicalDeviceCount;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, NULL);
	VkPhysicalDevice *physicalDevices = malloc(sizeof(VkPhysicalDevice) * physicalDeviceCount);
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

	uint32_t rating = 0;
	VkPhysicalDevice device = VK_NULL_HANDLE;

	for (int i = 0; i < physicalDeviceCount; i++) {
		uint32_t iRating = ratePysicalDevice(physicalDevices[i]);
		if (iRating > rating) {
			rating = iRating;
			device = physicalDevices[i];
		}

		//vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//fprintf(stderr, "%d\n", iRating);
	}

	if (!rating) {
		fprintf(stderr, "Failed to find a suitable GPU!");
		exit(EXIT_FAILURE);
	}

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	fprintf(stderr, "Using %s\n", deviceProperties.deviceName);
	return device;
}

struct QueueFamilyIndices getQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice device) {
	struct QueueFamilyIndices ret;
	ret.isComplete = false;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);

	VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies);

	fprintf(stderr, "Queue Family Count: %d\n", queueFamilyCount);

	for (int i = 0; i < queueFamilyCount; i++) {
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			fprintf(stderr, "Found graphics queue family.\n");
			ret.hasGraphicsFamily= true;
			ret.graphicsFamily = i;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (presentSupport) {
			fprintf(stderr, "Found present queue family.\n");
			ret.hasPresentFamily = true;
			ret.presentFamily = i;
		}
	}

	ret.isComplete = ret.hasGraphicsFamily && ret.hasPresentFamily;

	if (!ret.isComplete) {
		fprintf(stderr, "No Grapsics Queue\n");
		exit(EXIT_FAILURE);
	}

	return ret;
}

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, struct QueueFamilyIndices indices) {

	VkDevice ret;

	VkDeviceQueueCreateInfo queueCreateInfo = {0};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = indices.graphicsFamily;
	queueCreateInfo.queueCount = 1;

	float queuePriority = 1.0f;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	VkDeviceQueueCreateInfo *queueCreateInfos = malloc(sizeof(VkDeviceQueueCreateInfo) * 2);
	uint32_t uniqueQueueFamilies[2] = {indices.graphicsFamily, indices.presentFamily};

	for (uint32_t i = 0; i < 2; i ++) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos[i] = queueCreateInfo;
	}

	createInfo.pQueueCreateInfos = queueCreateInfos;
	if (indices.graphicsFamily == indices.presentFamily) {
		createInfo.queueCreateInfoCount = 1; // Hack can't be bothered to check for duplicate queue families
	} else {
		createInfo.queueCreateInfoCount = 2;
	}

	createInfo.pEnabledFeatures = &deviceFeatures;

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = layerNameCount;
		createInfo.ppEnabledLayerNames = layerNames;
	} else {
		createInfo.enabledLayerCount = 0;
	}

	createInfo.enabledExtensionCount = extensionNameCount;
	createInfo.ppEnabledExtensionNames = extensionNames;

	if (vkCreateDevice(physicalDevice, &createInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create logical device!");
		exit(EXIT_FAILURE);
	}

	return ret;
}

VkQueue getGraphicsQueue(VkDevice device, struct QueueFamilyIndices indices) {
	VkQueue ret = VK_NULL_HANDLE;

	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &ret);

	return ret;
}

VkQueue getPresentQueue(VkDevice device, struct QueueFamilyIndices indices) {
	VkQueue ret = VK_NULL_HANDLE;

	vkGetDeviceQueue(device, indices.presentFamily, 0, &ret);

	return ret;
}

VkSurfaceKHR createSurface(VkInstance instance, GLFWwindow* window) {
	VkSurfaceKHR surface;
	glfwCreateWindowSurface(instance, window, NULL, &surface);
	return surface;
}

VkSurfaceFormatKHR getSwapSurfaceFormat(struct SwapChainSupportDetails swapChainSupportDetails) {
	VkSurfaceFormatKHR ret;

	bool found = false;

	for (int i = 0; i < swapChainSupportDetails.formatCount; i++) {
		VkSurfaceFormatKHR format = swapChainSupportDetails.formats[i];

		if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			ret = format;
			found = true;
		}
	}

	if (!found) {
		fprintf(stderr, "Failed to find format!\nDefaulting to first format out of %d\n", swapChainSupportDetails.formatCount);
		ret = swapChainSupportDetails.formats[0];
	}

	return ret; 
}

VkPresentModeKHR getPresentMode(struct SwapChainSupportDetails swapChainSupportDetails) {
	VkPresentModeKHR ret;

	bool found;

	for (int i = 0; i < swapChainSupportDetails.presentModeCount; i++) {
		VkPresentModeKHR presentMode = swapChainSupportDetails.presentModes[i];

		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			ret = presentMode;
			found = true;
		}
	}

	if (!found) {
		fprintf(stderr, "VK_PRESENT_MODE_MAILBOX_KHR Not Supported!\nDefaulting to VK_PRESENT_MODE_FIFO_KHR\n");
		ret = VK_PRESENT_MODE_FIFO_KHR;
	}

	return ret;
}

VkExtent2D getSwapExtent(const VkSurfaceCapabilitiesKHR capabilities) {
	VkExtent2D ret;
	if (capabilities.currentExtent.width != UINT32_MAX) {
        ret = capabilities.currentExtent;
    } else {
        VkExtent2D actualExtent = {800, 600};

        actualExtent.width = MAX(capabilities.minImageExtent.width, MIN(capabilities.maxImageExtent.width, actualExtent.width));
        actualExtent.height = MAX(capabilities.minImageExtent.height, MIN(capabilities.maxImageExtent.height, actualExtent.height));

        ret = actualExtent;
    }
	return ret;
}

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, 
		VkSurfaceCapabilitiesKHR capabilities, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, 
		VkExtent2D extent, struct QueueFamilyIndices indices) {

	uint32_t imageCount = capabilities.minImageCount + 1;

	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.flags = VK_NULL_HANDLE;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = NULL; // Optional
	}

	createInfo.preTransform = capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.pNext = NULL;


	VkSwapchainKHR ret;

	if (vkCreateSwapchainKHR(device, &createInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create swap chain!");
	}

	return ret;
}

VkImage *getSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32_t *imageCount) {
	vkGetSwapchainImagesKHR(device, swapChain, imageCount, NULL);
	
	VkImage *ret = malloc(sizeof(VkImage) * *imageCount);

	vkGetSwapchainImagesKHR(device, swapChain, imageCount, ret);

	return ret;

}

VkImageView *createImageViews(VkDevice device, VkFormat format, uint32_t imageCount, VkImage* swapChainImages) {
	VkImageView *ret = malloc(sizeof(VkImageView) * imageCount);
	
	for (int i = 0; i < imageCount; i ++) {
		VkImageViewCreateInfo createInfo = {0};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];

		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, NULL, &ret[i]) != VK_SUCCESS) {
			fprintf(stderr, "Failed to create ImageView %d", i);
			exit(EXIT_FAILURE);
		}
	}

	return ret;
}

void const *readFile(char const *path, uint32_t *size) {
	FILE *file = fopen(path, "r");
	fseek(file, 0, SEEK_END);
	*size = ftell(file);
	fseek(file, 0, SEEK_SET);

	void *ret = malloc(*size);
	fread(ret, 1, *size, file);

	return ret;
}

VkShaderModule createShaderModule(VkDevice device, void const *source, uint32_t size) {
	
	VkShaderModuleCreateInfo createInfo = {0};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize  = size;
	createInfo.pCode = source;

	VkShaderModule ret;

	if (vkCreateShaderModule(device, &createInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create shader module!\n");
	}

	return ret;
}

VkPipelineLayout createPipelineLayout(VkDevice device) {

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {0};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = NULL; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = NULL; // Optional

		
	VkPipelineLayout ret;

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create pipeline layout!");
		exit(EXIT_FAILURE);
	}

	return ret;
}

VkPipeline createPipeline(VkDevice device, VkExtent2D swapChainExtent, VkRenderPass renderPass, VkPipelineLayout layout) {
	uint32_t vertSize;
	void const *vert = readFile("vert.spv", &vertSize);

	uint32_t fragSize;
	void const *frag = readFile("frag.spv", &fragSize);

	VkShaderModule vertModule = createShaderModule(device, vert, vertSize);
	VkShaderModule fragModule = createShaderModule(device, frag, fragSize);


	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {0};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	
	vertShaderStageInfo.module = vertModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {0};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	
	fragShaderStageInfo.module = fragModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[2] = {vertShaderStageInfo, fragShaderStageInfo};

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {0};
	
	VkVertexInputBindingDescription bindingDescription = getVertexBindingDescription();
	VkVertexInputAttributeDescription *attributeDescriptions = getVertexAttributeBindingDescriptions();

	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 2;
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {0};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {0};
	scissor.offset = (VkOffset2D) {0, 0};
	scissor.extent = swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {0};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {0};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;

	rasterizer.rasterizerDiscardEnable = VK_FALSE;

	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;

	rasterizer.lineWidth = 1.0f;

	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = NULL; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {0};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {0};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {0};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;
	
	VkGraphicsPipelineCreateInfo pipelineInfo = {0};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;

	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = NULL; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = NULL; // Optional

	pipelineInfo.layout = layout;

	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;

	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	VkPipeline ret;

	if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create pipeline!\n");
		exit(EXIT_FAILURE);
	}

	vkDestroyShaderModule(device, fragModule, NULL);
	vkDestroyShaderModule(device, vertModule, NULL);

	return ret;	

}

VkRenderPass createRenderPass(VkDevice device, VkFormat swapChainImageFormat) {
	VkAttachmentDescription colorAttachment = {0};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {0};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPass ret;

	VkRenderPassCreateInfo renderPassInfo = {0};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	
	VkSubpassDependency dependency = {0};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;

	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;

	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create render Pass");
	}

	return ret;
}

VkFramebuffer *createFrameBuffers(VkDevice device, VkRenderPass renderPass, VkExtent2D extent, uint32_t count, VkImageView *swapChainImageViews) {
	VkFramebuffer *ret = malloc(sizeof(VkFramebuffer) * count);

	for (int i = 0; i < count; i++) {
		VkImageView attachments[1] = {
			swapChainImageViews[i]
		};
	
		VkFramebufferCreateInfo frameBufferInfo = {0};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = renderPass;
		frameBufferInfo.attachmentCount = 1;
		frameBufferInfo.pAttachments = attachments;
		frameBufferInfo.width = extent.width;
		frameBufferInfo.height = extent.height;
		frameBufferInfo.layers = 1;

		if (vkCreateFramebuffer(device, &frameBufferInfo, NULL, &ret[i]) != VK_SUCCESS) {
			fprintf(stderr, "Failed to create framebuffer %d!\n", i);
			exit(EXIT_FAILURE);
		}
	}	

	return ret;
}

VkCommandPool createCommandPool(VkDevice device, struct QueueFamilyIndices queueFamilyIndices) {

	VkCommandPoolCreateInfo poolInfo = {0};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0; // Optional

	VkCommandPool ret;

	if (vkCreateCommandPool(device, &poolInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create commandPool!\n");
		exit(EXIT_FAILURE);
	}

	return ret;
}

VkCommandBuffer *createCommandBuffers(VkDevice device, VkPipeline pipeline, VkRenderPass renderPass, VkExtent2D extent, VkCommandPool commandPool, uint32_t count, VkFramebuffer *framebuffers) {
	VkCommandBuffer *ret = malloc(sizeof(VkCommandBuffer) * count);

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = count;

	if (vkAllocateCommandBuffers(device, &allocInfo, ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to allocate command buffers!\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < count; i++) {
		VkCommandBufferBeginInfo beginInfo = {0};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;
		beginInfo.pInheritanceInfo = NULL;

		if (vkBeginCommandBuffer(ret[i], &beginInfo) != VK_SUCCESS) {
			fprintf(stderr, "Failed to begin recording command buffer %d!\n", i);
		}
	
		VkRenderPassBeginInfo renderPassInfo = {0};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = framebuffers[i];

		renderPassInfo.renderArea.offset = (VkOffset2D) {0, 0};
		renderPassInfo.renderArea.extent = extent;

		VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(ret[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	
		vkCmdBindPipeline(ret[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

		vkCmdDraw(ret[i], 3, 1, 0, 0);

		vkCmdEndRenderPass(ret[i]);

		if (vkEndCommandBuffer(ret[i]) != VK_SUCCESS) {
			fprintf(stderr, "Failed to stop recording command buffer %d!\n", i);
			exit(EXIT_FAILURE);
		}
	}

	return ret;
}

struct SyncObjects createSyncObjects(VkDevice device, uint32_t count) {
	struct SyncObjects ret;
	ret.imageAvailable = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT); 
	ret.renderFinished = malloc(sizeof(VkSemaphore) * MAX_FRAMES_IN_FLIGHT);
	ret.inFlightFence = malloc(sizeof(VkFence) * MAX_FRAMES_IN_FLIGHT);
	ret.imageInFlight = malloc(sizeof(VkFence) * count);

	for (int i = 0; i < count; i++) {
		ret.imageInFlight[i] = VK_NULL_HANDLE;
	}

	VkSemaphoreCreateInfo semaphoreInfo = {0};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {0};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &ret.imageAvailable[i]) != VK_SUCCESS) {
			fprintf(stderr, "Failed to create image available semaphore for frame %d!\n", i);
			exit(EXIT_FAILURE);
		}

		if (vkCreateSemaphore(device, &semaphoreInfo, NULL, &ret.renderFinished[i]) != VK_SUCCESS) {
			fprintf(stderr, "Failed to create image available semaphore for frame %d!\n", i);
			exit(EXIT_FAILURE);
		}

		if (vkCreateFence(device, &fenceInfo, NULL, &ret.inFlightFence[i]) != VK_SUCCESS) {
			fprintf(stderr, "Failed to create in flight fence for frame %d!\n", i);
			exit(EXIT_FAILURE);
		}
	}
	return ret;
}

void drawFrame(VkDevice device, VkSwapchainKHR swapChain, VkQueue graphicsQueue, VkQueue presentQueue, VkCommandBuffer *commandBuffers, struct SyncObjects semaphores, uint32_t currentFrame) {
	vkWaitForFences(device, 1, (semaphores.inFlightFence + currentFrame), VK_TRUE, UINT64_MAX);
	//vkResetFences(device, 1, &semaphores.inFlightFence[currentFrame]);

	uint32_t imageIndex;
	//printf("%d\n", imageIndex);
	vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, semaphores.imageAvailable[currentFrame], VK_NULL_HANDLE, &imageIndex);

	if (semaphores.imageInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(device, 1, &semaphores.imageInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	semaphores.imageInFlight[imageIndex] = semaphores.inFlightFence[currentFrame];

	VkSubmitInfo submitInfo = {0};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = {semaphores.imageAvailable[currentFrame]};
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = {semaphores.renderFinished[currentFrame]};
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(device, 1, (semaphores.inFlightFence + currentFrame));

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, semaphores.inFlightFence[currentFrame]) != VK_SUCCESS) {
		fprintf(stderr, "Failed to submit draw command buffer\n");
		exit(EXIT_FAILURE);
	}

	VkPresentInfoKHR presentInfo = {0};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = {swapChain};
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = NULL;

	vkQueuePresentKHR(presentQueue, &presentInfo);

	//vkQueueWaitIdle(presentQueue);	
}

VkBuffer createVertexBuffer(VkDevice device) {
	VkBuffer ret;

	VkBufferCreateInfo bufferInfo = {0};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = sizeof(struct Vertex) * 3;
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create vertex buffer!\n");
		exit(EXIT_FAILURE);
	}

	return ret;
}

uint32_t getMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
    	if (typeFilter & (1 << i)) {
        	return i;
    	}
	}

	fprintf(stderr, "Failed to get suitible memory type");
	exit(EXIT_FAILURE);
}

int main() {

	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW!");
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "Vulkan window", NULL, NULL);

	if (!checkValidationLayerSupport()) {
		fprintf(stderr, "Validation layers not available");
		exit(EXIT_FAILURE);
	}

	VkInstance instance = createInstance();

	VkDebugUtilsMessengerEXT debugMessenger;

	if (enableValidationLayers) {
		debugMessenger = createDebugMessenger(instance);
	}

	enumerateExtensions();

	VkSurfaceKHR surface = createSurface(instance, window);

	VkPhysicalDevice physicalDevice = getPhysicalDevice(instance);

	struct QueueFamilyIndices queueFamilies = getQueueFamilies(surface, physicalDevice);

	VkDevice device = createLogicalDevice(physicalDevice, queueFamilies);

	VkQueue graphicsQueue = getGraphicsQueue(device, queueFamilies);

	VkQueue presentQueue = getPresentQueue(device, queueFamilies);

	struct SwapChainSupportDetails swapChainSupport = getSwapChainSupport(physicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = getSwapSurfaceFormat(swapChainSupport);
	VkPresentModeKHR presentMode = getPresentMode(swapChainSupport);
	VkExtent2D extent = getSwapExtent(swapChainSupport.capabilities);

	VkSwapchainKHR swapChain = createSwapChain(physicalDevice, device, surface, swapChainSupport.capabilities, surfaceFormat, presentMode, extent, queueFamilies);

	uint32_t imageCount;
	VkImage *images = getSwapChainImages(device, swapChain, &imageCount);

	VkImageView *imageViews = createImageViews(device, surfaceFormat.format, imageCount, images);

	VkPipelineLayout layout = createPipelineLayout(device);

	VkRenderPass renderPass = createRenderPass(device, surfaceFormat.format);

	VkPipeline pipeline = createPipeline(device, extent, renderPass, layout);

	VkFramebuffer *framebuffers = createFrameBuffers(device, renderPass, extent, imageCount, imageViews);

	VkCommandPool commandPool = createCommandPool(device, queueFamilies);

	VkCommandBuffer *commandBuffers = createCommandBuffers(device, pipeline, renderPass, extent, commandPool, imageCount, framebuffers);
	
	struct SyncObjects syncObjects = createSyncObjects(device, imageCount);

	VkBuffer vertexBuffer = createVertexBuffer(device);

	uint32_t currentFrame = 0;
	while(!glfwWindowShouldClose(window)) {
//		printf("frame: %d\n", currentFrame);
		glfwPollEvents();
		drawFrame(device, swapChain, graphicsQueue, presentQueue, commandBuffers, syncObjects, currentFrame);
		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	vkDeviceWaitIdle(device);

	vkDestroyBuffer(device, vertexBuffer, NULL);

	for (int i = 0; i < imageCount; i++) {
		vkDestroyImageView(device, imageViews[i], NULL);
		vkDestroyFramebuffer(device, framebuffers[i], NULL);
	}

	for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, syncObjects.imageAvailable[i], NULL);
		vkDestroySemaphore(device, syncObjects.renderFinished[i], NULL);
		vkDestroyFence(device, syncObjects.inFlightFence[i], NULL);
	}

	vkDestroyCommandPool(device, commandPool, NULL);

	vkDestroyPipeline(device, pipeline, NULL);

	vkDestroyPipelineLayout(device, layout, NULL);

	vkDestroyRenderPass(device, renderPass, NULL);

	vkDestroySwapchainKHR(device, swapChain, NULL);

	vkDestroySurfaceKHR(instance, surface, NULL);
	
	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
	}

	vkDestroyDevice(device, NULL);

	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);

	glfwTerminate();

	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  	return 0;
}
