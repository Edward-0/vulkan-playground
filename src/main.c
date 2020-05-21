#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <sys/param.h>

struct List {
	uint32_t count;
	void *data;
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

struct QueueFamilyIndicies {
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

struct QueueFamilyIndicies getQueueFamilies(VkSurfaceKHR surface, VkPhysicalDevice device) {
	struct QueueFamilyIndicies ret;
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

VkDevice createLogicalDevice(VkPhysicalDevice physicalDevice, struct QueueFamilyIndicies indices) {

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

VkQueue getGraphicsQueue(VkDevice device, struct QueueFamilyIndicies indices) {
	VkQueue ret = VK_NULL_HANDLE;

	vkGetDeviceQueue(device, indices.graphicsFamily, 0, &ret);

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

VkSwapchainKHR createSwapChain(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, struct QueueFamilyIndicies indices) {

	struct SwapChainSupportDetails swapChainSupport = getSwapChainSupport(physicalDevice, surface);

	VkSurfaceFormatKHR surfaceFormat = getSwapSurfaceFormat(swapChainSupport);
	VkPresentModeKHR presentMode = getPresentMode(swapChainSupport);
	VkExtent2D extent = getSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
		imageCount = swapChainSupport.capabilities.maxImageCount;
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

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = VK_NULL_HANDLE;
	createInfo.pNext = NULL;


	VkSwapchainKHR ret;

	fprintf(stderr, "ʕっ•ᴥ•ʔっ\n");

	if (vkCreateSwapchainKHR(device, &createInfo, NULL, &ret) != VK_SUCCESS) {
		fprintf(stderr, "Failed to create swap chain!");
	}

	fprintf(stderr, "ʕっ•ᴥ•ʔっ\n");

	return ret;
}

VkImage *getSwapChainImages(VkDevice device, VkSwapchainKHR swapChain, uint32_t *imageCount) {
	vkGetSwapchainImagesKHR(device, swapChain, imageCount, NULL);
	
	VkImage *ret = malloc(sizeof(VkImage) * *imageCount);

	vkGetSwapchainImagesKHR(device, swapChain, imageCount, ret);

	return ret;

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

	struct QueueFamilyIndicies queueFamilies = getQueueFamilies(surface, physicalDevice);

	VkDevice device = createLogicalDevice(physicalDevice, queueFamilies);

	VkQueue graphicsQueue = getGraphicsQueue(device, queueFamilies);

	VkSwapchainKHR swapChain = createSwapChain(physicalDevice, device, surface, queueFamilies);


	uint32_t imageCount;
	VkImage *images = getSwapChainImages(device, swapChain, &imageCount);

	while(glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, NULL);
	}

	vkDestroySwapchainKHR(device, swapChain, NULL);

	vkDestroySurfaceKHR(instance, surface, NULL);

	vkDestroyDevice(device, NULL);

	vkDestroyInstance(instance, NULL);

	glfwDestroyWindow(window);

	glfwTerminate();

	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  	return 0;
}
