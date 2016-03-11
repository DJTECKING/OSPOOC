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
			/* TODO Find a way to free all Xdisplay stuff : */
//			struct _XDisplay
//			{
//				XExtData *ext_data;	/* hook for extension to hang data */
//				struct _XFreeFuncs *free_funcs; /* internal free functions */
//				int fd;			/* Network socket. */
//				int conn_checker;         /* ugly thing used by _XEventsQueued */
//				int proto_major_version;/* maj. version of server's X protocol */
//				int proto_minor_version;/* minor version of server's X protocol */
//				char *vendor;		/* vendor of the server hardware */
//					XID resource_base;	/* resource ID base */
//				XID resource_mask;	/* resource ID mask bits */
//				XID resource_id;	/* allocator current ID */
//				int resource_shift;	/* allocator shift to correct bits */
//				XID (*resource_alloc)(	/* allocator function */
//					struct _XDisplay*
//					);
//				int byte_order;		/* screen byte order, LSBFirst, MSBFirst */
//				int bitmap_unit;	/* padding and data requirements */
//				int bitmap_pad;		/* padding requirements on bitmaps */
//				int bitmap_bit_order;	/* LeastSignificant or MostSignificant */
//				int nformats;		/* number of pixmap formats in list */
//				ScreenFormat *pixmap_format;	/* pixmap format list */
//				int vnumber;		/* Xlib's X protocol version number. */
//				int release;		/* release of the server */
//				struct _XSQEvent *head, *tail;	/* Input event queue. */
//				int qlen;		/* Length of input event queue */
//				unsigned long last_request_read; /* seq number of last event read */
//				unsigned long request;	/* sequence number of last request. */
//				char *last_req;		/* beginning of last request, or dummy */
//				char *buffer;		/* Output buffer starting address. */
//				char *bufptr;		/* Output buffer index pointer. */
//				char *bufmax;		/* Output buffer maximum+1 address. */
//				unsigned max_request_size; /* maximum number 32 bit words in request*/
//				struct _XrmHashBucketRec *db;
//				int (*synchandler)(	/* Synchronization handler */
//					struct _XDisplay*
//					);
//				char *display_name;	/* "host:display" string used on this connect*/
//				int default_screen;	/* default screen for operations */
//				int nscreens;		/* number of screens on this server*/
//				Screen *screens;	/* pointer to list of screens */
//				unsigned long motion_buffer;	/* size of motion buffer */
//				unsigned long flags;	   /* internal connection flags */
//				int min_keycode;	/* minimum defined keycode */
//				int max_keycode;	/* maximum defined keycode */
//				KeySym *keysyms;	/* This server's keysyms */
//				XModifierKeymap *modifiermap;	/* This server's modifier keymap */
//				int keysyms_per_keycode;/* number of rows */
//				char *xdefaults;	/* contents of defaults from server */
//				char *scratch_buffer;	/* place to hang scratch buffer */
//				unsigned long scratch_length;	/* length of scratch buffer */
//				int ext_number;		/* extension number on this display */
//				struct _XExten *ext_procs; /* extensions initialized on this display */
//				/*
//				 * the following can be fixed size, as the protocol defines how
//				 * much address space is available. 
//				 * While this could be done using the extension vector, there
//				 * may be MANY events processed, so a search through the extension
//				 * list to find the right procedure for each event might be
//				 * expensive if many extensions are being used.
//				 */
//				Bool (*event_vec[128])(	/* vector for wire to event */
//					Display *	/* dpy */,
//					XEvent *	/* re */,
//					xEvent *	/* event */
//					);
//				Status (*wire_vec[128])( /* vector for event to wire */
//					Display *	/* dpy */,
//					XEvent *	/* re */,
//					xEvent *	/* event */
//					);
//				KeySym lock_meaning;	   /* for XLookupString */
//				struct _XLockInfo *lock;   /* multi-thread state, display lock */
//				struct _XInternalAsync *async_handlers; /* for internal async */
//				unsigned long bigreq_size; /* max size of big requests */
//				struct _XLockPtrs *lock_fns; /* pointers to threads functions */
//				void (*idlist_alloc)(	   /* XID list allocator function */
//					Display *	/* dpy */,
//					XID *		/* ids */,
//					int		/* count */
//					);
//				/* things above this line should not move, for binary compatibility */
//				struct _XKeytrans *key_bindings; /* for XLookupString */
//				Font cursor_font;	   /* for XCreateFontCursor */
//				struct _XDisplayAtoms *atoms; /* for XInternAtom */
//				unsigned int mode_switch;  /* keyboard group modifiers */
//				unsigned int num_lock;  /* keyboard numlock modifiers */
//				struct _XContextDB *context_db; /* context database */
//				Bool (**error_vec)(	/* vector for wire to error */
//					Display     *	/* display */,
//					XErrorEvent *	/* he */,
//					xError      *	/* we */
//					);
//				/*
//				 * Xcms information
//				 */
//				struct {
//				   XPointer defaultCCCs;  /* pointer to an array of default XcmsCCC */
//				   XPointer clientCmaps;  /* pointer to linked list of XcmsCmapRec */
//				   XPointer perVisualIntensityMaps;
//							  /* linked list of XcmsIntensityMap */
//				} cms;
//				struct _XIMFilter *im_filters;
//				struct _XSQEvent *qfree; /* unallocated event queue elements */
//				unsigned long next_event_serial_num; /* inserted into next queue elt */
//				struct _XExten *flushes; /* Flush hooks */
//				struct _XConnectionInfo *im_fd_info; /* _XRegisterInternalConnection */
//				int im_fd_length;	/* number of im_fd_info */
//				struct _XConnWatchInfo *conn_watchers; /* XAddConnectionWatch */
//				int watcher_count;	/* number of conn_watchers */
//				XPointer filedes;	/* struct pollfd cache for _XWaitForReadable */
//				int (*savedsynchandler)( /* user synchandler when Xlib usurps */
//					Display *	/* dpy */
//					);
//				XID resource_max;	/* allocator max ID */
//				int xcmisc_opcode;	/* major opcode for XC-MISC */
//				struct _XkbInfoRec *xkb_info; /* XKB info */
//				struct _XtransConnInfo *trans_conn; /* transport connection object */
//			};
//			XFree(winstruct->_connection->_display); /* Close it once for all */
			winstruct->_connection->_display = 0; /* to avoid other events handling or server access */
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

		/* Subscribe to window closing event */
		winstruct->_connection->_WM_message_type_protocol =
			XInternAtom(display, "WM_PROTOCOLS", 1);
		winstruct->_connection->_WM_message_delete =
			XInternAtom(display, "WM_DELETE_WINDOW", 1);
		XSetWMProtocols(display, wnd, &winstruct->_connection->_WM_message_type_protocol, 2);
		
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
