#include"OSPlib.h"

#define WIDTH	640
#define HEIGHT	480

#define SIZE	100

int main(int argc, char *argv[]) {
	OSPwindow *wnd;
	OSPscene *scn;
	OSPobj *trig;
	
	int x;
	int y;
	int z;
	int run = 0x01;
	double green = 0;

	/* Create objects */

	wnd = OSPWnd(0, "OSPOOC Demo", 0, 0, WIDTH, HEIGHT, 0x00000000);

	if(!wnd) {
		OSPFre(0);
		return 0;
	}

	scn = OSPScn(wnd, 0, 0, WIDTH, HEIGHT, 0x00000000);

	if(!scn) {
		OSPFre(0);
		return 0;
	}

	while(run) {
		int timestat = 16;
		trig = OSPWte(&timestat);

		if(timestat) { /* One window were cancelled */
			if(trig == &OSPDpyOf(wnd)->_obj) {
				run = 0;
			}
		}
		else{
			if(green > 1) green = 0;
			else green += 0.1;

			glClearColor(0, green, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);
			OSPRun(&scn->_obj, OSPWND_SWAP);
		}
	}

	OSPFre(0);
	return 0;
}

