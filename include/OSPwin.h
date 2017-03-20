#ifndef __OSPWIN_H__
#define __OSPWIN_H__

#define OSPDPY_FLUSH			0

#define OSPWND_EVENT			0
#define OSPWND_GETKEY			1
#define OSPWND_GETBTN			2
#define OSPWND_SWAP				3
#define OSPSCN_FOCUS			4
#define OSPSCN_LOOK				5

#define OSPIMG_GETDATA			0

struct OSPwindow_s;
struct OSPscelem_s;

typedef struct OSPdisplay_s {
	OSPobj _obj;

	struct OSPwindow_s *_slv;

	Display *_dpy;
	XEvent _lst;
	XVisualInfo _vfo[2];
	XSetWindowAttributes _atr[2];

	int _scn;
	int _msx;
	int _msy;

	Atom _WMm[2];

	uint8_t _Keypad[64];
} OSPdisplay;

typedef struct OSPwindow_s {
	OSPobj _obj;

	struct OSPwindow_s *_slv; /* Can be an OSPscene * */
	struct OSPwindow_s *_mtr; /* Is the OSPdisplay * if parent is root window */
	struct OSPwindow_s *_prv;
	struct OSPwindow_s *_nxt;

	OSPdisplay *_dpy;
	Window _wnd;

	int _x;
	int _y;
	int _w;
	int _h;
	int _msx;
	int _msy;
	uint16_t _btn;

#ifdef OSP_XDBE_SUPPORT
	XdbeBackBuffer _bbf;
#endif
	GC _gc;
} OSPwindow;

typedef struct OSPscene_s {
	OSPobj _obj;

	struct OSPwindow_s *_slv;
	struct OSPwindow_s *_mtr;
	struct OSPwindow_s *_prv;
	struct OSPwindow_s *_nxt;

	OSPdisplay *_dpy;
	Window _wnd;

	int _x;
	int _y;
	int _w;
	int _h;
	int _msx;
	int _msy;
	uint16_t _btn;

	struct OSPscelem_s *_mvw; /* First element in a model view */
	GLXContext _glc;
	int _fd;

	uint8_t _stat;
} OSPscene;

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

OSPctr *OSPScnCtr();
OSPscene *OSPScn(OSPwindow *, int, int, int, int, uint32_t);

OSPctr *OSPImgCtr();
OSPimage *OSPImg(void *, int, int);

void OSPBlit(void *, void *, int, int, int, int, unsigned int, unsigned int);

OSPimage *OSPImgLoad(void *, const char *, int *);
int OSPImgSave(OSPimage *, int);

#endif /* __OSPWIN_H__ */

