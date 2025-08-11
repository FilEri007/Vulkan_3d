#include <vulkan/vulkan.h>
#include <iostream>

int main() {
    uint32_t version = 0;

    VkResult result = vkEnumerateInstanceVersion(&version);
    if (result != VK_SUCCESS) {
        std::cerr << "Failed to query Vulkan instance version!" << std::endl;
        return 1;
    }

    std::cout << "Vulkan API Version: "
              << VK_VERSION_MAJOR(version) << "."
              << VK_VERSION_MINOR(version) << "."
              << VK_VERSION_PATCH(version) << std::endl;

    return 0;
}