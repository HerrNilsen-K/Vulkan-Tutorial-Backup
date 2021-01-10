#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>

#define ASSERT_VULKAN(val)                                         \
    if (val != VK_SUCCESS)                                         \
    {                                                              \
        std::cout << "---------------------------------------\n";  \
        std::cout << "ERROR: 'RESULT != VK_SUCCESS'" << std::endl; \
        std::cout << __FILE__ << ": " << __LINE__ << std::endl;    \
        std::cout << "---------------------------------------\n";  \
        return -1;                                                 \
    }

VkInstance instance;
VkDevice device;

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

    delete[] familyProperties;

    std::cout << std::endl;
}

int main()
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

    //Create instance info
    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = validationLayers.size();
    instanceInfo.ppEnabledLayerNames = validationLayers.data();
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.ppEnabledExtensionNames = NULL;

    //Create instance
    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    ASSERT_VULKAN(result);

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
    deviceQueueCreateInfo.queueCount = 4;       //TODO Check if this amount is valid
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

    //Cleanup Vulkan
    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);

    return 0;
}