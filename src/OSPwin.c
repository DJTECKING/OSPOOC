#include"OSPlib.h"

void OSPDpyHdl(OSPobj **obj) {
	OSPdisplay *dpy = (OSPdisplay *) obj[0];
	unsigned char idx;
	char buf;

	/* Screen is about to be destroyed */
	if(!obj[0]->_buf) {
		int dpyfd = ConnectionNumber(dpy->_dpy);

		while(dpy->_slv) {
			OSPFre(&dpy->_slv->_obj);
		}

		if(dpy->_dpy) {
			XFreeColormap(dpy->_dpy, dpy->_atr.colormap);
			XCloseDisplay(dpy->_dpy);
		}

		OSPTrg(0, dpyfd, 0);

		return;
	}

	if((buf = recv(ConnectionNumber(dpy->_dpy), &buf, 1, MSG_PEEK)) < 0) {
		OSPrint(0, "OSPDpyHdl : Connection probing failed : %s", strerror(errno));
	}

	if(!buf) {
		OSPrint(0, "OSPDpyHdl : Connection closed, there will be memory leak");
	}

	if(buf < 1) {
		dpy->_dpy = 0;

		OSPFre(obj[0]);
		return;
	}

	while(XPending(dpy->_dpy)) {
		XNextEvent(dpy->_dpy, &dpy->_lst);

		switch(dpy->_lst.type) {
			case KeymapNotify:
				for(idx = 0; idx < 32; idx++) {
					dpy->_Keypad[idx + 32] = dpy->_Keypad[idx];
					dpy->_Keypad[idx] = dpy->_lst.xkeymap.key_vector[idx];
				}

				OSPrint(1, "OSPDpyHdl : Keymap event happened "
						"on connection %d", XConnectionNumber(dpy->_dpy));
				break;

			case ClientMessage:
				if(dpy->_lst.xclient.message_type == dpy->_WMm[0]) {
					if(((unsigned long) dpy->_lst.xclient.data.l[0]) == dpy->_WMm[1]) {
						if(dpy->_slv) {
							if(!dpy->_slv->_nxt) {
								OSPFre(&dpy->_obj);
								obj[0] = &dpy->_obj;
								return;
							}
							else {
								OSPRun(&dpy->_slv->_obj, OSPWND_EVENT, obj);
							}
						}
					}
				}

				break;

			default:
				if(dpy->_slv) {
					OSPRun(&dpy->_slv->_obj, OSPWND_EVENT, obj);
				}
		}
	}
}

OSPctr *OSPDpyCtr() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	return ret = OSPCtr(0, 0, sizeof(OSPdisplay), OSPDpyHdl);
}

OSPdisplay *OSPDpyOf(OSPwindow *wnd) {
	return wnd->_dpy;
}

OSPdisplay *OSPDpy(char *dpyname) {
	OSPdisplay *ret = (OSPdisplay *) OSPAdd(OSPDpyCtr());
	Drawable rtwnd;
	XdbeScreenVisualInfo *vfolst;
	XVisualInfo *vfomtc;

	int idx;
	int noret;
	int perf24 = -1;
	int perf = -1;
	int vfoidx24 = -1;
	int vfoidx = -1;

/*	Should we use it...
	char **extension_list = XListExtensions(dpy->_dpy, &noret);

	for(idx = 0; idx < noret; idx++) {
		printf("%d : %s\n",idx , extension_list[idx]);
	}

	XFreeExtensionList(extension_list);
*/

	if(!(ret->_dpy = XOpenDisplay(dpyname))) {
		/* Display not found */
		OSPrint(0, "OSPdisplay : Unable to connect to display");
		OSPFre(&ret->_obj);
		return 0;
	}

	noret = ret->_scn = XDefaultScreen(ret->_dpy);
	rtwnd = DefaultRootWindow(ret->_dpy);
	
	vfolst = XdbeGetVisualInfo(ret->_dpy, &rtwnd, &noret);
	
	if (!vfolst || (noret < 1) || (vfolst->count < 1)) {
		OSPrint(0, "OSPdisplay : Double buffer unable");
		if(vfolst) XdbeFreeVisualInfo(vfolst);
		OSPFre(&ret->_obj);
		return 0;
	}
	
	for(idx = 0; idx < noret; idx++) {
		switch(vfolst->visinfo[idx].depth) {
			case 24:
				vfoidx24 = idx;
				perf24 = vfolst->visinfo[idx].perflevel > perf24 ?
						vfolst->visinfo[idx].perflevel : perf24;
				break;
			case 32:
				vfoidx = idx;
				perf = vfolst->visinfo[idx].perflevel > perf ?
						vfolst->visinfo[idx].perflevel : perf;
			default:;
		}
	}
	
	if(vfoidx == -1) {
		if(vfoidx24 == -1) {
			OSPrint(0, "OSPdisplay : Screen not compatible with 32 or 24 bits depth");
			XdbeFreeVisualInfo(vfolst);
			OSPFre(&ret->_obj);
			return 0;
		}
	
		OSPrint(1, "OSPdisplay : 32 bits depth unavailable, using 24 bits depth");
		vfoidx = vfoidx24;
	}
	
	ret->_vfo.visualid = vfolst->visinfo[vfoidx].visual;
	ret->_vfo.screen = ret->_scn; 
	ret->_vfo.depth = vfolst->visinfo[vfoidx].depth;
	
	XdbeFreeVisualInfo(vfolst);
	
	vfomtc = XGetVisualInfo(ret->_dpy, VisualIDMask | VisualScreenMask | VisualDepthMask,
							&ret->_vfo, &noret);
	
	if(!vfomtc || (noret < 1)) {
		OSPrint(0, "OSPdisplay : Double buffer visual not mached");
		OSPFre(&ret->_obj);
		return 0;
	}
	
	memcpy(&ret->_vfo, vfomtc, sizeof(XVisualInfo));
	
	for(idx = 0; idx < noret; idx++) {
		XFree(&vfomtc[idx]);
	}

/*
	if(!XMatchVisualInfo(ret->_dpy, ret->_scn, 32, TrueColor, &ret->_vfo)) {
		OSPrint(1, "OSPdisplay : 32 bits color not alloed");
		if(!XMatchVisualInfo(ret->_dpy, ret->_scn, 24, TrueColor, &ret->_vfo)) {
*/			/* No proper color depth available */
/*			OSPrint(0, "OSPdisplay : Screen not compatible with 32 or 24 bits true color");
			OSPFre(&ret->_obj);
			return 0;
		}
	}
*/

	ret->_atr.colormap = XCreateColormap(ret->_dpy, rtwnd, ret->_vfo.visual, AllocNone);
	ret->_atr.border_pixel = 0;
	ret->_atr.event_mask =	KeyPressMask | /* Keyboard down events wanted */
							KeyReleaseMask | /* Keyboard up events wanted */
							ButtonPressMask | /* Pointer button down events wanted */
							ButtonReleaseMask | /* Pointer button up events wanted */
							EnterWindowMask | /* Pointer window entry events wanted */
							LeaveWindowMask | /* Pointer window leave events wanted */
							PointerMotionMask | /* Pointer motion events wanted */
							PointerMotionHintMask | /* Pointer motion hints wanted */
							Button1MotionMask | /* Pointer motion while button 1 down */
							Button2MotionMask | /* Pointer motion while button 2 down */
							Button3MotionMask | /* Pointer motion while button 3 down */
							Button4MotionMask | /* Pointer motion while button 4 down */
							Button5MotionMask | /* Pointer motion while button 5 down */
							ButtonMotionMask | /* Pointer motion while any button down */
							KeymapStateMask | /* Keyboard state wanted at window entry and focus in */
							ExposureMask; /* Any exposure wanted */

							/* VisibilityChangeMask | Any change in visibility wanted */
							/* StructureNotifyMask | Any change in window structure wanted */
							/* ResizeRedirectMask | Redirect resize of this window */
							/* SubstructureNotifyMask | Substructure notification wanted */
							/* SubstructureRedirectMask | Redirect structure requests on children */
							/* FocusChangeMask | Any change in input focus wanted */
							/* PropertyChangeMask | Any change in property wanted */
							/* ColormapChangeMask | Any change in colormap wanted */
							/* OwnerGrabButtonMask; Automatic grabs should activate with owner_events set to True */

	ret->_WMm[0] = XInternAtom(ret->_dpy, "WM_PROTOCOLS", 1);
	ret->_WMm[1] = XInternAtom(ret->_dpy, "WM_DELETE_WINDOW", 1);

	ret->_slv = 0;

	OSPTrg(&ret->_obj, ConnectionNumber(ret->_dpy),
			EPOLLIN | EPOLLRDHUP);

	return ret;
}

void OSPWndHdl(OSPobj **obj) {
	OSPwindow *wnd = (OSPwindow *) obj[0];

	if(wnd->_mtr->_slv == wnd) {
		if(wnd->_nxt) wnd->_mtr->_slv = wnd->_nxt;
		else wnd->_mtr->_slv = wnd->_prv;
	}

	if(wnd->_nxt) wnd->_nxt->_prv = wnd->_prv;
	if(wnd->_prv) wnd->_prv->_nxt = wnd->_nxt;
	while(wnd->_slv) OSPFre(&wnd->_slv->_obj);

	XdbeDeallocateBackBufferName(wnd->_dpy->_dpy, wnd->_bbf);
	XFreeGC(wnd->_dpy->_dpy, wnd->_gc);
	XUnmapWindow(wnd->_dpy->_dpy, wnd->_wnd);
	XDestroyWindow(wnd->_dpy->_dpy, wnd->_wnd);
	XFlush(wnd->_dpy->_dpy);
}

void OSPwnd_event(OSPobj *obj, va_list arg) {
	char buffer[20];
	KeySym key;
	OSPobj **ret = va_arg(arg, OSPobj **);
	OSPwindow *wnd = (OSPwindow *) obj;

	if(wnd->_dpy->_lst.xany.window != wnd->_wnd) {
		OSPobj *lstret = ret[0];

		if(wnd->_slv) {
			OSPRun(&wnd->_slv->_obj, OSPWND_EVENT, ret);
		}

		if(ret[0] != lstret) return;

		if(wnd->_nxt) {
			OSPRun(&wnd->_nxt->_obj, OSPWND_EVENT, ret);
		}

		return;
	}

	ret[0] = obj;

	switch(wnd->_dpy->_lst.type) {
		case KeyPress:
			XLookupString(&wnd->_dpy->_lst.xkey, buffer, 20, &key, 0);
			OSPrint(1, "OSPwnd_event : '%s' pressed "
						"on window %d on connection %d",
						buffer, wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case KeyRelease:
			XLookupString(&wnd->_dpy->_lst.xkey, buffer, 20, &key, 0);
			OSPrint(1, "OSPwnd_event : '%s' released "
						"on window %d on connection %d",
						buffer, wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case ButtonPress:
		case ButtonRelease:
			wnd->_btn = (wnd->_btn << 8) | (wnd->_btn & 0x00FF);

			if(wnd->_dpy->_lst.type == ButtonPress) {
				wnd->_btn |= 1 << (wnd->_dpy->_lst.xbutton.button - 1);
			}
			else{
				wnd->_btn &= ~(1 << (wnd->_dpy->_lst.xbutton.button - 1));
			}

			OSPrint(1, "OSPwnd_event : button state is now '%c%c%c%c%c' "
						"on window %d on connection %d",
						wnd->_btn & (Button5Mask / Button1Mask) ? '-' : '_',
						wnd->_btn & (Button4Mask / Button1Mask) ? '-' : '_',
						wnd->_btn & (Button3Mask / Button1Mask) ? '-' : '_',
						wnd->_btn & (Button2Mask / Button1Mask) ? '-' : '_',
						wnd->_btn & (Button1Mask / Button1Mask) ? '-' : '_',
						wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case MotionNotify:
			wnd->_msx = wnd->_dpy->_lst.xmotion.x;
			wnd->_msy = wnd->_dpy->_lst.xmotion.y;
			wnd->_dpy->_msx = wnd->_dpy->_lst.xmotion.x_root;
			wnd->_dpy->_msy = wnd->_dpy->_lst.xmotion.y_root;

			OSPrint(1, "OSPwnd_event : Pointer moved at %d %d "
						"on window %d on connection %d",
						wnd->_msx, wnd->_msy, wnd->_wnd,
						XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case EnterNotify:
			wnd->_msx = wnd->_dpy->_lst.xcrossing.x;
			wnd->_msy = wnd->_dpy->_lst.xcrossing.y;
			wnd->_dpy->_msx = wnd->_dpy->_lst.xcrossing.x_root;
			wnd->_dpy->_msy = wnd->_dpy->_lst.xcrossing.y_root;

			OSPrint(1, "OSPwnd_event : Pointer came in at %d %d "
						"on window %d on connection %d",
						wnd->_msx, wnd->_msy, wnd->_wnd,
						XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case LeaveNotify:
			wnd->_msx = -1;
			wnd->_msy = -1;
			wnd->_dpy->_msx = wnd->_dpy->_lst.xcrossing.x_root;
			wnd->_dpy->_msy = wnd->_dpy->_lst.xcrossing.y_root;

			OSPrint(1, "OSPwnd_event : Pointer moved out "
						"of window %d on connection %d",
						wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case ClientMessage:
			if(wnd->_dpy->_lst.xclient.message_type == wnd->_dpy->_WMm[0]) {
				if(((unsigned long) wnd->_dpy->_lst.xclient.data.l[0]) == wnd->_dpy->_WMm[1]) {
					OSPFre(&wnd->_obj);
				}
			}

			OSPrint(1, "OSPwnd_event : Window %d destroyed "
						"on connection %d", wnd->_wnd,
						XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case FocusIn:
			OSPrint(1, "OSPwnd_event : Window %d on connection %d "
						"got focused", wnd->_wnd,
						XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case FocusOut:
			OSPrint(1, "OSPwnd_event : Window %d on connection %d "
						"lost focus", wnd->_wnd,
						XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case Expose:
			OSPrint(1, "OSPwnd_event : Window %d on connection %d "
						"got exposed", wnd->_wnd,
						XConnectionNumber(wnd->_dpy->_dpy));
			break;

		case DestroyNotify:
		case GraphicsExpose:
		case NoExpose:
		case CirculateRequest:
		case ConfigureRequest:
		case MapRequest:
		case ResizeRequest:
		case CirculateNotify:
		case ConfigureNotify:
		case CreateNotify:
		case GravityNotify:
		case MapNotify:
		case MappingNotify:
		case ReparentNotify:
		case UnmapNotify:
		case VisibilityNotify:
		case ColormapNotify:
		case PropertyNotify:
		case SelectionClear:
		case SelectionNotify:
		case SelectionRequest:
		default:
			OSPrint(1, "OSPwnd_event : Happened unmanaged event "
						"on window %d on connection %d",
						wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
	}
}

void OSPwnd_getkey(OSPobj *obj, va_list arg) {
	uint8_t *ret = va_arg(arg, uint8_t *);
	OSPwindow *wnd = (OSPwindow *) obj;
	uint8_t it = 32;

	while(it--) {
		ret[it + 32] = wnd->_dpy->_Keypad[it + 32];
		ret[it] = wnd->_dpy->_Keypad[it];

		wnd->_dpy->_Keypad[it + 32] = ret[it];
	}

	OSPrint(1, "OSPwnd_getkey : Got key from "
				"connection %d by window %d",
				wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
}

void OSPwnd_getbtn(OSPobj *obj, va_list arg) {
	uint16_t *ret = va_arg(arg, uint16_t *);
	OSPwindow *wnd = (OSPwindow *) obj;

	ret[0] = wnd->_btn;
	wnd->_btn = (wnd->_btn << 8) | (wnd->_btn & 0x00FF);

	OSPrint(1, "OSPwnd_getbtn : Got key from "
				"window %d on connection %d",
				wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
}

void OSPwnd_swap(OSPobj *obj, va_list arg) {
	OSPwindow *wnd = (OSPwindow *) obj;
	XdbeSwapInfo swpifo;

	swpifo.swap_window = wnd->_wnd;
	swpifo.swap_action = XdbeUndefined;

	if(XdbeSwapBuffers(wnd->_dpy->_dpy, &swpifo, 1)) {
/*		OSPrint(1, "OSPwnd_swap : Window %d swapped "
					"on connection %d",
					wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy)); */
	}
	else {
		OSPrint(1, "OSPwnd_swap : Unable to swap window %d "
					"on connection %d",
					wnd->_wnd, XConnectionNumber(wnd->_dpy->_dpy));
	}
}

OSPctr *OSPWndCtr() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	ret = OSPCtr(0, 4, sizeof(OSPwindow), OSPWndHdl);

	ret->_fct[OSPWND_EVENT] = OSPwnd_event;
	ret->_fct[OSPWND_GETKEY] = OSPwnd_getkey;
	ret->_fct[OSPWND_GETBTN] = OSPwnd_getbtn;
	ret->_fct[OSPWND_SWAP] = OSPwnd_swap;

	return ret;
}

OSPwindow *OSPWnd(void *master, char *wndname, int x, int y,
					int width, int height, uint32_t back) {
	OSPdisplay *dpy;
	OSPwindow *mtr = (OSPwindow *) master;
	OSPwindow *ret;
	Window prt;

	/* If no connection specified, create a new local one */
	if(!mtr) mtr = (OSPwindow *) OSPDpy(0);
	if(!mtr) return 0;

	if(mtr->_obj._fct == OSPDpyCtr()->_fct) {
		/* Create a window */
		dpy = (OSPdisplay *) mtr;
		prt = DefaultRootWindow(dpy->_dpy);
	}
	else {
		/* Create a subwindow */
		if(mtr->_obj._fct != OSPWndCtr()->_fct) {
			return 0;
		}

		prt = mtr->_wnd;
		dpy = mtr->_dpy;
	}

	ret = (OSPwindow *) OSPAdd(OSPWndCtr());
	dpy->_atr.background_pixel = back;

	ret->_dpy = dpy;
	ret->_wnd = XCreateWindow(dpy->_dpy, prt, x, y, width, height, 0,
								dpy->_vfo.depth, InputOutput,
								dpy->_vfo.visual, CWColormap |
								CWBorderPixel | CWBackPixel |
								CWEventMask, &dpy->_atr);

	ret->_bbf = XdbeAllocateBackBufferName(dpy->_dpy, ret->_wnd, XdbeUndefined);

	if(mtr->_obj._fct == OSPDpyCtr()->_fct) {
		/* Initialize window */
		XWindowAttributes size;
		XSizeHints sizehints;

		/* Subscribe to window closing event */
		XSetWMProtocols(dpy->_dpy, ret->_wnd, dpy->_WMm, 2);

		/* Lock window size */
		sizehints.flags = PMinSize | PMaxSize;
		sizehints.min_width = width;
		sizehints.min_height = height;
		sizehints.max_width = width;
		sizehints.max_height = height;
		XSetWMNormalHints(dpy->_dpy, ret->_wnd, &sizehints);

		if(wndname) XStoreName(dpy->_dpy, ret->_wnd, wndname);

		ret->_gc = XCreateGC(dpy->_dpy, ret->_wnd, 0, 0);
		XGetWindowAttributes(dpy->_dpy, ret->_wnd, &size);

		ret->_x = size.x;
		ret->_y = size.y;
	}
	else {
		/* Initialize subwindow */
		ret->_x = x;
		ret->_y = y;
	}

	ret->_mtr = mtr;
	ret->_nxt = mtr->_slv;
	ret->_prv = mtr->_slv ? mtr->_slv->_prv : 0;
	if(ret->_nxt) ret->_nxt->_prv = ret;
	if(ret->_prv) ret->_prv->_nxt = ret;
	mtr->_slv = ret;
	while(mtr->_slv->_prv) mtr->_slv = mtr->_slv->_prv;

	XMapWindow(dpy->_dpy, ret->_wnd);

	return ret;
}

#define SHM_ATTR	0x01
#define SHM_ALLOC	0x02
#define SHM_ATTCH	0x04

void OSPImgHdl(OSPobj **obj) {
	OSPimage *img = (OSPimage *) obj[0];

	if(img->_stat & SHM_ATTCH) XShmDetach(img->_dpy->_dpy, &img->_shm);
	if(img->_stat & SHM_ALLOC) shmdt(img->_shm.shmaddr);
	if(img->_stat & SHM_ATTR) shmctl(img->_shm.shmid, IPC_RMID, 0);

/*	if(img->_img->data) free(img->_img->data); Notice that neither XCreateImage
							nor XInitImage allocate datas
							while XDestroyImage frees it */
	if(img->_img) XDestroyImage(img->_img);
}

void OSPimg_getdata(OSPobj *obj, va_list arg) {
	OSPimage *img = (OSPimage *) obj;
	uint32_t ***ret = va_arg(arg, uint32_t ***);

	ret[0] = img->_data;
}

OSPctr *OSPImgCtr() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	ret = OSPCtr(0, 1, sizeof(OSPimage), OSPImgHdl);

	ret->_fct[OSPIMG_GETDATA] = OSPimg_getdata;

	return ret;
}

OSPimage *OSPImg(void *from, int width, int height, uint8_t shared) {
	OSPimage *ret = (OSPimage *) OSPAdd(OSPImgCtr());
	char *datptr;
	int idx;

	if(from) {
		if(((OSPobj *) from)->_fct == OSPDpyCtr()->_fct) {
			ret->_dpy = (OSPdisplay *) from;
		}
		else {
			if(((OSPobj *) from)->_fct != OSPWndCtr()->_fct) {
				OSPrint(0, "OSPImg : Bad first argument, display or window needed");
				return 0;
			}

			ret->_dpy = ((OSPwindow *) from)->_dpy;
		}
	}
	else {
		OSPrint(0, "OSPImg : Absent first argument, display or window needed");
		return 0;
	}

	if(shared) {
		ret->_img = XShmCreateImage(ret->_dpy->_dpy, ret->_dpy->_vfo.visual,
									ret->_dpy->_vfo.depth, ZPixmap, 0, &ret->_shm,
									width, height);
	}

	if(ret->_img) {
		ret->_shm.shmid = shmget(IPC_PRIVATE, (height * ret->_img->bytes_per_line) +
								(width * sizeof(void *)), IPC_CREAT | 0666);

		if(ret->_shm.shmid < 0) {
			OSPrint(0, "OSPImg : Unable to get shared memory segment "
						"on connection %d : %s",
						XConnectionNumber(ret->_dpy->_dpy), strerror(errno));

			OSPFre(&ret->_obj);
			return 0;
		}

		ret->_stat = SHM_ATTR;
		ret->_shm.shmaddr = ret->_img->data = shmat(ret->_shm.shmid, 0, 0);

		if(ret->_shm.shmaddr == (void *) -1) {
			OSPrint(0, "OSPImg : Unable to allocate shared memory segment "
						"data on connection %d : %s",
						XConnectionNumber(ret->_dpy->_dpy), strerror(errno));

			OSPFre(&ret->_obj);
			return 0;
		}

		ret->_stat |= SHM_ALLOC;
		ret->_shm.readOnly = False;

		if(!XShmAttach(ret->_dpy->_dpy, &ret->_shm)) {
			OSPrint(0, "OSPImg : Unable to attach shared memory segment "
						"to connection %d",
						XConnectionNumber(ret->_dpy->_dpy));

			OSPFre(&ret->_obj);
			return 0;
		}

		ret->_stat |= SHM_ALLOC;
	}
	else {
		if(shared) {
			OSPrint(1, "OSPImg : Unable to use shared memory extension "
						"on connection %d",
						XConnectionNumber(ret->_dpy->_dpy));
		}

		ret->_img = XCreateImage(ret->_dpy->_dpy, ret->_dpy->_vfo.visual,
								ret->_dpy->_vfo.depth, ZPixmap, 0, 0,
								width, height, 32, 0);

		if(!ret->_img) {
			OSPrint(0, "OSPImg : Unable to create image on connection %d",
						XConnectionNumber(ret->_dpy->_dpy));
			OSPFre(&ret->_obj);
			return 0;
		}

		if(!XInitImage(ret->_img)) {
			OSPrint(0, "OSPImg : Unable to init image on connection %d",
						XConnectionNumber(ret->_dpy->_dpy));
			OSPFre(&ret->_obj);
			return 0;
		}
		ret->_img->data = malloc((height * ret->_img->bytes_per_line) +
								(width * sizeof(void *)));

		if(!ret->_img->data) {
			OSPrint(0, "OSPImg : Unable to allocate image on connection %d",
						XConnectionNumber(ret->_dpy->_dpy));
			OSPFre(&ret->_obj);
			return 0;
		}
	}

	ret->_data = (uint32_t **)
				&((uint8_t *) ret->_img->data)[height * ret->_img->bytes_per_line];
	datptr = ret->_img->data;

	for(idx = 0; idx < width; idx++) {
		ret->_data[idx] = (uint32_t *) datptr;
		datptr += ret->_img->bytes_per_line;
	}

	return ret;
}

void OSPImgBlit(void *orig, void *dest, int x_orig, int y_orig,
			int x_dest, int y_dest, unsigned int width, unsigned int height) {
	OSPimage *orig_as_image = (OSPimage *) orig;
	OSPwindow *orig_as_window = (OSPwindow *) orig;
	OSPimage *dest_as_image = (OSPimage *) dest;
	OSPwindow *dest_as_window = (OSPwindow *) dest;

	enum {
		image_to_image = 0,
		image_to_window = 1,
		window_to_image = 2,
		window_to_window = 3
	} mode = image_to_window;

	switch(mode) {
		case image_to_window:
			if(orig_as_image->_stat & SHM_ATTCH) {
				/* No error check since Xlib craches the process itself */
				XShmPutImage(dest_as_window->_dpy->_dpy, dest_as_window->_bbf,
							dest_as_window->_gc, orig_as_image->_img,
							x_orig, y_orig, x_dest, y_dest, width, height, True);
			}
			else {
				/* No error check since Xlib craches the process itself */
				XPutImage(dest_as_window->_dpy->_dpy, dest_as_window->_bbf,
							dest_as_window->_gc, orig_as_image->_img,
							x_orig, y_orig, x_dest, y_dest, width, height);
			}
		default:;
	}
}

#undef SHM_ATTR
#undef SHM_ALLOC
#undef SHM_ATTCH

