#include"OSPobj.h"

#define OSPPROCEEDXEVENT 2 /* Called by OSPWte, never call outside, it updates the _event field */
#define OSPREFRESH 3 /* Refresh all the window with subwindows */

static const char *event_names[] = { /* For debug */
	"Field 0",
	"Field 1",
	"KeyPress",
	"KeyRelease",
	"ButtonPress",
	"ButtonRelease",
	"MotionNotify",
	"EnterNotify",
	"LeaveNotify",
	"FocusIn",
	"FocusOut",
	"KeymapNotify",
	"Expose",
	"GraphicsExpose",
	"NoExpose",
	"VisibilityNotify",
	"CreateNotify",
	"DestroyNotify",
	"UnmapNotify",
	"MapNotify",
	"MapRequest",
	"ReparentNotify",
	"ConfigureNotify",
	"ConfigureRequest",
	"GravityNotify",
	"ResizeRequest",
	"CirculateNotify",
	"CirculateRequest",
	"PropertyNotify",
	"SelectionClear",
	"SelectionRequest",
	"SelectionNotify",
	"ColormapNotify",
	"ClientMessage",
	"MappingNotify",
};

typedef struct {
	Display *_display;
	XEvent _event;
	Atom _WM_message_type_protocol; /* Window Manager dependent */
	Atom _WM_message_delete; /* Window Manager dependent */
} OSPwndCommon;

typedef struct {
	Window _window;
	XImage _image; /* Client side image of the window */
	int _width, _height, _depth;
	OSPwndCommon *_connection;
	OSPwndCommon _common; /* Never use common fields directly */ 
} OSPwnd;

static void OSPwndMng(OSPobj *wnd, void *arg) {
	OSPobj *parent = arg;
	OSPwnd *winstruct;
	printf("Window Managing\n");

	if(!wnd) return;
	if(!wnd->_dat) return;
	winstruct = wnd->_dat;

	if(parent) {
		/* TODO, manage merging */
	}
	else {
		if(winstruct->_connection->_display) {
			while(wnd->_slv) OSPDEL(wnd->_slv); /* Delete all children before closing connection */
			
			XDestroyWindow(winstruct->_connection->_display, winstruct->_window);
			
			if(winstruct->_connection == &winstruct->_common) { /* This is the main window */
				XCloseDisplay(winstruct->_connection->_display);
					/* Close properly communication with X fonctions */
			}
		}
	}
		
	OSPMng(wnd, parent); /* Delete or move this window */
}

static void OSPProceedXEvent(OSPobj *wnd, void *arg) { /* Fetch and proceeds event */
	OSPwnd *winstruct;
	uint8_t *run = arg;
	printf("Window Triggering\n");

	if(!wnd || !run) return;
	if(!wnd->_dat) return;
	winstruct = wnd->_dat;

	if(winstruct->_connection == &winstruct->_common) { /* This is the main window */
			printf("Main window cnum = %d\n", wnd->_tfd);
		if(wnd->_tfd < 0) return;
		
		run[0] = recv(wnd->_tfd, run, 1, MSG_PEEK | MSG_DONTWAIT);
		/* If 0, connection lost since recent window manager don't forward
			decoration events though X connection (e.g. close button) */
		if(run[0]) { /* X connection is alive */
			printf("Connection alive\n");
			/* Fetching one element in event queue */
			XNextEvent(winstruct->_connection->_display, &winstruct->_connection->_event);
			run[0] = 2; /* Let's find the targetted window */
		}
		else { /* Forced to close, don't attempt to communicate with the server */
			printf("connection %d brutaly closed\n", wnd->_tfd); /* For debug */
			close(wnd->_tfd); /* Close properly communication */
			winstruct->_connection->_display = 0; /* Close it once for all
							to avoid other events handling or server access */
		}
	}
	
	switch(run[0]) {
	case 2:/* connection is alive search the targeted */
		if(winstruct->_window == winstruct->_connection->_event.xany.window) {
			/* This is the targeted window */
			printf("Append %s event on connection %d\n",
					event_names[winstruct->_connection->_event.type],
					wnd->_tfd); /* For debug */
			
			switch(winstruct->_connection->_event.type) {
			case Expose:
				winstruct->_width = winstruct->_connection->_event.xexpose.width;
				winstruct->_height = winstruct->_connection->_event.xexpose.height;
				printf("w = %d, h = %d\n", winstruct->_width, winstruct->_height);				
				break;
				
			case DestroyNotify: /* Will you please close your window */
				run[0] = 0; /* You are lucky to have a such polite x server */
				break;
				
			case ClientMessage:
				{
					Atom msgtype = winstruct->_connection->_event.xclient.message_type;
					if(msgtype == winstruct->_connection->_WM_message_type_protocol) {
						Atom msg = (Atom)winstruct->_connection->_event.xclient.data.l[0];
						
						if(msg == winstruct->_connection->_WM_message_delete) { /* Will you please close your window */
							run[0] = 0; /* You are lucky to have a such polite window manager */
						}
					}
				}
			default:;
			}
			
			if(!run[0]) OSPDEL(wnd);
		}
		else {
			/* This is not the targeted window */
					printf("Not targeted\n");
			OSPobj *next = wnd->_slv;
			if(next) {
					printf("There are chindren\n");
				do {
					OSPRUN(next, next->_trg, run);
					next = next->_nxt;
				} while((next != wnd->_slv) && (run[0] != 1));
			}

			if((run[0] == 2) && (winstruct->_connection == &winstruct->_common)) {
				/* If window is still not reached */
				printf("window not defined\n");
				winstruct->_connection->_event.xany.window = winstruct->_window;
				OSPRUN(wnd, wnd->_trg, run);
			}
		}
	case 1:
		return; /* Done then back to caller */
	default:
		OSPDEL(wnd); /* Delete immediately all X branch, user is notified by the run state */
	}
}

static void OSPRefreshWindow(OSPobj *wnd, void *arg) { /* Refresh window */
	printf("Window refreshing\n");
	/* TODO Copy image to window */
}

typedef enum {
	BLDWNDMAIN,
	BLDWNDSUB
} OSPbuildwindowtype;

static OSPbuildwindowtype OSPlastbuildwindowtype = BLDWNDSUB;

void OSPBuildWindow(OSPbuildwindowtype type) {
	size_t size = sizeof(OSPwnd) - ((type == BLDWNDSUB) ? sizeof(OSPwndCommon) : 0);
	OSPlastbuildwindowtype = type;
	printf("Window building\n");

	OSPRESET;
	OSPAdd(ADDSZE, &size);
	OSPADDFCT(OSPMNG, OSPwndMng);
	OSPADDFCT(OSPPROCEEDXEVENT, OSPProceedXEvent); /* Called by OSPWte, never call outside, */
	OSPADDFCT(OSPREFRESH, OSPRefreshWindow);
}

OSPobj *OSPAddWindow(OSPobj *parent, uint8_t *status, char *displayname) {
	OSPobj *ret;
	OSPwnd *parentwinstruct = 0;
	OSPwnd *winstruct = 0;
	printf("Window adding\n");
	
	if(!status) return 0;
	
	if(parent) {
		if(OSPlastbuildwindowtype == BLDWNDMAIN) return 0;
		
		parentwinstruct = parent->_dat;
		
		if(!parentwinstruct) {
			return 0;
		}
	}
	else {
		if(OSPlastbuildwindowtype == BLDWNDSUB) return 0;
	}
	
	ret = OSPADDOBJ(parent);
	winstruct = ret->_dat;
	
	if(parent) {
		winstruct->_connection = parentwinstruct->_connection;
	}
	else {
		Display *display = 0;
		int screen;
		Window wnd;
		XVisualInfo vinfo;
		XSetWindowAttributes attr;
		
		winstruct->_connection = &winstruct->_common;
		
		if(!(display = XOpenDisplay(displayname))) {
			/* Display not found */
			OSPMng(ret, 0); /* Erase created object */
			return 0;
		}
		winstruct->_connection->_display = display;

		screen = XDefaultScreen(display);

//		winstruct->_connection->_WM_message_type_protocol =
//			XInternAtom(display, "WM_PROTOCOLS", 1);
//		winstruct->_connection->_WM_message_delete =
//			XInternAtom(display, "WM_DELETE_WINDOW", 1);

		winstruct->_depth = 32;
		if(!XMatchVisualInfo(display, screen, 32, TrueColor, &vinfo)) {
			winstruct->_depth = 24;
	
			if(!XMatchVisualInfo(display, screen, 24, TrueColor, &vinfo)) {
				/* No proper color depth available */
				XCloseDisplay(winstruct->_connection->_display); /* Close X communication */
				winstruct->_connection->_display = 0;
				OSPMng(ret, 0); /* Erase created object */
				return 0;
			}
		}

		attr.colormap = XCreateColormap(display, DefaultRootWindow(display), vinfo.visual, AllocNone);
		attr.border_pixel = 0;
		attr.background_pixel = 0x80000000;

		wnd = XCreateWindow(display, DefaultRootWindow(display), 0, 0, 100, 100, 0, vinfo.depth,
			InputOutput, vinfo.visual, CWColormap | CWBorderPixel | CWBackPixel, &attr);
		winstruct->_window = wnd;
		winstruct->_width = 100;
		winstruct->_height = 100;
		
		XFreeColormap(display, attr.colormap);
		XSelectInput(display, wnd, ExposureMask | ButtonPressMask | KeyPressMask);
		XMapWindow(display, wnd);
		XFlush(display);
		
		printf("common offset = %ld\n", (void *) winstruct->_connection - (void *) ret);

		ret->_tfd = -1;
		OSPTRG(ret, XConnectionNumber(display), status,
			OSPPROCEEDXEVENT, EPOLLIN | EPOLLHUP); /* Set the trigger function */
	}
	
	return ret;
}

int main(int argc, char* argv[])
{
	uint8_t run1 = 1;
	uint8_t run2 = 1;
	
	OSPBuildWindow(BLDWNDMAIN);
	OSPobj *window1 = OSPAddWindow(0, &run1, 0);
	OSPobj *window2 = OSPAddWindow(0, &run2, 0);
	
	while(run1 || run2) {
		OSPobj *append = OSPWte(0); /* -1 = always */
	}
	
	OSPFREEALL;
	
	return 0;
}