#include"OSPlib.h"

#define OSP_MAX_QUE_FAM 16
#define OSP_VK_QUEUE_DEFAULT_BITS \
		VK_QUEUE_GRAPHICS_BIT | \
		VK_QUEUE_COMPUTE_BIT | \
		VK_QUEUE_TRANSFER_BIT | \
		VK_QUEUE_SPARSE_BINDING_BIT

/* Extentions that vulkan must use to interact with the window manager */
static const char *const enabled_extension_names[2] = { // Seen in glfw/src/x11_window.c line 2418
	"VK_KHR_surface",
	"VK_KHR_xlib_surface"
};

void OSPVlkHdl(OSPobj **obj) {
	OSPvulkan *vlk = (OSPvulkan *) obj[0];

	/* Vulkan instance is about to be destroyed */
	if(!obj[0]->_buf) {
		while(vlk->_slv) {
			OSPFre(&vlk->_slv->_obj);
		}

		while(vlk->_dev_cnt--) {
			if(vlk->_dev[vlk->_dev_cnt]._que_blk) free(vlk->_dev[vlk->_dev_cnt]._que_blk);
		}

		if(vlk->_dev) free(vlk->_dev);
		if(vlk->_instance) vkDestroyInstance(vlk->_instance, 0);

		return;
	}
}

OSPctr *OSPVlkCtr() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	ret = OSPCtr(0, 0, sizeof(OSPvulkan), OSPVlkHdl);

	/* ret->_fct[OSPVLK_EXAMPLE] = OSPvlk_example; */

	return ret;
}

OSPvulkan *OSPVlk(const char *appname, uint16_t appversionmajor, uint16_t appversionminor, uint16_t appversionpatch,
			const char *enginename, uint16_t engineversionmajor, uint16_t engineversionminor, uint16_t engineversionpatch) {
	OSPvulkan *ret = (OSPvulkan *) OSPAdd(OSPVlkCtr());

	uint32_t devidx;
	uint32_t queidx;

	VkPhysicalDevice *physical_device = 0;
	VkQueueFamilyProperties *queue_family_properties = 0;
/*
	uint32_t phys_dev_cnt = 0;
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
*/
	ret->_instance = 0;
	ret->_dev_cnt = 0;
	ret->_dev = 0;

	// Application's informations filling :
	ret->_app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	ret->_app_info.pNext = 0;
	ret->_app_info.pApplicationName = appname; /* TODO strdup */
	ret->_app_info.applicationVersion = VK_MAKE_VERSION(appversionmajor, appversionminor, appversionpatch);
	ret->_app_info.pEngineName = enginename; /* TODO strdup */
	ret->_app_info.engineVersion = VK_MAKE_VERSION(engineversionmajor, engineversionminor, engineversionpatch);
	ret->_app_info.apiVersion = VK_API_VERSION_1_0;

	// Instance informations filling :
	ret->_inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	ret->_inst_info.pNext = 0;
	ret->_inst_info.flags = 0;
	ret->_inst_info.pApplicationInfo = &ret->_app_info;
	ret->_inst_info.enabledLayerCount = 0;
	ret->_inst_info.ppEnabledLayerNames = 0;
	// Informations on layers and extentions here : https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/blob/master/loader/LoaderAndLayerInterface.md
	ret->_inst_info.enabledExtensionCount = 2; // Vu dans glfw/src/vulkan.c ligne 243
	ret->_inst_info.ppEnabledExtensionNames = enabled_extension_names;

	// Vulkan instance creation
	if(vkCreateInstance(&ret->_inst_info, 0, &ret->_instance) != VK_SUCCESS) {
		OSPrint(0, "OSPVlk : Failed to create vulkan instance");
		OSPFre(&ret->_obj);
	    return 0;
	}

	// Physical devices enumeration
	if(vkEnumeratePhysicalDevices(ret->_instance, &ret->_dev_cnt, 0) != VK_SUCCESS) {
		OSPrint(0, "OSPVlk : Unable to get number of physical devices");
		OSPFre(&ret->_obj);
	    return 0;
	}

	if(!ret->_dev_cnt) {
		OSPrint(0, "OSPVlk : No compatible vulkan physical vulkan device present");
		OSPFre(&ret->_obj);
	    return 0;
	}

	if(!(ret->_dev = calloc(ret->_dev_cnt, sizeof(ret->_dev[0])))) {
		OSPrint(0, "OSPVlk : Unable to allocate array of physical devices");
		ret->_dev_cnt = 0;
		OSPFre(&ret->_obj);
	    return 0;
	}

	if(vkEnumeratePhysicalDevices(ret->_instance, &ret->_dev_cnt, (VkPhysicalDevice *) ret->_dev) != VK_SUCCESS) {
		OSPrint(0, "OSPVlk : Unable to get list of physical devices");
		OSPFre(&ret->_obj);
	    return 0;
	}

	devidx = ret->_dev_cnt;
	physical_device = (VkPhysicalDevice *) ret->_dev;
	while(devidx--) {
		ret->_dev[devidx]._phys_dev = physical_device[devidx];
		vkGetPhysicalDeviceProperties(ret->_dev[devidx]._phys_dev, &ret->_dev[devidx]._phys_dev_prop);

		/* This is done by calloc
		ret->_dev[devidx]._que_blk_cnt = 0;
		ret->_dev[devidx]._que_blk = 0; */

		/* Queue block enumeration for each physical devices */
		vkGetPhysicalDeviceQueueFamilyProperties(ret->_dev[devidx]._phys_dev, &ret->_dev[devidx]._que_blk_cnt, 0);

		if(!ret->_dev[devidx]._que_blk_cnt) {
			OSPrint(0, "OSPVlk : No queue available for vulkan physical device %s",
				ret->_dev[devidx]._phys_dev_prop.deviceName);
			continue;
		}

		if(!(ret->_dev[devidx]._que_blk = calloc(ret->_dev[devidx]._que_blk_cnt, sizeof(ret->_dev[devidx]._que_blk[0])))) {
			OSPrint(0, "OSPVlk : Unable to allocate array of queue block for vulkan physical device %s",
				ret->_dev[devidx]._phys_dev_prop.deviceName);
			continue;
		}

		vkGetPhysicalDeviceQueueFamilyProperties(ret->_dev[devidx]._phys_dev,
			&ret->_dev[devidx]._que_blk_cnt, (VkQueueFamilyProperties *) ret->_dev[devidx]._que_blk);

		queidx = ret->_dev[devidx]._que_blk_cnt;
		queue_family_properties = (VkQueueFamilyProperties *) ret->_dev[devidx]._que_blk;

		while(queidx--) {
			memmove(&ret->_dev[devidx]._que_blk[queidx]._que_fam_prop, &queue_family_properties[queidx],
				sizeof(VkQueueFamilyProperties));
			ret->_dev[devidx]._que_blk[queidx]._que_abl =
				ret->_dev[devidx]._que_blk[queidx]._que_fam_prop.queueCount;
		}
	}

	return ret;
}

uint8_t OSPgetVulkanQueueIndexFromQueueFamily(VkQueueFlags quefam) {
	uint8_t ret = quefam & VK_QUEUE_GRAPHICS_BIT ? 0x08 : 0x00;
	ret |= quefam & VK_QUEUE_COMPUTE_BIT ? 0x04 : 0x00;
	ret |= quefam & VK_QUEUE_TRANSFER_BIT ? 0x02 : 0x00;
	return ret | (quefam & VK_QUEUE_SPARSE_BINDING_BIT) ? 0x01 : 0x00;
}

/* Returns a staticaly allocated list of Vulkan queue queue block available for the device no dev */
uint32_t *OSPgetVulkanAvailableQueueArrayFromPhysicalDeviceID(OSPvulkan *vlk, uint32_t dev) {
	static uint32_t QueueArray[OSP_MAX_QUE_FAM];
	uint32_t queidx;

	for(queidx = 0; queidx < OSP_MAX_QUE_FAM; queidx++) {
		if(queidx) QueueArray[queidx] = 0;
		else QueueArray[queidx] = dev;
	}

	for(queidx = 0; queidx < vlk->_dev[dev]._que_blk_cnt; queidx++) {
		uint32_t queabl = vlk->_dev[dev]._que_blk[queidx]._que_abl;
		VkQueueFlags quefam = vlk->_dev[dev]._que_blk[queidx]._que_fam_prop.queueFlags;
		QueueArray[OSPgetVulkanQueueIndexFromQueueFamily(quefam)] = queabl;
	}

	return QueueArray[0] == dev ? QueueArray : 0;
}

char *OSPgetVulkanPhysicalDeviceNameFromID(OSPvulkan *vlk, uint32_t dev) {
	return vlk->_dev[dev]._phys_dev_prop.deviceName;
}

uint32_t OSPgetVulkanPhysicalDeviceCount(OSPvulkan *vlk) {
	return vlk->_dev_cnt;
}

void OSPVulkanInstanceState(OSPvulkan *vlk) {
	uint32_t devidx;
	uint32_t queidx;
	uint32_t quenbr;
	VkQueueFlags quefam;

	OSPrint(1, "OSPrintVulkanInstanceState : Vulkan instance for application %s", vlk->_app_info.pApplicationName);
	OSPrint(1, "OSPrintVulkanInstanceState :     Queue block letters meaning :");
	OSPrint(1, "OSPrintVulkanInstanceState :         G : GRAPHICS");
	OSPrint(1, "OSPrintVulkanInstanceState :         C : COMPUTE");
	OSPrint(1, "OSPrintVulkanInstanceState :         T : TRANSFER");
	OSPrint(1, "OSPrintVulkanInstanceState :         B : SPARSE_BINDING");
	OSPrint(1, "OSPrintVulkanInstanceState :         Uppercase : Available propriety");
	OSPrint(1, "OSPrintVulkanInstanceState :         Lowercase : Absent propriety");

	for(devidx = 0; devidx < OSPgetVulkanPhysicalDeviceCount(vlk); devidx++) {
		OSPrint(1, "OSPrintVulkanInstanceState :     On device no %d named %s :", devidx, OSPgetVulkanPhysicalDeviceNameFromID(vlk, devidx));

		for(queidx = 0; queidx < vlk->_dev[devidx]._que_blk_cnt; queidx++) {
			quenbr = vlk->_dev[devidx]._que_blk[queidx]._que_fam_prop.queueCount;
			quefam = vlk->_dev[devidx]._que_blk[queidx]._que_fam_prop.queueFlags;
			OSPrint(1, "OSPrintVulkanInstanceState :         On queue block no %d with attributes %c%c%c%c", queidx,
				quefam & VK_QUEUE_GRAPHICS_BIT ? 'G' : 'g',
				quefam & VK_QUEUE_COMPUTE_BIT ? 'C' : 'c',
				quefam & VK_QUEUE_TRANSFER_BIT ? 'T' : 't',
				quefam & VK_QUEUE_SPARSE_BINDING_BIT ? 'B' : 'b');

			while(quenbr--) {
				OSPrint(1, "OSPrintVulkanInstanceState :             Queue %s",
					((quenbr + 1) > vlk->_dev[devidx]._que_blk[queidx]._que_abl) ? "In use" : "Available");
			}
		}
	}
}

uint32_t OSPgetVulkanPhysicalDeviceQueuePropertyCount(OSPvulkan *vlk, uint32_t dev) {
	return vlk->_dev[dev]._que_blk_cnt;
}

VkDeviceCreateInfo *OSPVulkanAllocateDeviceInfos(OSPvulkan *vlk, uint32_t dev, ...) {
	size_t alloc_size = sizeof(VkDeviceCreateInfo);
	VkDeviceCreateInfo *ret;
	VkDeviceQueueCreateInfo *queues;
	float *priors;
	uint32_t nbr = 0;
	uint32_t x;
	va_list arg;

	va_start(arg, dev);
	while(va_arg(arg, unsigned int)) {
		va_arg(arg, unsigned int);
		va_arg(arg, double);
		nbr++;
		alloc_size += sizeof(VkDeviceQueueCreateInfo) + sizeof(float);
	}
	va_end(arg);

	if(!nbr) return 0;

	ret = malloc(sizeof(VkDeviceCreateInfo) + 
		(nbr * (sizeof(VkDeviceQueueCreateInfo) + sizeof(float))));
	queues = (VkDeviceQueueCreateInfo *) &ret[1];
	priors = (float *) &queues[nbr];

	ret->sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	ret->pNext = 0;
	ret->flags = 0;
	ret->queueCreateInfoCount = nbr;
	ret->pQueueCreateInfos = queues;
	ret->enabledLayerCount = 0;
	ret->ppEnabledLayerNames = 0;
	ret->enabledExtensionCount = 0;
	ret->ppEnabledExtensionNames = 0;
	ret->pEnabledFeatures = 0;

	va_start(arg, dev);
	for(x = 0; x < nbr; x++) {
		uint32_t y;
		uint32_t max_prop = OSPgetVulkanPhysicalDeviceQueuePropertyCount(vlk, dev);
		uint8_t req_flags;
		queues[x].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queues[x].pNext = ((x + 1) < nbr) ? &queues[x + 1] : 0;
		queues[x].flags = 0;
		queues[x].queueCount = va_arg(arg, unsigned int);
		req_flags = va_arg(arg, unsigned int);
		priors[x] = va_arg(arg, double);
		queues[x].pQueuePriorities = &priors[x];

		for(y = 0; y < max_prop; y++) {
			VkQueueFlags quefam = vlk->_dev[dev]._que_blk[y]._que_fam_prop.queueFlags;
			uint8_t get_flags = OSPgetVulkanQueueIndexFromQueueFamily(quefam);

			if(req_flags != get_flags) continue;
			if(queues[x].queueCount > vlk->_dev[dev]._que_blk[y]._que_abl) {
				free(ret);
				return 0;
			}

			queues[x].queueFamilyIndex = y;
			break;
		}
	}
	va_end(arg);

	return ret;
}

void OSPGpdHdl(OSPobj **obj) {
	OSPgpdev *gpd = (OSPgpdev *) obj[0];

	/* Vulkan device is about to be destroyed */
	if(!obj[0]->_buf) {
		while(gpd->_slv) free(gpd->_slv);

		if(gpd->_dev_crt_info) {
			uint32_t x = gpd->_dev_crt_info->queueCreateInfoCount;

			while(x--) {
				uint32_t famidx = gpd->_dev_crt_info->pQueueCreateInfos[x].queueFamilyIndex;
				uint32_t famcnt = gpd->_dev_crt_info->pQueueCreateInfos[x].queueCount;
				gpd->_mtr->_dev[gpd->_idx]._que_blk[famidx]._que_abl += famcnt;
			}

			free(gpd->_dev_crt_info);
			vkDestroyDevice(gpd->_dev, 0);
		}

		if(gpd->_mtr) {
			if(gpd->_mtr->_slv == gpd) {
				if(gpd->_nxt) gpd->_mtr->_slv = gpd->_nxt;
				else gpd->_mtr->_slv = gpd->_prv;
			}

			if(gpd->_nxt) gpd->_nxt->_prv = gpd->_prv;
			if(gpd->_prv) gpd->_prv->_nxt = gpd->_nxt;
		}
	}
}

OSPctr *OSPGpdCtr() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	ret = OSPCtr(0, 0, sizeof(OSPgpdev), OSPGpdHdl);

	/* ret->_fct[OSPGPD_EXAMPLE] = OSPgpd_example; */

	return ret;
}

OSPgpdev *OSPGpd(OSPvulkan *vlk, uint32_t dev, VkDeviceCreateInfo *infos) {
	OSPgpdev *ret;
	uint32_t x;
	uint64_t que_cnt = 0;
	uint8_t novlk = 0;

	if(vlk && (dev >= vlk->_dev_cnt)) {
		if(novlk) OSPFre(&vlk->_obj);
		return 0;
	}
	else {
		vlk = OSPVlk(0, 0, 0, 0, 0, 0, 0, 0);
		novlk = 1;
		dev = -1;

		while(!infos && (++dev < vlk->_dev_cnt)) {
			infos = OSPVulkanAllocateDeviceInfos(vlk, dev, 1,
				OSPgetVulkanQueueIndexFromQueueFamily(OSP_VK_QUEUE_DEFAULT_BITS), 1.0, 0);
		}
	}

	if(!novlk && !infos) {
		infos = OSPVulkanAllocateDeviceInfos(vlk, dev, 1,
			OSPgetVulkanQueueIndexFromQueueFamily(OSP_VK_QUEUE_DEFAULT_BITS), 1.0, 0);
	}

	if(infos) {
		x = infos->queueCreateInfoCount;
		uint8_t error = 0;

		while(x--) {
			uint32_t famidx = infos->pQueueCreateInfos[x].queueFamilyIndex;
			uint32_t famcnt = infos->pQueueCreateInfos[x].queueCount;

			if(vlk->_dev[dev]._que_blk[famidx]._que_abl < famcnt) {
				error = 1;
				break;
			}

			vlk->_dev[dev]._que_blk[famidx]._que_abl -= famcnt;
			que_cnt += famcnt;
		}

		if(error) {
			while(++x < infos->queueCreateInfoCount) {
				uint32_t famidx = infos->pQueueCreateInfos[x].queueFamilyIndex;
				uint32_t famcnt = infos->pQueueCreateInfos[x].queueCount;

				vlk->_dev[dev]._que_blk[famidx]._que_abl += famcnt;
			}

			free(infos);
			infos = 0;
		}
	}

	if(!infos) {
		return 0;
	}

	ret = (OSPgpdev *) OSPAdd(OSPGpdCtr());
	ret->_dev_crt_info = infos;

	if((x = vkCreateDevice(vlk->_dev[dev]._phys_dev, infos, 0, &ret->_dev)) != VK_SUCCESS) {
		OSPrint(0, "OSPGpd : Failed to create vulkan logical device (x = %d)", x);
		x = infos->queueCreateInfoCount;

		while(x--) {
			uint32_t famidx = infos->pQueueCreateInfos[x].queueFamilyIndex;
			uint32_t famcnt = infos->pQueueCreateInfos[x].queueCount;
			vlk->_dev[dev]._que_blk[famidx]._que_abl += famcnt;
		}

		free(infos);
		ret->_dev_crt_info = 0;
		OSPFre(&ret->_obj);
		if(novlk) OSPFre(&vlk->_obj);
	    return 0;
	}

	ret->_idx = dev;
	ret->_mtr = vlk;
	ret->_nxt = vlk->_slv;
	ret->_prv = ret->_nxt ? ret->_nxt->_prv : 0;
	if(ret->_nxt) ret->_nxt->_prv = ret;
	if(ret->_prv) ret->_prv->_nxt = ret;
	vlk->_slv = ret;
	while(vlk->_slv->_prv) vlk->_slv = vlk->_slv->_prv;

	return ret;
}

uint32_t OSPgetVulkanDeviceQueueCountFromBlockID(OSPgpdev *dev, uint32_t idx) {
	return dev->_dev_crt_info->pQueueCreateInfos[idx].queueCount;
}

VkQueueFlags OSPgetVulkanDeviceQueueFlagsFromBlockID(OSPgpdev *dev, uint32_t idx) {
	OSPvulkan *vlk = dev->_mtr;
	idx = dev->_dev_crt_info->pQueueCreateInfos[idx].queueFamilyIndex;
	return vlk->_dev[dev->_idx]._que_blk[idx]._que_fam_prop.queueFlags;
}

double OSPgetVulkanDeviceQueuePriorityFromBlockID(OSPgpdev *dev, uint32_t idx) {
	return dev->_dev_crt_info->pQueueCreateInfos[idx].pQueuePriorities[0];
}

/* TODO Select best fitting queue */
uint32_t OSPgetVulkanFirstFreeQueueCommandPool(OSPgpdev *dev, VkQueueFlags *flgs, double prior, uint32_t *blk_idx) {
	uint32_t blk_idx_max = dev->_dev_crt_info->queueCreateInfoCount;

	for(blk_idx[0] = 0; blk_idx[0] < blk_idx_max; blk_idx[0]++) {
		VkQueueFlags blk_flgs = OSPgetVulkanDeviceQueueFlagsFromBlockID(dev, blk_idx[0]);
		double blk_prior = OSPgetVulkanDeviceQueuePriorityFromBlockID(dev, blk_idx[0]);

		if(blk_flgs == flgs[0] && (blk_prior >= prior)) {
			uint32_t idx;
			uint32_t idx_max = dev->_dev_crt_info->pQueueCreateInfos[blk_idx[0]].queueCount;
			uint8_t avail = 1;

			for(idx = 0; idx < idx_max; idx++) {
				OSPvkque *que;

				for(que = dev->_slv; que; que = que->_nxt) {
					if((que->_blk_idx = blk_idx[0]) && (que->_idx = idx)) avail = 0;
				}

				if(avail) return idx;
			}
		}
	}

	blk_idx[0] = 0;
	flgs[0] = 0;
	return 0;
}

void OSPVqeHdl(OSPobj **obj) {
	OSPvkque *que = (OSPvkque *) obj[0];

	/* Vulkan queue is about to be destroyed */
	if(!obj[0]->_buf) {
		
		if(que->_mtr) {
			vkDestroyCommandPool(que->_mtr->_dev, que->_pool, 0);

			if(que->_mtr->_slv == que) {
				if(que->_nxt) que->_mtr->_slv = que->_nxt;
				else que->_mtr->_slv = que->_prv;
			}

			if(que->_nxt) que->_nxt->_prv = que->_prv;
			if(que->_prv) que->_prv->_nxt = que->_nxt;
		}
	}
}

OSPctr *OSPVqeCtr() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	ret = OSPCtr(0, 0, sizeof(OSPvkque), OSPVqeHdl);

	/* ret->_fct[OSPVQE_EXAMPLE] = OSPvqe_example; */

	return ret;
}

OSPvkque *OSPVqe(OSPgpdev *dev, VkQueueFlags flgs, double prior) {
	OSPvulkan *vlk;
	OSPvkque *ret;
	uint32_t idx;
	uint32_t blk_idx;
	uint32_t *features = 0;
	VkCommandPoolCreateInfo infos;

	if(!flgs) {
		flgs = OSP_VK_QUEUE_DEFAULT_BITS;
		prior = 1.0;
	}

	if(dev) vlk = dev->_mtr;
	else {
		uint32_t x;

		if(!(vlk = OSPVlk(0, 0, 0, 0, 0, 0, 0, 0))) return 0;
		features = (uint32_t *) 1;

		for(x = 0; x < vlk->_dev_cnt; x++) {
			features = OSPgetVulkanAvailableQueueArrayFromPhysicalDeviceID(vlk, x);
			uint8_t idx = OSPgetVulkanQueueIndexFromQueueFamily(OSP_VK_QUEUE_DEFAULT_BITS);

			if(features[idx] && (dev = OSPGpd(vlk, idx, 0))) break;
		}

		if(!dev) {
			OSPFre(&vlk->_obj);
			return 0;
		}
	}

	idx = OSPgetVulkanFirstFreeQueueCommandPool(dev, &flgs, prior, &blk_idx);

	if(!flgs) {
		if(features) OSPFre(&vlk->_obj);
		return 0;
	}

	infos.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	infos.pNext = 0;
	infos.flags = 0;
	infos.queueFamilyIndex = blk_idx;

	ret = (OSPvkque *) OSPAdd(OSPVqeCtr());
	ret->_mtr = dev;
	ret->_nxt = dev->_slv;
	ret->_prv = ret->_nxt ? ret->_nxt->_prv : 0;
	if(ret->_nxt) ret->_nxt->_prv = ret;
	if(ret->_prv) ret->_prv->_nxt = ret;
	dev->_slv = ret;
	while(dev->_slv->_prv) dev->_slv = dev->_slv->_prv;

	vkGetDeviceQueue(dev->_dev, blk_idx, idx, &ret->_que);
	if(vkCreateCommandPool(dev->_dev, &infos, 0, &ret->_pool) != VK_SUCCESS) {
		OSPrint(0, "OSPVqe : Unable to allocate command pool");
		if(features) OSPFre(&vlk->_obj);
		else OSPFre(&ret->_obj);
		return 0;
	}

	ret->_que_flgs = flgs;
	ret->_que_prior = OSPgetVulkanDeviceQueuePriorityFromBlockID(dev, blk_idx);

	ret->_idx = idx;
	ret->_blk_idx = blk_idx;

	return ret;
}

