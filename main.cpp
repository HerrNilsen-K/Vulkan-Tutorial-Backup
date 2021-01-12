//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>

#define ASSERT_VULKAN(val)                                         \
    if (val != VK_SUCCESS)                                         \
    {                                                              \
        std::cout << "---------------------------------------\n";  \
        std::cout << "ERROR: 'RESULT != VK_SUCCESS'" << std::endl; \
        std::cout << __FILE__ << ": " << __LINE__ << std::endl;    \
        std::cout << "---------------------------------------\n";  \
        assert(false);                                             \
    }

VkInstance instance;
VkSurfaceKHR surface;
VkDevice device;
VkSwapchainKHR swapchain;
VkShaderModule shaderModuleVert;
VkShaderModule shaderModuleFrag;
VkImageView *imageViews;
GLFWwindow *window;

uint32_t amountOfImagesInSwapchain = 0;
const uint32_t WIDTH = 400, HEIGHT = 300;

//Print some stats about the graphics card
void printStats(const VkPhysicalDevice &device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    uint32_t apiVer = properties.apiVersion;

    std::cout << "Name:                     " << properties.deviceName << std::endl
              << "API Version:              " << VK_VERSION_MAJOR(apiVer) << '.' << VK_VERSION_MINOR(apiVer) << '.' << VK_VERSION_PATCH(apiVer) << std::endl
              << "Driver Version:           " << properties.driverVersion << std::endl
              << "Vendor ID:                " << properties.vendorID << std::endl
              << "Device ID:                " << properties.deviceID << std::endl
              << "Device Type:              " << properties.deviceType << std::endl
              << "DiscreteQueuePriorities:  " << properties.limits.discreteQueuePriorities << std::endl;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    std::cout << "Geometry Shader:          " << features.geometryShader << std::endl;

    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(device, &memProp);

    uint32_t amountOfQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilies, NULL);
    VkQueueFamilyProperties *familyProperties = new VkQueueFamilyProperties[amountOfQueueFamilies];
    vkGetPhysicalDeviceQueueFamilyProperties(device, &amountOfQueueFamilies, familyProperties);

    std::cout << "Amount of Queue Families: " << amountOfQueueFamilies << std::endl;

    for (int i = 0; i < amountOfQueueFamilies; i++)
    {
        std::cout << std::endl;
        std::cout << "Queue Familie #" << i << std::endl;
        std::cout << "VK_QUEUE_GRAPHICS_BIT         " << ((familyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) ? "true" : "false") << std::endl;
        std::cout << "VK_QUEUE_COMPUTE_BIT          " << ((familyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) ? "true" : "false") << std::endl;
        std::cout << "VK_QUEUE_TRANSFER_BIT         " << ((familyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) ? "true" : "false") << std::endl;
        std::cout << "VK_QUEUE_SPARSE_BINDING_BIT   " << ((familyProperties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) ? "true" : "false") << std::endl;
        std::cout << "Queue Count: " << familyProperties[i].queueCount << std::endl;
        std::cout << "Timestamp valid Bits: " << familyProperties[i].timestampValidBits << std::endl;
        uint32_t width = familyProperties[i].minImageTransferGranularity.width;
        uint32_t height = familyProperties[i].minImageTransferGranularity.height;
        uint32_t depth = familyProperties[i].minImageTransferGranularity.depth;
        std::cout << "Min image Timestamp Grabularity: " << width << ", " << height << ", " << depth << std::endl;
    }

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &surfaceCapabilities);

    std::cout << "\tSurface capabilities:         " << std::endl;
    std::cout << "\tMin Image Count:              " << surfaceCapabilities.minImageCount << std::endl;
    std::cout << "\tMax Image Count:              " << surfaceCapabilities.maxImageCount << std::endl;
    std::cout << "\tCurrent Extent:               " << surfaceCapabilities.currentExtent.width << '/' << surfaceCapabilities.currentExtent.height << std::endl;
    std::cout << "\tMin Image Extent:             " << surfaceCapabilities.minImageExtent.width << '/' << surfaceCapabilities.minImageExtent.height << std::endl;
    std::cout << "\tMax Image Extent:             " << surfaceCapabilities.maxImageExtent.width << '/' << surfaceCapabilities.maxImageExtent.height << std::endl;
    std::cout << "\tMax Image Array Layers:       " << surfaceCapabilities.maxImageArrayLayers << std::endl;
    std::cout << "\tSupported Transforms:         " << surfaceCapabilities.supportedTransforms << std::endl;
    std::cout << "\tCurrent Transforms:           " << surfaceCapabilities.currentTransform << std::endl;
    std::cout << "\tSupported Composite Alpha:    " << surfaceCapabilities.supportedCompositeAlpha << std::endl;
    std::cout << "\tSupported Usage Flags:        " << surfaceCapabilities.supportedUsageFlags << std::endl;

    uint32_t amountOfFormats = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, NULL);
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    surfaceFormats.resize(amountOfFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &amountOfFormats, surfaceFormats.data());

    std::cout << std::endl;
    std::cout << "Amount of Formats: " << amountOfFormats << std::endl;
    for (auto &&i : surfaceFormats)
    {
        std::cout << "Formats: " << i.format << std::endl;
        std::cout << "Color Space: " << i.colorSpace << std::endl;
    }

    uint32_t amountOfPresentationModes = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentationModes, NULL);
    std::vector<VkPresentModeKHR> presentModes;
    presentModes.resize(amountOfPresentationModes);
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &amountOfPresentationModes, presentModes.data());

    std::cout << std::endl;
    std::cout << "Amount of Presentation Modes: " << amountOfPresentationModes << std::endl;
    for (auto &&i : presentModes)
    {
        std::cout << "Supported presentation mode: " << i << std::endl;
    }

    delete[] familyProperties;

    std::cout << std::endl;
}

std::vector<char> readFile(const std::string &&filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (!file)
    {
        throw std::runtime_error("Failed to open file");
    }
    size_t fileSize = (size_t)file.tellg();
    std::vector<char> fileBuffer(fileSize);
    file.seekg(0);
    file.read(fileBuffer.data(), fileSize);
    return fileBuffer;
}

void startGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Tutorial", NULL, NULL);
}

void createShaderModule(const std::vector<char> &code, VkShaderModule *shaderModule)
{
    VkShaderModuleCreateInfo shaderCreateInfo;
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.pNext = NULL;
    shaderCreateInfo.flags = 0;
    shaderCreateInfo.codeSize = code.size();
    shaderCreateInfo.pCode = (uint32_t *)code.data();

    VkResult result = vkCreateShaderModule(device, &shaderCreateInfo, NULL, shaderModule);
    ASSERT_VULKAN(result);
}

void startVulkan()
{
    //Create application info
    VkApplicationInfo appInfo;
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pNext = NULL;
    appInfo.pApplicationName = "Vulkan Tutorial";
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.pEngineName = "Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    //Get and print Layers
    uint32_t amountOfLayers = 0;
    vkEnumerateInstanceLayerProperties(&amountOfLayers, NULL);
    std::vector<VkLayerProperties> layers;
    layers.resize(amountOfLayers);
    vkEnumerateInstanceLayerProperties(&amountOfLayers, layers.data());

    std::cout << "Amount of layers: " << amountOfLayers << std::endl;
    for (int i = 0; i < amountOfLayers; i++)
    {
        std::cout << "Layer: " << i << std::endl;
        std::cout << "\tName:         " << layers[i].layerName << std::endl;
        std::cout << "\tSpec version: " << layers[i].specVersion << std::endl;
        std::cout << "\tImpl Version: " << layers[i].implementationVersion << std::endl;
        std::cout << "\tDescription:  " << layers[i].description << std::endl;
        std::cout << std::endl;
    }

    //Get and print Extensions
    uint32_t amountOfExtensions = 0;
    vkEnumerateInstanceExtensionProperties(NULL, &amountOfExtensions, NULL);
    std::vector<VkExtensionProperties> extensions;
    extensions.resize(amountOfExtensions);
    vkEnumerateInstanceExtensionProperties(NULL, &amountOfExtensions, extensions.data());

    std::cout << std::endl;
    std::cout << "Amount of Extensions: " << amountOfExtensions << std::endl;
    for (int i = 0; i < amountOfExtensions; i++)
    {
        std::cout << std::endl;
        std::cout << "Name: " << extensions[i].extensionName << std::endl;
        std::cout << "Spec Version: " << extensions[i].specVersion << std::endl;
    }
    std::cout << std::endl;

    const std::vector<const char *> validationLayers = {
        "VK_LAYER_LUNARG_standard_validation"};

    uint32_t amountOfGlfwExtensions = 0;
    auto glfwExtension = glfwGetRequiredInstanceExtensions(&amountOfGlfwExtensions);

    //Create instance info
    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = validationLayers.size();
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = amountOfGlfwExtensions;
    instanceInfo.ppEnabledExtensionNames = glfwExtension;

    //Create instance
    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    ASSERT_VULKAN(result);

    result = glfwCreateWindowSurface(instance, window, NULL, &surface);

    //Get amount of physical devices
    uint32_t amountOfPhysicalDevices = 0;
    result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, NULL);
    ASSERT_VULKAN(result);

    std::vector<VkPhysicalDevice> physicalDevice;
    physicalDevice.resize(amountOfPhysicalDevices);

    result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevice.data());

    ASSERT_VULKAN(result);

    //Print informations about the graphics card/driver
    for (int i = 0; i < amountOfPhysicalDevices; i++)
    {
        printStats(*physicalDevice.data());
    }

    float queuePrios[] = {1.0f, 1.0f, 1.0f, 1.0f};

    //Create device queue info
    VkDeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.pNext = NULL;
    deviceQueueCreateInfo.flags = 0;
    deviceQueueCreateInfo.queueFamilyIndex = 0; //TODO Choose correct family index
    deviceQueueCreateInfo.queueCount = 1;       //TODO Check if this amount is valid
    deviceQueueCreateInfo.pQueuePriorities = queuePrios;

    VkPhysicalDeviceFeatures usedFeatures = {};

    const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    //Create device info
    VkDeviceCreateInfo devicesCreateInfo;
    devicesCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devicesCreateInfo.pNext = NULL;
    devicesCreateInfo.flags = 0;
    devicesCreateInfo.queueCreateInfoCount = 1;
    devicesCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    devicesCreateInfo.enabledLayerCount = 0;
    devicesCreateInfo.ppEnabledLayerNames = NULL;
    devicesCreateInfo.enabledExtensionCount = deviceExtensions.size();
    devicesCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
    devicesCreateInfo.pEnabledFeatures = &usedFeatures;

    //Craete device
    //TODO pick "best device" instead of first device
    result = vkCreateDevice(physicalDevice[0], &devicesCreateInfo, NULL, &device);
    ASSERT_VULKAN(result);

    //Create a Queue
    VkQueue queue;
    vkGetDeviceQueue(device, 0, 0, &queue);

    VkBool32 surfaceSupport = false;
    result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice[0], 0, surface, &surfaceSupport);
    ASSERT_VULKAN(result);

    if (!surfaceSupport)
    {
        std::cerr << "Surface not supported!" << std::endl;
        assert(false);
    }

    //Create a Swapchain info
    VkSwapchainCreateInfoKHR swapchainCreateInfo;
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.pNext = NULL;
    swapchainCreateInfo.flags = 0;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.minImageCount = 2;                                   //TODO Check if valid
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;              //TODO civ
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; //TODO civ
    swapchainCreateInfo.imageExtent = {WIDTH, HEIGHT};
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; //TODO civ
    swapchainCreateInfo.queueFamilyIndexCount = 0;
    swapchainCreateInfo.pQueueFamilyIndices = NULL;
    swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; //TODO civ
    swapchainCreateInfo.clipped = VK_TRUE;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    //Creatinf the Swapchain
    result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &swapchain);
    ASSERT_VULKAN(result);

    vkGetSwapchainImagesKHR(device, swapchain, &amountOfImagesInSwapchain, NULL);
    std::vector<VkImage> swapchainImages;
    swapchainImages.resize(amountOfImagesInSwapchain);
    result = vkGetSwapchainImagesKHR(device, swapchain, &amountOfImagesInSwapchain, swapchainImages.data());
    ASSERT_VULKAN(result);

    //imageViews.resize(amountOfImagesInSwapchain);
    imageViews = new VkImageView[amountOfImagesInSwapchain];
    for (int i = 0; i < amountOfImagesInSwapchain; i++)
    {
        //Create image view info
        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = NULL;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_UNORM; //TODO civ
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &imageViewCreateInfo, NULL, &imageViews[i]);
        ASSERT_VULKAN(result);
    }

    auto shaderCodeVert = readFile("vert.spv");
    auto shaderCodeFrag = readFile("frag.spv");

    createShaderModule(shaderCodeVert, &shaderModuleVert);
    createShaderModule(shaderCodeFrag, &shaderModuleFrag);

    VkPipelineShaderStageCreateInfo shaderStageCreateInfoVert;
    shaderStageCreateInfoVert.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoVert.pNext = NULL;
    shaderStageCreateInfoVert.flags = 0;
    shaderStageCreateInfoVert.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageCreateInfoVert.module = shaderModuleVert;
    shaderStageCreateInfoVert.pName = "main";
    shaderStageCreateInfoVert.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo shaderStageCreateInfoFrag;
    shaderStageCreateInfoFrag.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfoFrag.pNext = NULL;
    shaderStageCreateInfoFrag.flags = 0;
    shaderStageCreateInfoFrag.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageCreateInfoFrag.module = shaderModuleFrag;
    shaderStageCreateInfoFrag.pName = "main";
    shaderStageCreateInfoFrag.pSpecializationInfo = NULL;

    VkPipelineShaderStageCreateInfo shaderStages[] = {shaderStageCreateInfoVert,
                                                      shaderStageCreateInfoFrag};
}

void startGameLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        //glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void shutdownVulkan()
{
    //Cleanup Vulkan
    vkDeviceWaitIdle(device);

    for (int i = 0; i < amountOfImagesInSwapchain; i++)
    {
        vkDestroyImageView(device, imageViews[i], NULL);
    }
    delete[] imageViews;
    vkDestroyShaderModule(device, shaderModuleVert, NULL);
    vkDestroyShaderModule(device, shaderModuleFrag, NULL);
    vkDestroySwapchainKHR(device, swapchain, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyInstance(instance, NULL);
}

void shutdownGLFW()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

int main()
{
    startGLFW();
    startVulkan();

    startGameLoop();

    shutdownVulkan();
    shutdownGLFW();

    return 0;
}