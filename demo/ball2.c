#include"OSPlib.h"

#define WIDTH	640
#define HEIGHT	480

#define SIZE	100

int main(int argc, char *argv[]) {
	OSPwindow *wnd;
	OSPimage *img;
	OSPimage *ball;
	
	int x;
	int y;
	int z;
	int run = 1;

	double ball_xd = 9.3;
	double ball_yd = 0;
	double ball_xp = 0;
	double ball_yp = 0;

	/* Create objects */

	wnd = OSPWnd(0, "OSPOOC Demo", 0, 0, WIDTH, HEIGHT, 0x00000000);

	if(!wnd) {
		OSPFre(0);
		return 0;
	}

	img = OSPImg(wnd, WIDTH, HEIGHT);

	if(!img) {
		OSPFre(0);
		return 0;
	}

	ball = OSPImgLoad(wnd, "demo/banana_crush_saga/banana_default_texture.tga", 0);

	if(!ball) {
		OSPFre(0);
		return 0;
	}

	int timestat = 16;
	OSPWte(&timestat);

	OSPBlit(ball, wnd, 0, 0, 0, 0, SIZE, SIZE);
#ifdef OSP_XDBE_SUPPORT
	OSPRun(&wnd->_obj, OSPWND_SWAP);
#endif

	timestat = 16;
	OSPWte(&timestat);
	timestat = 16;
	OSPWte(&timestat);

	sleep(3);

	OSPFre(0);
	return 0;
}

