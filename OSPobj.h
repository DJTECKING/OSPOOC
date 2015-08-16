#ifndef __OSPOBJ_H__
#define __OSPOBJ_H__

#include<stdlib.h>
#include<stdio.h>
#include<termios.h>
#include<stdint.h>
#include<unistd.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<time.h>
/* #include<.h> */
/* #include".h" */

typedef struct OSPobj_s {
    struct OSPobj_s *_slv; /* Slave */
    struct OSPobj_s *_mtr; /* Master */
    struct OSPobj_s *_prv; /* Previous */
    struct OSPobj_s *_nxt; /* Next */
    void *_OSPdat; /* Data pointer */
    void (*_OSPFct[])(struct OSPobj_s *, void *); /* Function array */
} OSPobj;

typedef struct functdesc_s {
    uint64_t _functionid; /* Function id */
    void (*_function)(struct OSPobj_s *, void *); /* Function pointer */
} functdesc;

typedef enum {
    ADDOBJ,
    ADDFCT,
    ADDSZE
} OSPaddtask;

#define OSPMNG 0
#define OSPDUI 1

#define OSPDO(object, fctid, argptr) {(object)->_OSPFct[fctid]((object), (argptr));}
#define OSPADDFCT(fctid, fctptr) {functdesc fdesc = {(fctid), (fctptr)}; OSPAdd(ADDFCT, &fdesc);}
#define OSPRESET OSPAdd(ADDFCT, 0);
#define OSPFREEALL OSPAdd(ADDSZE, 0);

OSPobj *OSPAdd(OSPaddtask, void *);
void OSPMng(OSPobj *, void *);
void OSPDui(OSPobj *, void *);

#endif /* __OSPOBJ_H__ */
