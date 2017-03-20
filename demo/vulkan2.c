#include<vulkan/vulkan.h>
#include<stdlib.h>
#include<stdio.h>

int main(int argc, char *argv[]) {
	uint32_t idx;

	// Informations sur l'application :
	VkApplicationInfo app_info = {0};
	// Informations sur l'instance qu'on veut creer :
	VkInstanceCreateInfo instance_info = {0};
	// Instance vulkan :
	VkInstance instance = {0};

	// Extentions que vulkan doit utiliser pour s'integrer au window manager
	const char *const enabled_extension_names[2] = { // Vu dans glfw/src/x11_window.c ligne 2418
		"VK_KHR_surface",
		"VK_KHR_xlib_surface"
	};

	uint32_t physical_device_count = 0;
	VkPhysicalDevice *physical_device = 0;
	VkPhysicalDevice selected_physical_device = {0};
	VkPhysicalDeviceProperties physical_device_proprieties = {0};

	uint32_t queue_family_property_count = 0;
	VkQueueFamilyProperties* queue_family_properties;
	VkDeviceQueueCreateInfo queue_create_info = {0};
	float queue_priority = 1;
	VkPhysicalDeviceFeatures device_features;
	VkDeviceCreateInfo device_info = {0};
	VkDevice device = 0;

	// Informations sur l'application :
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pNext = 0;
	app_info.pApplicationName = "OSPOOC vulkan demo";
	app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 0);
	app_info.pEngineName = "OSPOOC";
	app_info.engineVersion = VK_MAKE_VERSION(0, 0, 0);
	app_info.apiVersion = VK_API_VERSION_1_0;

	// Informations sur l'instance qu'on veut creer :
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pNext = 0;
	instance_info.flags = 0;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledLayerCount = 0;
	instance_info.ppEnabledLayerNames = 0;
	// Informations sur les layers et les extentions ici : https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/blob/master/loader/LoaderAndLayerInterface.md
	instance_info.enabledExtensionCount = 2; // Vu dans glfw/src/vulkan.c ligne 243
	instance_info.ppEnabledExtensionNames = enabled_extension_names;

	// Creation de l'instance vulkan
	if(vkCreateInstance(&instance_info, 0, &instance) != VK_SUCCESS) {
		printf("Failed to create vulkan instance\n");
	    return -1;
	}

	if(vkEnumeratePhysicalDevices(instance, &physical_device_count, 0) != VK_SUCCESS) {
		vkDestroyInstance(instance, 0);
		printf("Unable to get number of physical devices\n");
	    return -1;
	}

	// Recuperation d'un handler de la carte graphique
	if(!physical_device_count) {
		vkDestroyInstance(instance, 0);
		printf("No compatible physical vulkan device present\n");
	    return -1;
	}

	if(!(physical_device = malloc(physical_device_count * sizeof(VkPhysicalDevice)))) {
		vkDestroyInstance(instance, 0);
		printf("Unable to allocate list of physical devices\n");
	    return -1;
	}

	if(vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_device) != VK_SUCCESS) {
		vkDestroyInstance(instance, 0);
		printf("Unable to get list of physical devices\n");
	    return -1;
	}

	selected_physical_device = physical_device[0]; // VkPhysicalDevice est un handler de device sous forme de pointeur
	free(physical_device);

	// Recuperation des infos de la carte graphique
	vkGetPhysicalDeviceProperties(selected_physical_device, &physical_device_proprieties);
	// Recuperation des infos des files d'instructions
	vkGetPhysicalDeviceQueueFamilyProperties(selected_physical_device, &queue_family_property_count, 0);

	if(!queue_family_property_count) {
		vkDestroyInstance(instance, 0);
		printf("Unable to get any queue family property for device %s\n", physical_device_proprieties.deviceName);
	    return -1;
	}

	if(!(queue_family_properties = malloc(queue_family_property_count * sizeof(VkQueueFamilyProperties)))) {
		vkDestroyInstance(instance, 0);
		printf("Unable to allocate list of queue family property\n");
	    return -1;
	}

	vkGetPhysicalDeviceQueueFamilyProperties(selected_physical_device, &queue_family_property_count, queue_family_properties);

	// Choix du premier groupe de files d'instructions compatible avec le traitement graphique
	queue_create_info.queueCount = 0;
	for(idx = 0; idx < queue_family_property_count; idx++) {
		int pipe = 0;

		printf("queue_family_properties[%d].queueCount = %d, queue_family_properties[%d].queueFlags = ",
			idx, queue_family_properties[idx].queueCount, idx);
		if(queue_family_properties[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			printf("VK_QUEUE_GRAPHICS_BIT");
			pipe++;
		}
		if(queue_family_properties[idx].queueFlags & VK_QUEUE_COMPUTE_BIT) {
			printf("%sVK_QUEUE_COMPUTE_BIT", pipe++ ? " | " : "");
		}
		if(queue_family_properties[idx].queueFlags & VK_QUEUE_TRANSFER_BIT) {
			printf("%sVK_QUEUE_TRANSFER_BIT", pipe++ ? " | " : "");
		}
		if(queue_family_properties[idx].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) {
			printf("%sVK_QUEUE_SPARSE_BINDING_BIT", pipe++ ? " | " : "");
		}

		printf("%s\n", pipe ? "" : "0");

		if((queue_family_properties[idx].queueCount) && (!queue_create_info.queueCount) &&
			(queue_family_properties[idx].queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
			queue_create_info.queueCount = queue_family_properties[idx].queueCount;
			queue_create_info.queueFamilyIndex = idx;
		}
	}

	free(queue_family_properties);

	if(!queue_create_info.queueCount) {
		vkDestroyInstance(instance, 0);
		printf("Unable find any graphics queue\n");
	    return -1;
	}

	queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_info.pNext = 0;
	queue_create_info.flags = 0;
	queue_create_info.pQueuePriorities = &queue_priority; // Defini la priorite de la file d'instruction entre 0.0 et 1.0

	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.pNext = 0;
	device_info.flags = 0;
	device_info.queueCreateInfoCount = 1;
	device_info.pQueueCreateInfos = &queue_create_info;
	device_info.enabledLayerCount = 0;
	device_info.ppEnabledLayerNames = 0;
	device_info.enabledExtensionCount = 0;
	device_info.ppEnabledExtensionNames = 0;
	device_info.pEnabledFeatures = &device_features;

	// Creation du device logique
	if((idx = vkCreateDevice(selected_physical_device, &device_info, 0, &device)) != VK_SUCCESS) {
		vkDestroyInstance(instance, 0);
		printf("Unable to create logical device, idx = %d\n", idx);
	    return -1;
	}

    printf("On a trouve le device %s\n", physical_device_proprieties.deviceName);

	vkDestroyDevice(device, 0);
	vkDestroyInstance(instance, 0);

    return 0;
}

