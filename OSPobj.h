#ifndef __OSPOBJ_H__
#define __OSPOBJ_H__

#include<time.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<unistd.h>
#include<termios.h>
#include<X11/Xlib.h>
#include<sys/stat.h>
#include<sys/epoll.h>
#include<sys/types.h>
/* #include<.h> */
/* #include".h" */

typedef struct OSPobj_s {
    struct OSPobj_s *_slv; /* Slave */
    struct OSPobj_s *_mtr; /* Master */
    struct OSPobj_s *_prv; /* Previous */
    struct OSPobj_s *_nxt; /* Next */
    void *_dat; /* Data pointer */
    int _tfd; /* File descriptor used as trigger source */
    uint64_t _trg; /* Trigger function id */
    void *_tpt; /* Trigger pointer argument */
    void (*_fct[])(struct OSPobj_s *, void *); /* Function array */
} OSPobj;

typedef struct {
    uint64_t _functionid; /* Function id */
    void (*_function)(struct OSPobj_s *, void *); /* Function pointer */
} functdesc;

typedef struct {
	OSPobj *_object; /* Object to be triggered */
    int _filedesc; /* file descriptor to use as trigger source */
    void *_pointer; /* Argument to be passed when triggering */
	uint64_t _functionid; /* Function id to run in object */
    uint32_t _events; /* epoll events option */
} trigdesc;

typedef enum {
    ADDOBJ,
    ADDFCT,
    ADDSZE
} OSPaddtask;

#define OSPMNG 0
#define OSPDUI 1

#define OSPRUN(object, fctid, argptr) {(object)->_fct[fctid]((object), (argptr));}
#define OSPFDT(object) ((object)->_dat)
#define OSPDEL(object) OSPRUN(object, OSPMNG, 0)

#define OSPADDOBJ(parent) OSPAdd(ADDOBJ, parent);
#define OSPADDFCT(fctid, fctptr) {functdesc fdesc = {(fctid), (fctptr)}; OSPAdd(ADDFCT, &fdesc);}
#define OSPADDSZE(typeorvariable) {size_t size = sizeof(typeorvariable); OSPAdd(ADDSZE, &size);}
#define OSPRESET OSPAdd(ADDFCT, 0);
#define OSPFREEALL OSPAdd(ADDSZE, 0);

#define OSPTRG(objptr, fd, fctid, event) {trigdesc tdesc = {(objptr), (fd), (fctid), (event)}; OSPTrg(&tdesc);}

void OSPMng(OSPobj *, void *);
void OSPDui(OSPobj *, void *);

OSPobj *OSPAdd(OSPaddtask, void *);

void OSPTrg(trigdesc *);
void OSPWte(int); /* -1 = always */

#endif /* __OSPOBJ_H__ */
