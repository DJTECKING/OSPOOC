#include"OSPlib.h"

int main(int argc, char *argv[]) {
	OSPvulkan *vulkan = OSPVlk(0, 0, 0, 0, 0, 0, 0, 0);
	OSPgpdev *videocard;

	if(!vulkan) {
		return 0;
	}

	OSPVulkanInstanceState(vulkan);

	if(!(videocard = OSPGpd(vulkan, 0, 0))) {
		return 0;
	}

	printf("video created -------\n");
	OSPVulkanInstanceState(vulkan);
	OSPFre(&videocard->_obj);
	printf("video destroyed -------\n");
	OSPVulkanInstanceState(vulkan);

	OSPFre(0);

    return 0;
}

