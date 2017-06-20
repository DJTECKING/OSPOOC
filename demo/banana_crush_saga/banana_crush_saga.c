#include"OSPlib.h"

#define SIZE	64

typedef struct cell_s {
	struct cell_s *_u;
	struct cell_s *_d;
	struct cell_s *_l;
	struct cell_s *_r;
	struct cell_s *_p;

	int _x;
	int _y;

	int _id;
	int _path;
	int _solv;
	int _prop;
} cell_t;

void gencell(cell_t *cell, uint32_t *seed) {
	int corresp[16] = {
		2,
		2,
		2,

		0x0F,
		0x0F,
		0x0F,
		0x03,
		0x0C,

		0x09,
		0x0A,
		0x06,
		0x05,

		0x0D,
		0x0B,
		0x0E,
		0x07
	};

	if(cell->_id) return;
	cell->_id = corresp[RANDOM(*seed) & 0x0F];
}

uint32_t genpix(float red, float green, float blue) {
	uint32_t pix = 0;

	if(red > 1) red = 1;
	if(red < 0) red = 0;
	if(green > 1) green = 1;
	if(green < 0) green = 0;
	if(blue > 1) blue = 1;
	if(blue < 0) blue = 0;

	pix = red * 255;
	pix <<= 8;
	pix |= (int) (green * 255);
	pix <<= 8;
	pix |= (int) (blue * 255);

	return pix | 0xFF000000;
}

OSPimage *texture(void *context, char *path) {
	static OSPimage *ret = 0;
	return context && path ? ret = OSPImgLoad(context, path, 0) : ret;
}

void drawcell(OSPwindow *wnd, OSPimage *img, cell_t *cell,
				int curx, int cury, uint32_t *seed) {
	int x = cell->_x;
	int y = cell->_y;
	int lx = -(SIZE >> 1);
	int ly = -(SIZE >> 1);
	int ex;
	int ey;

	int selected = 0;

	if(x == curx && y == cury) {
		selected = 1;
	}

	x *= SIZE;
	y *= SIZE;

	ex = x + SIZE;
	ey = y + SIZE;

	for(y = ey - SIZE; y < ey; y++) {
		for(x = ex - SIZE; x < ex; x++) {
			float grad = 1. - sqrt((lx * lx + ly * ly) / 10000.);

			img->_data[y][x] = genpix(0, grad / 3., grad / 2.);

			lx++;
		}

		lx = -(SIZE >> 1);
		ly++;
	}

	x = ex - SIZE;
	y = ey - SIZE;

	switch(cell->_id) {
	case 1:
	case 2:
		OSPBlit(texture(0, 0), img, SIZE * cell->_id, 0, x, y, SIZE, SIZE);
		break;
	default:
		OSPBlit(texture(0, 0), img, SIZE * 3, 0, x, y, SIZE, SIZE);

		ey = 0;
		for(ex = 0x01; ex & 0x0F; ex <<= 1) {
			if(cell->_id & ex) OSPBlit(texture(0, 0), img, SIZE * ey, SIZE, x, y, SIZE, SIZE);
			ey++;
		}

//		if(!(RANDOM(*seed) % 3)) OSPBlit(texture(0, 0), img, (RANDOM(*seed) % 4) * SIZE,
//													((RANDOM(*seed) % 2) + 4) * SIZE, x, y, SIZE, SIZE);

	case 0:;
	}

	ey = 0;
	for(ex = 0x01; ex & 0x0F; ex <<= 1) {
		if(cell->_solv & ex) OSPBlit(texture(0, 0), img, SIZE * ey, SIZE * 2, x, y, SIZE, SIZE);
		ey++;
	}

	if(selected) {
		OSPBlit(texture(0, 0), img, 0, 0, x, y, SIZE, SIZE);
	}

	OSPBlit(img, wnd, x, y, x, y, SIZE, SIZE);
}

int check(cell_t *cell, cell_t **shortest) {
	int dir;
	int ret = 0;

	for(dir = 0x08; dir; dir >>= 1) {
		int opdir;
		cell_t *celldir;

		switch(dir) {
		case 0x01:
			opdir = 0x02;
			celldir = cell->_u;
			break;

		case 0x02:
			opdir = 0x01;
			celldir = cell->_d;
			break;

		case 0x04:
			opdir = 0x08;
			celldir = cell->_l;
			break;

		default:
			opdir = 0x04;
			celldir = cell->_r;
		}

		if(celldir) {
			switch(celldir->_id) {
			case 2:
				if((cell->_id == 1) || (cell->_id & dir)) {
					celldir->_path = cell->_path + 1;

					if(shortest && (!shortest[0] || !shortest[0]->_path ||
						(shortest[0]->_path > celldir->_path))) {

						shortest[0] = celldir;
					}

					return 1;
				}
				break;

			default:
				if(((cell->_id == 1) || (cell->_id & dir)) &&
					((celldir->_path > cell->_path + 1) || !celldir->_path) &&
					(celldir->_id & opdir)) {

					celldir->_path = cell->_path + 1;
					ret |= check(celldir, shortest);
				}

			case 1:;
			}
		}
	}

	if(!ret) cell->_path = 0;
	return ret;
}

void trace(cell_t *root) {
	int dir;

	if(root->_id == 1) return;

	for(dir = 0x08; dir; dir >>= 1) {
		int opdir;
		cell_t *celldir;

		switch(dir) {
		case 0x01:
			opdir = 0x02;
			celldir = root->_u;
			break;

		case 0x02:
			opdir = 0x01;
			celldir = root->_d;
			break;

		case 0x04:
			opdir = 0x08;
			celldir = root->_l;
			break;

		default:
			opdir = 0x04;
			celldir = root->_r;
		}

		if(celldir) {
			if(((root->_id & dir) || (root->_id == 2)) &&
				((celldir->_id & opdir) || (celldir->_id == 1)) &&
				(celldir->_path == (root->_path - 1))) {

				celldir->_p = root;
				trace(celldir);
				break;
			}
		}
	}
}

cell_t *resolve(cell_t **cell, int sx, int sy) {
	int x;
	int y;

	cell_t *result[2] = {0};

	for(y = 0; y < sy; y++) {
		for(x = 0; x < sx; x++) {
			if(cell[x][y]._id == 1) {
				result[0] = &cell[x][y];
				result[0]->_path = 1;

				if(check(result[0], &result[1])) {
					trace(result[1]);

					for(y = 0; y < sy; y++) {
						for(x = 0; x < sx; x++) {
							cell[x][y]._path = 0;
						}
					}

					return result[0];
				}
				else {
					result[0]->_path = 0;
				}
			}
		}
	}

	return 0;
}

cell_t *find(cell_t **cell, int sx, int sy) {
	int x;
	int y;

	cell_t *result = 0;

	for(y = 0; y < sy; y++) {
		for(x = 0; x < sx; x++) {
			if(cell[x][y]._id == 1) {
				result = &cell[x][y];
				result->_path = 1;

				if(check(result, 0)) {
					for(y = 0; y < sy; y++) {
						for(x = 0; x < sx; x++) {
							cell[x][y]._path = 0;
						}
					}

					return result;
				}
				else {
					result->_path = 0;
				}
			}
		}
	}

	return 0;
}

void shake(cell_t **cell, int sx, int sy, uint32_t *seed) {
	cell_t *array = &cell[0][0];
	int pcs[16] = {0};
	int newpdx = 0x03;
	int pnb = 0;
	int size = sx * sy;
	int idx;
	int x;
	int y;

	for(idx = 0; idx < size; idx++) {
		if(array[idx]._id > 2) {
			pcs[array[idx]._id]++;
			array[idx]._id = 4;
			pnb++;
		}
	}

	while(pnb--) {
		while(!pcs[newpdx]) newpdx++;
		pcs[newpdx]--;

		idx = (idx + RANDOM(*seed)) % size;
		while(array[idx]._id != 4) idx = (idx + 1) % size;
		array[idx]._id = newpdx;
	}
}

int main(int argc, char *argv[]) {
	OSPwindow *wnd;
	OSPimage *img;
	OSPobj *trig;
	cell_t **cell = 0;
	char texpath[50] = "demo/banana_crush_saga/banana_default_texture.tga";

	int x;
	int y;
	int cx = 0;
	int cy = 0;
	int sx = 0;
	int sy = 0;
	int run = 1;
	int tmo = 16;
	int process = 1;
	int redraw = 0;
	int level = -1;
	uint32_t seed = time(0);
	cell_t *start;

	enum {
		MENU,
		NETWORK,
		CHOOSE,
		RUN,
		FALL,
		FIND
	} state = FIND;

	OSPrint(2, 0);
	srand(seed);

	if(argc != 2) {
		printf("Need level\n");
		return -1;
	}

	if((level = open(argv[1], O_RDONLY)) < 0) {
		printf("Unable to load level %s : %s\n", argv[1], strerror(errno));
		OSPFre(0);
		return -1;
	}

	{
		char *filler = texpath;
		char input;

		while((read(level, &input, 1) == 1) && (input != '\n')) *(filler++) = input;
		if(filler - texpath) filler[0] = 0;

		while((read(level, &input, 1) == 1)) {
			if(input == '\n') sy++;
			if(!sy) sx++;
		}

		if(lseek(level, filler - texpath, SEEK_SET) < 0) {
			close(level);
			printf("Unable to jump to %s datas : %s\n", level, strerror(errno));
			OSPFre(0);
			return -1;
		}
	}

	cell = (cell_t **) OSPArray(sizeof(cell_t), sx, sy, 0);

	wnd = OSPWnd(0, "Banana crush saga!!!", 0, 0, sx * SIZE, sy * SIZE, 0x00000000);

	if(!wnd) {
		OSPFre(0);
		return -1;
	}

	img = OSPImg(wnd, sx * SIZE, sy * SIZE);

	if(!img) {
		OSPFre(0);
		return -1;
	}

	if(!texture(wnd, texpath)) {
		close(level);
		printf("Unable to load %s : %s\n", texpath, strerror(errno));
		OSPFre(0);
		return -1;
	}

	cell[RANDOM(seed) % sx][RANDOM(seed) % sy]._id = 1;

	/* Filling array */
	for(y = 0; y < sy; y++) {
		for(x = 0; x < sx; x++) {
			int ix;
			int iy;
			gencell(&cell[x][y], &seed);

			if(x) cell[x][y]._l = &cell[x - 1][y];
			if(x < (sx - 1)) cell[x][y]._r = &cell[x + 1][y];
			if(y) cell[x][y]._u = &cell[x][y - 1];
			if(y < (sy - 1)) cell[x][y]._d = &cell[x][y + 1];

			cell[x][y]._x = x;
			cell[x][y]._y = y;

			drawcell(wnd, img, &cell[x][y], cx, cy, &seed);
		}
	}

	if(start = resolve(cell, sx, sy)) {
		tmo = 10;
		state = RUN;
	}

	/* Main loop */
	while(run) {
		int timestat = tmo;
		trig = OSPWte(&timestat);

		if(timestat) { /* Window cancelled */
			run = 0;
		}
		else{
			uint16_t btn;

			switch(state) {
			case CHOOSE:
				OSPRun(&wnd->_obj, OSPWND_GETBTN, &btn);

				if((btn & 0x0101) == 0x0001) {
					int lcx = cx;
					int lcy = cy;
					cx = wnd->_msx / SIZE;
					cy = wnd->_msy / SIZE;

					if(((lcx == cx) && ((lcy == cy + 1) || (lcy == cy - 1))) ||
						((lcy == cy) && ((lcx == cx + 1) || (lcx == cx - 1)))) {
						int lid = cell[lcx][lcy]._id;
						cell[lcx][lcy]._id = cell[cx][cy]._id;
						cell[cx][cy]._id = lid;

						if(start = resolve(cell, sx, sy)) {
							tmo = 100;
							state = RUN;
						}
						else {
							cell[cx][cy]._id = cell[lcx][lcy]._id;
							cell[lcx][lcy]._id = lid;
						}
					}

					drawcell(wnd, img, &cell[lcx][lcy], cx, cy, &seed);

					if(state == RUN) {
						for(y = 0; y < sy; y++) {
							for(x = 0; x < sx; x++) {
								if(cell[x][y]._solv) {
									cell[x][y]._solv = 0x00;
									drawcell(wnd, img, &cell[x][y], cx, cy, &seed);
								}
							}
						}

						drawcell(wnd, img, &cell[cx][cy], -1, -1, &seed);
						cx = -1;
						cy = -1;
					}
					else {
						drawcell(wnd, img, &cell[cx][cy], cx, cy, &seed);
					}

#ifdef OSP_XDBE_SUPPORT
					OSPRun(&wnd->_obj, OSPWND_SWAP);
#endif
				}
				if((btn & 0x0404) == 0x0004) {
					cx = wnd->_msx / SIZE;
					cy = wnd->_msy / SIZE;
					cell[cx][cy]._id = 0;
					cx = -1;
					cy = -1;

					process = 1;
					state = FALL;
				}
				break;

			case RUN:
				if(start) {
					cell_t *last = start;
					start = start->_p;
					last->_p = 0;

					if(start) {
						last->_id = 0;
						start->_id = 1;
						drawcell(wnd, img, last, cx, cy, &seed);
						drawcell(wnd, img, start, cx, cy, &seed);

#ifdef OSP_XDBE_SUPPORT
						OSPRun(&wnd->_obj, OSPWND_SWAP);
#endif
					}
				}
				else {
					process = 1;
					state = FALL;
				}
				break;

			case FALL:
				if(process) {
					int banana = 3;
					process = 0;

					for(y = sy - 1; y >= 0; y--) {
						for(x = 0; x < sx; x++) {
							if(!cell[x][y]._id) {
								if(y) {
									cell[x][y]._id = cell[x][y - 1]._id;
									cell[x][y - 1]._id = 0;
								}
								else gencell(&cell[x][y], &seed);
								drawcell(wnd, img, &cell[x][y], -1, -1, &seed);
								process = 1;
							}
							else if((cell[x][y]._id == 2) && banana) banana = 0;
						}
					}

					while(!process && banana--) {
						while(1) {
							cell_t *pick = &cell[RANDOM(seed) % sx][RANDOM(seed) % sy];
							if(pick->_id < 2) continue;
							pick->_id = 2;
							drawcell(wnd, img, pick, -1, -1, &seed);
							break;
						}
					}

#ifdef OSP_XDBE_SUPPORT
					OSPRun(&wnd->_obj, OSPWND_SWAP);
#endif
				}
				else {
					if(start = resolve(cell, sx, sy)) {
						state = RUN;
					}
					else {
						state = FIND;
					}
				}
				break;

			case FIND:
				process = 1;

				for(y = 0; y < sy; y++) {
					for(x = 0; x < sx; x++) {
						if(cell[x][y]._solv) {
							cell[x][y]._solv = 0x00;
							drawcell(wnd, img, &cell[x][y], cx, cy, &seed);
						}
					}
				}

				while(process) {
					for(y = 0; y < sy; y++) {
						for(x = 0; x < sx; x++) {
							if(cell[x][y]._id) {
								if(x && cell[x - 1][y]._id) {
									int lid = cell[x][y]._id;
									cell[x][y]._id = cell[x - 1][y]._id;
									cell[x - 1][y]._id = lid;

									if(find(cell, sx, sy)) {
										process = 0;
										cell[x][y]._solv |= 0x04;
										cell[x - 1][y]._solv |= 0x08;
									}

									cell[x - 1][y]._id = cell[x][y]._id;
									cell[x][y]._id = lid;
								}

								if(y && cell[x][y - 1]._id) {
									int lid = cell[x][y]._id;
									cell[x][y]._id = cell[x][y - 1]._id;
									cell[x][y - 1]._id = lid;

									if(find(cell, sx, sy)) {
										process = 0;
										cell[x][y]._solv |= 0x01;
										cell[x][y - 1]._solv |= 0x02;
									}

									cell[x][y - 1]._id = cell[x][y]._id;
									cell[x][y]._id = lid;
								}
							}
						}
					}

					if(process) {
						shake(cell, sx, sy, &seed);
						redraw++;
					}
				}

				if(redraw) {
					printf("%d shakes\n", redraw);
					redraw = 0;

					if(start = resolve(cell, sx, sy)) {
						for(y = 0; y < sy; y++) {
							for(x = 0; x < sx; x++) {
								if(cell[x][y]._solv) {
									cell[x][y]._solv = 0x00;
								}
							}
						}
					}

					for(y = 0; y < sy; y++) {
						for(x = 0; x < sx; x++) {
							drawcell(wnd, img, &cell[x][y], cx, cy, &seed);
						}
					}
				}
				else {
					for(y = 0; y < sy; y++) {
						for(x = 0; x < sx; x++) {
							if(cell[x][y]._solv) drawcell(wnd, img, &cell[x][y], cx, cy, &seed);
						}
					}
				}

#ifdef OSP_XDBE_SUPPORT
				OSPRun(&wnd->_obj, OSPWND_SWAP);
#endif

				if(start) {
					state = RUN;
					break;
				}

				cx = 0;
				cy = 0;
				tmo = 16;
				state = CHOOSE;
				break;
			}
		}
	}

	free(cell);
	OSPFre(0);
	return 0;
}

