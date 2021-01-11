//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>
#include <vector>

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
GLFWwindow *window;

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

    delete[] familyProperties;

    std::cout << std::endl;
}

void startGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(400, 300, "Vulkan Tutorial", NULL, NULL);
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

    //Create device info
    VkDeviceCreateInfo devicesCreateInfo;
    devicesCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    devicesCreateInfo.pNext = NULL;
    devicesCreateInfo.flags = 0;
    devicesCreateInfo.queueCreateInfoCount = 1;
    devicesCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    devicesCreateInfo.enabledLayerCount = 0;
    devicesCreateInfo.ppEnabledLayerNames = NULL;
    devicesCreateInfo.enabledExtensionCount = 0;
    devicesCreateInfo.ppEnabledExtensionNames = NULL;
    devicesCreateInfo.pEnabledFeatures = &usedFeatures;

    //Craete device
    //TODO pick "best device" instead of first device
    result = vkCreateDevice(physicalDevice[0], &devicesCreateInfo, NULL, &device);
    ASSERT_VULKAN(result);

    //Create a Queue
    VkQueue queue;
    vkGetDeviceQueue(device, 0, 0, &queue);
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