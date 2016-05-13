#include"OSPlib.h"

#define WIDTH	640
#define HEIGHT	480

int main(int argc, char *argv[]) {
	OSPdisplay *dpy = OSPDpy(0);
	OSPobj *trig;
	
	int x;
	int y;
	uint8_t offset = 0;
	double floatset = 254;
	int run = 1;

	if(!dpy) {
		OSPFre(0);
		return 0;
	}

	OSPwindow *win = OSPWnd(dpy, "OSPOOC Demo", 0, 0, WIDTH, HEIGHT, 0x0000FF00);

	if(!win) {
		OSPFre(0);
		return 0;
	}

	OSPimage *img = OSPImg(dpy, WIDTH, HEIGHT, OSPIMG_OPTION_LOCAL);
/*	OSPimage *img = OSPImg(dpy, WIDTH, HEIGHT, OSPIMG_OPTION_SHARED); */

	if(!img) {
		OSPFre(0);
		return 0;
	}

	while(run) {
		int timestat = 10;
	OSPRun(&dpy->_obj, OSPDPY_FLUSH);
		trig = OSPWte(&timestat);

		if(timestat) {
			if(trig == &dpy->_obj) {
				run = 0;
			}
		}
		else{
			offset += 1;
			floatset += 0.01;

			for(y = 0; y < HEIGHT; y++) {
				for(x = 0; x < WIDTH; x++) {
					int xrelat = x - (WIDTH >> 1);
					int yrelat = y - (HEIGHT >> 1);
					uint8_t red = x + y + offset;
					uint8_t green = (sin(sqrt((xrelat * xrelat) +
									(yrelat * yrelat) >> 6) - offset) * 128) + 128;
					uint8_t blue = xrelat * yrelat * floatset;

					img->_data[y][x] = blue | (green << 8) | (red << 16);
				}
			}

			OSPImgBlit(img, win, 0, 0, 0, 0, WIDTH, HEIGHT);

			OSPRun(&win->_obj, OSPWND_SWAP);
		}
	}

	OSPFre(0);
	return 0;
}

