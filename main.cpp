//#include <vulkan/vulkan.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cassert>
#include <vector>
#include <fstream>
#include <limits>

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
std::vector<VkImageView> imageViews;
std::vector<VkFramebuffer> framebuffers;
GLFWwindow *window;
VkPipelineLayout pipelineLayout;
VkRenderPass renderPass;
VkPipeline pipeline;
VkCommandPool commandPool;
std::vector<VkCommandBuffer> commandBuffers;
VkSemaphore semaphoreImageAvailable;
VkSemaphore semaphoreRenderingDone;
VkQueue queue;

uint32_t amountOfImagesInSwapchain = 0;
const uint32_t WIDTH = 400, HEIGHT = 300;
const VkFormat ourFormat = VK_FORMAT_B8G8R8A8_SRGB;

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

void createInstance(){
    
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
    std::vector<VkLayerProperties> layers;
    uint32_t amountOfLayers = 0;
    vkEnumerateInstanceLayerProperties(&amountOfLayers, NULL);
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
        "VK_LAYER_KHRONOS_validation"};
    //VK_LAYER_LUNARG_standard_validation may needs to be relpaced with:
    //
    //VK_LAYER_LUNARG_standard_validation
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
    swapchainCreateInfo.imageFormat = ourFormat;                             //TODO civ
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

    imageViews.resize(amountOfImagesInSwapchain);
    for (int i = 0; i < amountOfImagesInSwapchain; i++)
    {
        //Create image view info
        VkImageViewCreateInfo imageViewCreateInfo;
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.pNext = NULL;
        imageViewCreateInfo.flags = 0;
        imageViewCreateInfo.image = swapchainImages[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = ourFormat; //TODO civ
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &imageViewCreateInfo, NULL, &imageViews.data()[i]);
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

    VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo;
    vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputCreateInfo.pNext = NULL;
    vertexInputCreateInfo.flags = 0;
    vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputCreateInfo.pVertexBindingDescriptions = NULL;
    vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputCreateInfo.pVertexAttributeDescriptions = NULL;

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo;
    inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyCreateInfo.pNext = NULL;
    inputAssemblyCreateInfo.flags = 0;
    inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

    //Create a viewport
    VkViewport viewport;
    viewport.x = 0.f;
    viewport.y = 0.f;
    viewport.width = WIDTH;
    viewport.height = HEIGHT;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 0.f;

    //Create a scissor
    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = {WIDTH, HEIGHT};

    //Create a viewport state with a viewport and scissor
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo;
    viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCreateInfo.pNext = NULL;
    viewportStateCreateInfo.flags = 0;
    viewportStateCreateInfo.viewportCount = 1;
    viewportStateCreateInfo.pViewports = &viewport;
    viewportStateCreateInfo.scissorCount = 1;
    viewportStateCreateInfo.pScissors = &scissor;

    //Create a Rasterizater state
    VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo;
    rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationCreateInfo.pNext = NULL;
    rasterizationCreateInfo.flags = 0;
    rasterizationCreateInfo.depthClampEnable = VK_FALSE;
    rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
    rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizationCreateInfo.depthBiasEnable = VK_FALSE;
    rasterizationCreateInfo.depthBiasConstantFactor = 0.f;
    rasterizationCreateInfo.depthBiasClamp = 0.f;
    rasterizationCreateInfo.depthBiasSlopeFactor = 0.f;
    rasterizationCreateInfo.lineWidth = 1.f;

    //Create a Multisampler state
    VkPipelineMultisampleStateCreateInfo multisampleCreateInfo;
    multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleCreateInfo.pNext = NULL;
    multisampleCreateInfo.flags = 0;
    multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
    multisampleCreateInfo.minSampleShading = 1.f;
    multisampleCreateInfo.pSampleMask = NULL;
    multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
    multisampleCreateInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo;
    colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendCreateInfo.pNext = NULL;
    colorBlendCreateInfo.flags = 0;
    colorBlendCreateInfo.logicOpEnable = VK_FALSE;
    colorBlendCreateInfo.logicOp = VK_LOGIC_OP_NO_OP;
    colorBlendCreateInfo.attachmentCount = 1;
    colorBlendCreateInfo.pAttachments = &colorBlendAttachment;
    colorBlendCreateInfo.blendConstants[0] = 0.f;
    colorBlendCreateInfo.blendConstants[1] = 0.f;
    colorBlendCreateInfo.blendConstants[2] = 0.f;
    colorBlendCreateInfo.blendConstants[3] = 0.f;

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo;
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.pNext = NULL;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.setLayoutCount = 0;
    pipelineLayoutCreateInfo.pSetLayouts = NULL;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = NULL;

    result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, NULL, &pipelineLayout);
    ASSERT_VULKAN(result);

    VkAttachmentDescription attachmentDescription;
    attachmentDescription.flags = 0;
    attachmentDescription.format = ourFormat;
    attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
    attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference attachmentReference;
    attachmentReference.attachment = 0;
    attachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescription;
    subpassDescription.flags = 0;
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.inputAttachmentCount = 0;
    subpassDescription.pInputAttachments = NULL;
    subpassDescription.colorAttachmentCount = 1;
    subpassDescription.pColorAttachments = &attachmentReference;
    subpassDescription.pResolveAttachments = NULL;
    subpassDescription.pDepthStencilAttachment = NULL;
    subpassDescription.preserveAttachmentCount = 0;
    subpassDescription.pPreserveAttachments = NULL;

    VkSubpassDependency subpassDependency;
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = 0;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDependency.dependencyFlags = 0;

    VkRenderPassCreateInfo renderPassCreateInfo;
    renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pNext = NULL;
    renderPassCreateInfo.flags = 0;
    renderPassCreateInfo.attachmentCount = 1;
    renderPassCreateInfo.pAttachments = &attachmentDescription;
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDescription;
    renderPassCreateInfo.dependencyCount = 1;
    renderPassCreateInfo.pDependencies = &subpassDependency;

    result = vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &renderPass);
    ASSERT_VULKAN(result);

    VkGraphicsPipelineCreateInfo pipelineCreateInfo;
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCreateInfo.pNext = NULL;
    pipelineCreateInfo.flags = 0;
    pipelineCreateInfo.stageCount = 2;
    pipelineCreateInfo.pStages = shaderStages;
    pipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
    pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
    pipelineCreateInfo.pTessellationState = NULL;
    pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
    pipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
    pipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
    pipelineCreateInfo.pDepthStencilState = NULL;
    pipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
    pipelineCreateInfo.pDynamicState = 0;
    pipelineCreateInfo.layout = pipelineLayout;
    pipelineCreateInfo.renderPass = renderPass;
    pipelineCreateInfo.subpass = 0;
    pipelineCreateInfo.basePipelineHandle = NULL;
    pipelineCreateInfo.basePipelineIndex = -1;

    result = vkCreateGraphicsPipelines(device, NULL, 1, &pipelineCreateInfo, NULL, &pipeline);
    ASSERT_VULKAN(result);

    for (size_t i = 0; i < amountOfImagesInSwapchain; i++)
    {
        VkFramebufferCreateInfo framebufferCreateInfo;
        framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferCreateInfo.pNext = NULL;
        framebufferCreateInfo.flags = 0;
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = 1;
        framebufferCreateInfo.pAttachments = &(imageViews[i]);
        framebufferCreateInfo.width = WIDTH;
        framebufferCreateInfo.height = HEIGHT;
        framebufferCreateInfo.layers = 1;

        framebuffers.resize(amountOfImagesInSwapchain);
        result = vkCreateFramebuffer(device, &framebufferCreateInfo, NULL, &(framebuffers.data()[i]));
        ASSERT_VULKAN(result);
    }

    VkCommandPoolCreateInfo commandPoolCreateInfo;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.pNext = NULL;
    commandPoolCreateInfo.flags = 0;
    commandPoolCreateInfo.queueFamilyIndex = 0;

    result = vkCreateCommandPool(device, &commandPoolCreateInfo, NULL, &commandPool);
    ASSERT_VULKAN(result);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo;
    commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocateInfo.pNext = NULL;
    commandBufferAllocateInfo.commandPool = commandPool;
    commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    commandBufferAllocateInfo.commandBufferCount = amountOfImagesInSwapchain;

    commandBuffers.resize(amountOfImagesInSwapchain);
    result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data());
    ASSERT_VULKAN(result);

    for (size_t i = 0; i < amountOfImagesInSwapchain; i++)
    {
        VkCommandBufferBeginInfo commandBufferBeginInfo;
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        commandBufferBeginInfo.pNext = NULL;
        commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        commandBufferBeginInfo.pInheritanceInfo = NULL;
        result = vkBeginCommandBuffer(commandBuffers[i], &commandBufferBeginInfo);
        ASSERT_VULKAN(result);

        VkRenderPassBeginInfo renderPassBeginInfo;
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.pNext = NULL;
        renderPassBeginInfo.renderPass = renderPass;
        renderPassBeginInfo.framebuffer = framebuffers[i];
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = {WIDTH, HEIGHT};
        VkClearValue clearValue = {0.f, 0.f, 0.f, 1.f};
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

        vkCmdDraw(commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(commandBuffers[i]);

        result = vkEndCommandBuffer(commandBuffers[i]);
        ASSERT_VULKAN(result);
    }

    VkSemaphoreCreateInfo semaphoreCreateInfo;
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.pNext = NULL;
    semaphoreCreateInfo.flags = 0;

    result = vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &semaphoreImageAvailable);
    ASSERT_VULKAN(result);
    result = vkCreateSemaphore(device, &semaphoreCreateInfo, NULL, &semaphoreRenderingDone);
    ASSERT_VULKAN(result);
}

void drawFrame()
{
    uint32_t imageIndex;
    vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), semaphoreImageAvailable, NULL, &imageIndex);

    VkSubmitInfo submitInfo;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = NULL;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &semaphoreImageAvailable;
    VkPipelineStageFlags waitStageMask[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
        //VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
    };
    submitInfo.pWaitDstStageMask = waitStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &semaphoreRenderingDone;

    VkResult result = vkQueueSubmit(queue, 1, &submitInfo, NULL);
    ASSERT_VULKAN(result);

    VkPresentInfoKHR presentInfo;
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = NULL;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &semaphoreRenderingDone;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = NULL;

    result = vkQueuePresentKHR(queue, &presentInfo);
    ASSERT_VULKAN(result);
    //vkQueueWaitIdle(queue);
}

void startGameLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        double start = glfwGetTime();
        //glfwSwapBuffers(window);
        glfwPollEvents();
        drawFrame();
        double end = glfwGetTime();
        std::cout << "FPS: " << 1 / (end - start) << std::endl;
    }
}

void shutdownVulkan()
{
    //Cleanup Vulkan
    vkDeviceWaitIdle(device);

    vkDestroySemaphore(device, semaphoreImageAvailable, NULL);
    vkDestroySemaphore(device, semaphoreRenderingDone, NULL);
    vkFreeCommandBuffers(device, commandPool, amountOfImagesInSwapchain, commandBuffers.data());
    vkDestroyCommandPool(device, commandPool, NULL);
    for (size_t i = 0; i < amountOfImagesInSwapchain; i++)
    {
        vkDestroyFramebuffer(device, framebuffers.data()[i], NULL);
    }

    vkDestroyPipeline(device, pipeline, NULL);
    vkDestroyRenderPass(device, renderPass, NULL);
    for (int i = 0; i < amountOfImagesInSwapchain; i++)
    {
        vkDestroyImageView(device, imageViews.data()[i], NULL);
    }
    vkDestroyPipelineLayout(device, pipelineLayout, NULL);
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