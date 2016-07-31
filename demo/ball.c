#include"OSPlib.h"

#define WIDTH	640
#define HEIGHT	480

#define SIZE	100

int main(int argc, char *argv[]) {
	OSPdisplay *dpy;
	OSPwindow *wnd;
	OSPwindow *wnd2;
	OSPimage *img;
	OSPimage *ball;
	OSPobj *trig;
	
	int x;
	int y;
	int z;
	int run = 0x03; /* 0x01 = Demo window still alive */
					/* 0x02 = Blitted window still alive */

	double ball_xd = 3.7;
	double ball_yd = 0;
	double ball_xp = 0;
	double ball_yp = 0;

	/* Create objects */

	wnd = OSPWnd(0, "OSPOOC Demo", 0, 0, WIDTH, HEIGHT, 0x00000000);

	if(!wnd) {
		OSPFre(0);
		return 0;
	}

	dpy = OSPDpyOf(wnd);

	wnd2 = OSPWnd(dpy, "Blitted", 0, 0, WIDTH / 2, HEIGHT / 2, 0x00000000);

	if(!wnd2) {
		OSPFre(0);
		return 0;
	}

	img = OSPImg(dpy, WIDTH, HEIGHT);

	if(!img) {
		OSPFre(0);
		return 0;
	}

	ball = OSPImg(dpy, SIZE, SIZE * 0x0600);

	if(!ball) {
		OSPFre(0);
		return 0;
	}

	/* Filling images */

	for(y = 0; y < HEIGHT; y++) {
		for(x = 0; x < WIDTH; x++) {
			img->_data[y][x] = 0x00000000;
		}
	}

	for(z = 0; z < 0x0600; z++) {
		uint32_t color;
		uint8_t fade = z;

		switch(z & 0x0700) {
			case 0x0000:
				color = 0x0000FF00 | (fade << 16);
				break;

			case 0x0100:
				color = 0x00FF0000 | ((0xFF - fade) << 8);
				break;

			case 0x0200:
				color = 0x00FF0000 | fade;
				break;

			case 0x0300:
				color = 0x000000FF | ((0xFF - fade) << 16);
				break;

			case 0x0400:
				color = 0x000000FF | (fade << 8);
				break;

			default:
				color = 0x0000FF00 | (0xFF - fade);
		}

		for(y = 0; y < SIZE; y++) {
			for(x = 0; x < SIZE; x++) {
				uint8_t rayon = SIZE / 3;
				int8_t refx = x - (SIZE / 2);
				int8_t refy = y - (SIZE / 2);
				uint32_t dist = sqrt((refx * refx) + (refy * refy));

				if(dist >= rayon) {
					ball->_data[y + (z * SIZE)][x] = 0x00000000;
				}
				else {
					dist = (0xFF000000 / rayon) * (rayon - dist);
					ball->_data[y + (z * SIZE)][x] =
									(dist & 0xFF000000) | color;
				}
			}
		}
	}

	z = 0;

	/* Main loop */

	while(run) {
		int timestat = 16;
		trig = OSPWte(&timestat);

		if(timestat) { /* One window were cancelled */
			if(trig == &wnd->_obj) {
				run &= ~0x01;
			}

			if(trig == &wnd2->_obj) {
				run &= ~0x02;
			}

			if(trig == &dpy->_obj) {
				run = 0;
			}
		}
		else{
			uint16_t btn;

			if(run & 0x01) { /* Is demo window still alive */
				OSPRun(&wnd->_obj, OSPWND_GETBTN, &btn);
			}
			else {
				btn = 0;
			}

			/* Change ball color */
			if(z >= 0x05FF) {
				z = 0;
			}
			else {
				z++;
			}

			/* Change ball place */
			ball_xp += ball_xd;
			ball_yp += ball_yd;
			ball_yd += 0.1;

			/* Window borders interactions */
			if(ball_xp < 0) {
				ball_xp = 0;
				ball_xd = -ball_xd;
			}

			if(ball_xp > (WIDTH - SIZE)) {
				ball_xp = WIDTH - SIZE;
				ball_xd = -ball_xd;
			}

			if(ball_yp < 0) {
				ball_yp = 0;
				ball_yd = -ball_yd;
			}

			if(ball_yp > (HEIGHT - SIZE)) {
				ball_yp = HEIGHT - SIZE;
				ball_yd = -(ball_yd * 0.9);
			}

			/* Buttons interactions */
			if((btn & 0x0101) == 0x0001) {
				ball_yd *= 3;
			}

			if((btn & 0x0404) == 0x0004) {
				for(y = 0; y < HEIGHT; y++) {
					for(x = 0; x < WIDTH; x++) {
						img->_data[y][x] = 0xFF000000;
					}
				}

				run |= 0x04;
			}

			if(run & 0x01) { /* Is demo window still alive */
				/* Blitting images to demo windows */
				OSPBlit(ball, img, 0, z * SIZE, ball_xp, ball_yp, SIZE, SIZE);
				if(run & 0x04) {
					OSPBlit(img, wnd, 0, 0, 0, 0, WIDTH, HEIGHT);
					run &= ~0x04;
				}
				else {
					OSPBlit(img, wnd, ball_xp, ball_yp, ball_xp, ball_yp, SIZE, SIZE);
				}
#ifdef OSP_XDBE_SUPPORT
				OSPRun(&wnd->_obj, OSPWND_SWAP);
#endif
			}

			if(run & 0x02) { /* Is blitted window still alive */
				if(((ball_xp + SIZE) > (WIDTH / 4)) &&
					(ball_xp < ((3 * WIDTH) / 4)) &&
					((ball_yp + SIZE) > (HEIGHT / 4)) &&
					(ball_yp < ((3 * HEIGHT) / 4))) {

					uint16_t src_x = ball_xp > (WIDTH / 4) ? 0 : (WIDTH / 4) - ball_xp;
					uint16_t src_y = ball_yp > (HEIGHT / 4) ? 0 : (HEIGHT / 4) - ball_yp;
					uint16_t dst_x = ball_xp > (WIDTH / 4) ? ball_xp - (WIDTH / 4) : 0;
					uint16_t dst_y = ball_yp > (HEIGHT / 4) ? ball_yp - (HEIGHT / 4) : 0;
					uint16_t sze_x = (ball_xp + SIZE) > ((3 * WIDTH) / 4) ?
									SIZE + ((3 * WIDTH) / 4) - (ball_xp + SIZE) : SIZE;
					uint16_t sze_y = (ball_yp + SIZE) > ((3 * HEIGHT) / 4) ?
									SIZE + ((3 * HEIGHT) / 4) - (ball_yp + SIZE) : SIZE;
					if(ball_xp < (WIDTH / 4)) sze_x = SIZE + ball_xp - (WIDTH / 4);
					if(ball_yp < (HEIGHT / 4)) sze_y = SIZE + ball_yp - (HEIGHT / 4);

					OSPBlit(ball, wnd2, src_x, src_y + (z * SIZE), dst_x, dst_y, sze_x, sze_y);
				}

#ifdef OSP_XDBE_SUPPORT
				OSPRun(&wnd2->_obj, OSPWND_SWAP);
#endif
			}
		}
	}

	OSPFre(0);
	return 0;
}

