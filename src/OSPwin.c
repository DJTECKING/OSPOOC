#include"OSPlib.h"

void OSPDpyHdl(OSPobj **obj) {
	OSPdisplay *dpy = (OSPdisplay *) obj[0];
	int eventnb;
	unsigned char idx;
	char buf;

	/* Screen is about to be destroyed */
	if(!obj[0]->_buf) {
		while(dpy->_slv) {
			OSPFre(&dpy->_slv->_obj);
		}

		if(dpy->_dpy) {
			XFreeColormap(dpy->_dpy, dpy->_atr.colormap);
			XCloseDisplay(dpy->_dpy);
		}

		OSPTrg(0 , ConnectionNumber(dpy->_dpy), 0);

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

	eventnb = XPending(dpy->_dpy);

	while(eventnb--) {
		XNextEvent(dpy->_dpy, &dpy->_lst);

		OSPrint(1, "OSPDpyHdl : Event %d happened", eventnb);

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
								OSPFre(obj[0]);
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

	if(!(ret->_dpy = XOpenDisplay(dpyname))) {
		/* Display not found */
		OSPrint(0, "OSPdisplay : Unable to connect to display");
		OSPFre(&ret->_obj);
		return 0;
	}

	ret->_scn = XDefaultScreen(ret->_dpy);
	
	if(!XMatchVisualInfo(ret->_dpy, ret->_scn, 32, TrueColor, &ret->_vfo)) {
		OSPrint(1, "OSPdisplay : 32 bits color not alloed");
		if(!XMatchVisualInfo(ret->_dpy, ret->_scn, 24, TrueColor, &ret->_vfo)) {
			/* No proper color depth available */
			OSPrint(0, "OSPdisplay : Screen not compatible with 32 or 24 bits true color");
			OSPFre(&ret->_obj);
			return 0;
		}
	}

	ret->_atr.colormap = XCreateColormap(ret->_dpy,
									DefaultRootWindow(ret->_dpy),
									ret->_vfo.visual, AllocNone);
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

	XFlush(ret->_dpy);

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
						wnd->_msx, wnd->_msy, wnd->_wnd,
						XConnectionNumber(wnd->_dpy->_dpy));
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

OSPctr *OSPWndCtr() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	ret = OSPCtr(0, 3, sizeof(OSPwindow), OSPWndHdl);

	ret->_fct[OSPWND_EVENT] = OSPwnd_event;
	ret->_fct[OSPWND_GETKEY] = OSPwnd_getkey;
	ret->_fct[OSPWND_GETBTN] = OSPwnd_getbtn;

	return ret;
}

OSPwindow *OSPWnd(OSPobj *master, char *wndname, int x, int y,
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

	ret->_mtr = mtr;
	ret->_dpy = dpy;
	ret->_wnd = XCreateWindow(dpy->_dpy, prt, x, y, width, height, 0,
								dpy->_vfo.depth, InputOutput,
								dpy->_vfo.visual, CWColormap |
								CWBorderPixel | CWBackPixel |
								CWEventMask, &dpy->_atr);

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
	XFlush(dpy->_dpy);

	return ret;
}

