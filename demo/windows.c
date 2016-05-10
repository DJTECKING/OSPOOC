#include"OSPlib.h"

int main(int argc, char *argv[]) {
	OSPdisplay *dpy = OSPDpy(0);
	OSPobj *trig;
	
	int x;
	int y;
	uint8_t offset = 0;
	int run = 1;

	OSPwindow *win1 = OSPWnd(&dpy->_obj, "OSPOOC Demo", 0, 0, 640, 480, 0x0000FF00);
//	OSPWnd(&dpy->_obj, "OSPOOC Demo2", 0, 0, 640, 480, 0x000000FF);
	OSPimage *img = OSPImg(dpy, 640, 480);

	while(run) {
		int timestat = 16;
		trig = OSPWte(&timestat);

		if(timestat) {
			if(trig == &dpy->_obj) {
				run = 0;
			}
		}
		else{
			offset += 9;

			for(y = 0; y < 480; y++) {
				for(x = 0; x < 640; x++) {
					uint8_t val = x + y + offset;
					img->_data[x][y] = val | (val << 8) | (val << 16);
				}
			}

			OSPImgBlit(&img->_obj, &win1->_obj, 0, 0, 0, 0, 640, 480);

			OSPRun(&win1->_obj, OSPWND_SWAP);
		}
	}

	OSPFre(0);
	return 0;
}

