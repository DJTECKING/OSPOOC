#ifndef __OSPOBJ_H__
#define __OSPOBJ_H__

#include<stdlil.h>
#include<stdio.h>
#include<termios.h>
#include<stdint.h>
#include<unistd.h>
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
    void (*OSPFct[])(struct OSPobj *, void *); /* Function array */
} OSPobj;

typedef enum {
    ADDOBJ,
    ADDFCT,
    ADDSZE
} OSPaddtask;

#define OSPRESET OSPAdd(ADDFCT, 0);
#define OSPFREEALL OSPAdd(ADDSZE, 0);

OSPobj *OSPAdd(OSPaddtask task, void *arg);

#endif /* __OSPOBJ_H__ */