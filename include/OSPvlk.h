#ifndef __OSPVLK_H__
#define __OSPVLK_H__

struct OSPgpdev_s;
struct OSPvkque_s;

/* Vulkan instance structure */
typedef struct OSPvulkan_s {
	OSPobj _obj;

	struct OSPgpdev_s *_slv; /* List of loaded vulkan compatible devices */

	VkInstance _instance; /* Vulkan instance */
	VkInstanceCreateInfo _inst_info; /* Vulkan instance infos */
	VkApplicationInfo _app_info; /* App infos */

	uint32_t _dev_cnt;
	struct {
		VkPhysicalDevice _phys_dev; /* Vulkan physical device handler */
		VkPhysicalDeviceProperties _phys_dev_prop; /* Physical proprieties povided by the device */

		uint32_t _que_blk_cnt; /* Number of queue block available for this physical device */
		struct {
			uint32_t _que_abl; /* Number of queues available in this block */
			VkQueueFamilyProperties _que_fam_prop; /* Properties of queue block */
		} *_que_blk;
	} *_dev;
} OSPvulkan;

/* GraPhic/General_Purpose device structure */
typedef struct OSPgpdev_s {
	OSPobj _obj;

	struct OSPvulkan_s *_mtr;
	struct OSPgpdev_s *_prv;
	struct OSPgpdev_s *_nxt;
	struct OSPvkque_s *_slv;

	/* Physical device index in _mtr->_dev[] */
	uint32_t _idx;

	/* Soft device structure and handler */
	VkDevice _dev; /* Vulkan device handler */
	VkDeviceCreateInfo *_dev_crt_info;
} OSPgpdev;

/* Queue and command pool managment structure */
typedef struct OSPvkque_s {
	OSPobj _obj;

	struct OSPgpdev_s *_mtr;
	struct OSPvkque_s *_prv;
	struct OSPvkque_s *_nxt;

	/* Queue family index in _mtr->_dev_crt_info.pQueueCreateInfos[] */
	uint32_t _blk_idx;
	uint32_t _idx;

	VkQueue _que; /* Vulkan queue handler */
	VkQueueFlags _que_flgs; /* Vulkan queue family flags */
	float _que_prior;
	/* Copied from _mtr->_mtr->_dev[_mtr->_idx]._que_blk[_mtr->_dev_crt_info.pQueueCreateInfos[_blk_idx]
					.queueFamilyIndex]._que_fam_prop.queueFlags */

	/* Command buffers are owned by OSPdraw objects */
	VkCommandPool _pool;
} OSPvkque;

OSPctr *OSPVlkCtr();
OSPvulkan *OSPVlk(const char *, uint16_t, uint16_t, uint16_t,
					const char *, uint16_t, uint16_t, uint16_t);

uint32_t *OSPgetVulkanAvailableQueueArrayFromPhysicalDeviceID(OSPvulkan *, uint32_t);
char *OSPgetVulkanPhysicalDeviceNameFromID(OSPvulkan *, uint32_t);
uint32_t OSPgetVulkanPhysicalDeviceCount(OSPvulkan *);
void OSPVulkanInstanceState(OSPvulkan *);
uint32_t OSPgetVulkanPhysicalDeviceQueuePropertyCount(OSPvulkan *, uint32_t);
VkDeviceCreateInfo *OSPVulkanAllocateDeviceInfos(OSPvulkan *, uint32_t, ...);

OSPctr *OSPGpdCtr();
OSPgpdev *OSPGpd(OSPvulkan *, uint32_t, VkDeviceCreateInfo *);

uint32_t OSPgetVulkanDeviceQueueCountFromBlockID(OSPgpdev *, uint32_t);
VkQueueFlags OSPgetVulkanDeviceQueueFlagsFromBlockID(OSPgpdev *, uint32_t);
double OSPgetVulkanDeviceQueuePriorityFromBlockID(OSPgpdev *, uint32_t);
uint32_t OSPgetVulkanFirstFreeQueueCommandPool(OSPgpdev *, VkQueueFlags *, double, uint32_t *);

OSPctr *OSPVqeCtr();
OSPvkque *OSPVqe(OSPgpdev *, VkQueueFlags, double);

#endif /* __OSPVLK_H__ */

