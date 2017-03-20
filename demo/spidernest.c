#include"OSPlib.h"

#define WIDTH	320
#define HEIGHT	280

uint32_t hash(uint32_t a) {
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    return a;
}

double getoffset(uint32_t seed, uint32_t cx, uint32_t cy) {
	if(!seed) seed++;
	seed = hash(seed);
	seed = hash(seed + cx);
	seed = hash(seed + cy);
	return seed;
}

double getangle(double dx, double dy) {
	if(dx == 0) {
		if(dy == 0) return 0;
		return dy > 0 ? M_PI_2 : -M_PI_2;
	}

	return atan(dy/dx) + (dx > 0 ? 0 : (dy > 0 ? -M_PI : M_PI));
}

double getbell(double t) {
	double ret = t;

	if(ret >= 2) return 0;
	ret *= t - 4; // t2 - 4 t
	ret *= t; // t3 - 4 t2
	ret += 4 * t; // t3 - 4 t2 + 4 t
	return ret * t; // t4 - 4 t3 + 4 t2
}

double getgrad(double angle) {
	double grad = fmod(angle, 2 * M_PI); // 2 M_PI > grad > -2 M_PI
	
	if(grad < 0) grad += 2 * M_PI; // 2 M_PI > grad >= 0
	return getbell(grad / M_PI) * 2 - 1;
}

double getcos(double dx, double dy) {
	double distance = sqrt((dx * dx) + (dy * dy));
	return getbell(distance);
}

double getpartial(uint32_t seed, double px, double py, uint32_t cx, uint32_t cy) {
	double dx = px - cx;
	double dy = py - cy;
	return getgrad(getangle(dx, dy) + getoffset(seed, cx, cy)) * getcos(dx, dy);
}

double getheight(uint32_t seed, double px, double py) {
	double ret = 0;
	uint32_t maxx = px + 3;
	uint32_t maxy = py + 3;
	uint32_t x, y;

	for(y = py -1; y < maxy; y++) {
		for(x = px -1; x < maxx; x++) {
			// if(!(x & 3) && !(y & 3)) ret += getpartial(seed, px, py, x, y);
			ret += getpartial(seed, px, py, x, y);
		}
	}

	return (ret / 6);
}

int main(int argc, char *argv[]) {
	OSPwindow *wnd;
	OSPimage *img;
	uint32_t map = time(0);
	uint16_t x, y;

	wnd = OSPWnd(0, "OSPOOC Demo", 0, 0, WIDTH, HEIGHT, 0x00000000);
	img = OSPImg(wnd, WIDTH, HEIGHT);

	for(y = 0; y < HEIGHT; y++) {
		for(x = 0; x < WIDTH; x++) {
			uint32_t dx = x - 450 + 290;
			uint32_t dy = y - 450 + 10;
			double distance = sqrt((dx * dx) + (dy * dy));
			uint8_t octave = 15;
			double noise = getheight(map, ((double) (x + 290)) / octave, ((double) (y + 10)) / octave);
			uint32_t col = 0;
			double val;

			if(distance > 400) val = 0;
			else if(distance > 350) val = noise + (0.004 * distance) - 1.3;
			else if(distance > 250) val = noise + 0.1;
			else if(distance > 200) val = noise + 4.6 - (0.018 * distance);
			else val = 1;

			if(val > 0) {
				double noise1 = getheight(map, ((double) x) / 5, ((double) y) / 5);
				double noise2 = getheight(map, ((double) x + 100) / 5, ((double) y + 100) / 5);
				uint8_t fade;

				
				fade = 128 - (fmax(noise1, noise2) * 120);
				col = fade;
				col <<= 8;
				col |= fade;
				col <<= 8;
				col |= fade;
				col |= 0xFF000000;
			}

			img->_data[y][x] = col | 0xFF000000;
//			img->_data[y][x] = 0xFF000000 | (uint32_t) getoffset(map, x, y); To see hash function
		}
	}
	
	OSPBlit(img, wnd, 0, 0, 0, 0, WIDTH, HEIGHT);

#ifdef OSP_XDBE_SUPPORT
	OSPRun(&wnd->_obj, OSPWND_SWAP);
#endif

	sleep(10);

	OSPFre(0);

	return 0;
}

