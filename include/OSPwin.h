#ifndef __OSPWIN_H__
#define __OSPWIN_H__

#include"../include/OSPlib.h"

#define OSPDPY_FLUSH			0

#define OSPWND_EVENT			0
#define OSPWND_GETKEY			1
#define OSPWND_GETBTN			2
#define OSPWND_SWAP				3

#define OSPIMG_GETDATA			0

#define OSPIMG_OPTION_LOCAL		0
#define OSPIMG_OPTION_SHARED	1

struct OSPwindow_s;

typedef struct OSPdisplay_s {
	OSPobj _obj;

	struct OSPwindow_s *_slv;
	Display *_dpy;
	XEvent _lst;
	XVisualInfo _vfo;
	XSetWindowAttributes _atr;

	int _scn;
	int _msx;
	int _msy;

	Atom _WMm[2];

	uint8_t _Keypad[64];
} OSPdisplay;

typedef struct OSPwindow_s {
	OSPobj _obj;

	struct OSPwindow_s *_slv;
	struct OSPwindow_s *_mtr; /* Is the OSPdisplay * if parent is root window */
	struct OSPwindow_s *_prv;
	struct OSPwindow_s *_nxt;

	OSPdisplay *_dpy;
	Window _wnd;
	XdbeBackBuffer _bbf;
	GC _gc;
	int _x;
	int _y;
	int _w;
	int _h;
	int _msx;
	int _msy;
	uint16_t _btn;
} OSPwindow;

typedef struct OSPimage_s {
	OSPobj _obj;

	OSPdisplay *_dpy;
	XImage *_img;
	XShmSegmentInfo _shm;

	uint32_t **_data;
	uint8_t _stat;
} OSPimage;

OSPctr *OSPDpyCtr();
OSPdisplay *OSPDpyOf(OSPwindow *wnd);
OSPdisplay *OSPDpy(char *dpyname);

OSPctr *OSPWndCtr();
OSPwindow *OSPWnd(void *, char *, int, int, int, int, uint32_t);

OSPctr *OSPImgCtr();
OSPimage *OSPImg(void *, int, int, uint8_t);


void OSPImgBlit(void *, void *, int, int, int, int, unsigned int, unsigned int);

#endif /* __OSPWIN_H__ */
