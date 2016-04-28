#include"../include/OSPlib.h"

int main(int argc, char *argv[]) {
	OSPwindow *window1 = OSPWnd(0, "Win1", 0, 0, 500, 400, 0x00FF0000);
	OSPwindow *window2 = OSPWnd(&OSPDpyOf(window1)->_obj, "Win2", 0, 0, 400, 300, 0x0000FF00);
	OSPwindow *window3 = OSPWnd(&window1->_obj, "Win3", 0, 0, 400, 300, 0x000000FF);
	OSPobj *trig;
	int timestat = 37;
	int run = 3;

	while(run) {
		trig = OSPWte(&timestat);

		if(timestat) {
			if(trig == &OSPDpyOf(window1)->_obj) {
				run = 0;
			}
		}
	}

	OSPFre(0);
	return 0;
}

