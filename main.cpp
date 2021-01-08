#include <vulkan/vulkan.h>
#include <iostream>

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

//Print some stats about the graphics card
void printStats(const VkPhysicalDevice &device)
{
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    uint32_t apiVer = properties.apiVersion;

    std::cout << "Name:             " << properties.deviceName << std::endl
              << "API Version:      " << VK_VERSION_MAJOR(apiVer) << '.' << VK_VERSION_MINOR(apiVer) << '.' << VK_VERSION_PATCH(apiVer) << std::endl
              << "Driver Version:   " << properties.driverVersion << std::endl
              << "Vendor ID:        " << properties.vendorID << std::endl
              << "Device ID:        " << properties.deviceID << std::endl
              << "Device Type:      " << properties.deviceType << std::endl;

    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);
    std::cout << "Geometry Shader:  " << features.geometryShader << std::endl;

    VkPhysicalDeviceMemoryProperties memProp;
    vkGetPhysicalDeviceMemoryProperties(device, &memProp);

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

    //Create instance info
    VkInstanceCreateInfo instanceInfo;
    instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceInfo.pNext = NULL;
    instanceInfo.flags = 0;
    instanceInfo.pApplicationInfo = &appInfo;
    instanceInfo.enabledLayerCount = 0;
    instanceInfo.ppEnabledLayerNames = NULL;
    instanceInfo.enabledExtensionCount = 0;
    instanceInfo.ppEnabledExtensionNames = NULL;

    //Create instance
    VkResult result = vkCreateInstance(&instanceInfo, NULL, &instance);
    ASSERT_VULKAN(result);

    //Get amount of physical devices
    uint32_t amountOfPhysicalDevices = 0;
    result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, NULL);
    ASSERT_VULKAN(result);

    VkPhysicalDevice *physicalDevice = new VkPhysicalDevice[amountOfPhysicalDevices];
    result = vkEnumeratePhysicalDevices(instance, &amountOfPhysicalDevices, physicalDevice);

    ASSERT_VULKAN(result);

    for (int i = 0; i < amountOfPhysicalDevices; i++)
    {
        printStats(*physicalDevice);
    }

    return 0;
}