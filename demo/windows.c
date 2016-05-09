#include"OSPlib.h"

int main(int argc, char *argv[]) {
	OSPdisplay *dpy = OSPDpy(0);
	OSPobj *trig;
	int run = 1;

	OSPWnd(&dpy->_obj, "OSPOOC Demo", 0, 0, 640, 480, 0x0000FF00);

	while(run) {
		int timestat = 37;
		trig = OSPWte(&timestat);

		if(timestat) {
			if(trig == &dpy->_obj) {
				run = 0;
			}
		}
		else{
			// printf("timout\n");
		}
	}

	OSPFre(0);
	return 0;
}

