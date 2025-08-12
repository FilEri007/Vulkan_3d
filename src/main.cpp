#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>

#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>

#include "window.hpp"

const std::vector<const char*> validation_layers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool enable_validation_layers = false;
#else
    const bool enable_validation_layers = true;
#endif

VkResult create_debug_utils_messenger_ext(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *p_create_info, const VkAllocationCallbacks *p_allocator, VkDebugUtilsMessengerEXT *p_debug_messenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, p_create_info, p_allocator, p_debug_messenger);
    } else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void destroy_debug_utils_messenger_ext(VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *p_allocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debug_messenger, p_allocator);
    }
}

class App
{
public:
    void run()
    {
        init();
        init_vulkan();
        main_loop();
        cleanup();
    }

private:
    bool game_running = true;
    
    // SDL3:
    SDL_Event event;
    Window window = Window("Vulkan", 800, 600, SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

    // Vulkan:
    VkInstance instance;
    std::vector<const char*> required_extensions;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
    VkPhysicalDevice physical_device = VK_NULL_HANDLE;

    void init(void)
    {
        if (SDL_Init(SDL_INIT_VIDEO) != true)
        {
            throw std::runtime_error(SDL_GetError());
        }
    }

    void init_vulkan(void)
    {
        create_instance();
        pick_physical_device();
    }

    void pick_physical_device(void)
    {
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
        if (device_count == 0)
        {
            throw std::runtime_error("Failed to find a GPU with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, devices.data());

        for (const auto &device : devices)
        {
            if (is_device_suitable(device))
            {
                physical_device = device;
                break;
            }
        }

        if (physical_device == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Failed to find a suitable GPU!");
        }
    }

    bool is_device_suitable(VkPhysicalDevice device)
    {
        VkPhysicalDeviceProperties device_properties;
        VkPhysicalDeviceFeatures device_features;
        vkGetPhysicalDeviceProperties(device, &device_properties);
        vkGetPhysicalDeviceFeatures(device, &device_features);

        return true;
    }

    void create_instance(void)
    {
        if (enable_validation_layers && !check_validation_layer_support())
        {
            throw std::runtime_error("Validation layers requested, but not available!");
        }

        VkApplicationInfo application_info{};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = "Hello Triangle";
        application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.pEngineName = "No Engine";
        application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &application_info;

        auto extensions = get_required_extensions();
        create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        create_info.ppEnabledExtensionNames = extensions.data();

        Uint32 extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, available_extensions.data());

        std::cout << "Available extensions: \n";

        for (const auto &extension : available_extensions)
        {
            std::cout << "\t" << extension.extensionName << "\n";
        }
        
        VkDebugUtilsMessengerCreateInfoEXT debug_create_info{};
        if (enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<uint32_t>(validation_layers.size());
            create_info.ppEnabledLayerNames = validation_layers.data();

            populate_debug_messenger_create_info(debug_create_info);
            create_info.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debug_create_info;
        }
        else
        {
            create_info.enabledLayerCount = 0;

            create_info.pNext = nullptr;
        }

        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create a Vulkan Instance!");
        }

        if (create_debug_utils_messenger_ext(instance, &debug_create_info, nullptr, &debug_messenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }
    

    bool check_validation_layer_support(void)
    {
        uint32_t layer_count = 0;
        vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

        std::vector<VkLayerProperties> available_layers(layer_count);
        vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

        for (const char *layer_name : validation_layers)
        {
            bool layer_found = false;

            for (const auto &layer_properties : available_layers)
            {
                if (strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
            {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> get_required_extensions(void)
    {
        uint32_t sdl_extension_count = 0;
        const char * const * sdl_extensions_const;
        sdl_extensions_const = SDL_Vulkan_GetInstanceExtensions(&sdl_extension_count);
        const char **sdl_extensions = const_cast<const char**>(sdl_extensions_const);

        std::vector<const char*> extensions(sdl_extensions, sdl_extensions + sdl_extension_count);

        if (enable_validation_layers)
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT *p_callback_data,
        void *p_user_data
    )
    {
        std::cerr << "Validation layer: " << p_callback_data->pMessage << std::endl;

        return VK_FALSE;
    }

    void setup_debug_messenger(void)
    {
        if (!enable_validation_layers) return;

        VkDebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);
        if (create_debug_utils_messenger_ext(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to set up debug messenger!");
        }
    }
    
    void populate_debug_messenger_create_info(VkDebugUtilsMessengerCreateInfoEXT &create_info)
    {
        create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
    }

    void main_loop(void)
    {
        while (game_running)
        {
            while (SDL_PollEvent(&event))
            {
                switch (event.type)
                {
                    case SDL_EVENT_QUIT:
                        game_running = false;
                }
            }
        }
    }

    void cleanup(void)
    {
        if (enable_validation_layers)
        {
            destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
        }
        vkDestroyInstance(instance, nullptr);
        SDL_Quit();
    }
};

int main(void)
{
    App app;

    if (enable_validation_layers)
    {
        std::cout << "Debug mode!\n" << std::endl;
    }

    try
    {
        app.run();
    } catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
