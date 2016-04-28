#ifndef __OSPLIB_H__
#define __OSPLIB_H__

#include<time.h>
#include<errno.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdarg.h>
#include<unistd.h>
#include<string.h>
#include<termios.h>
#include<X11/Xlib.h>
#include<X11/Xutil.h>			/* For visual informations */
#include<X11/keysym.h>			/* For ??? */
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glxext.h>			/* For ??? */
#include<sys/stat.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
/* #include<.h> */

#include"../include/OSPobj.h"	/* General object implementation */
#include"../include/OSPutl.h"	/* General functions with no dependence */
#include"../include/OSPwin.h"	/* Graphical functions */
//	#include"OSPdev.h"			/* Generic devices, never implement as is */
//	#include"OSPsrv.h"			/* Server implementation */

#endif /* __OSPLIB_H__ */

