#include<vulkan/vulkan.h>
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[]) {
	// Application's informations :
	VkApplicationInfo app_info = {0};
	// Informations on instance that we want to create :
	VkInstanceCreateInfo instance_info = {0};
	// Vulkan instance :
	VkInstance instance = {0};

	// Extentions that vulkan must use to interact with the window manager
	const char *const enabled_extension_names[2] = { // Seen in glfw/src/x11_window.c line 2418
		"VK_KHR_surface",
		"VK_KHR_xlib_surface"
	};

	// Application's informations filling :
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = 0;
	app_info.pApplicationName = "vulkan test";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	app_info.pEngineName = "None";
	app_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	// Instance informations filling :
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pNext = 0;
	instance_info.flags = 0;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = 0;
	instance_info.ppEnabledLayerNames = 0;
	// Informations on layers dans extentions here : https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/blob/master/loader/LoaderAndLayerInterface.md
	instance_info.enabledExtensionCount = 2; // Seen in glfw/src/vulkan.c line 243
	instance_info.ppEnabledExtensionNames = enabled_extension_names;

	// Vulkan instance creation
	if(vkCreateInstance(&instance_info, 0, &instance) != VK_SUCCESS) {
		printf("Failed to create vulkan instance\n");
	    return -1;
	}

	// Vulkan instance destruction
	vkDestroyInstance(instance, 0);

    return 0;
}

