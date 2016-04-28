#ifndef __OSPOBJ_H__
#define __OSPOBJ_H__

#include"../include/OSPlib.h"

struct OSPobj_s;

typedef void (*OSPfct)(struct OSPobj_s *, va_list); // General method definition
typedef void (*OSPhdl)(struct OSPobj_s **); // General event handler including free

typedef struct OSPbuf_s {
	struct OSPbuf_s *_nxt; /* Next pool */
	struct OSPctr_s *_ctr; /* Constructor */
	uint64_t _fre[]; /* Bitewise array of free flags */
	/* Instances comes here after _fre */
} OSPbuf;

typedef struct OSPobj_s {
	OSPbuf *_buf; /* Pool containing this instance */
	OSPfct *_fct; /* Function pointer to constructor's to function array */
    /* Object data */
} OSPobj;

typedef struct OSPctr_s {
	struct OSPctr_s *_slv; /* Slave */
	struct OSPctr_s *_nxt; /* Next */
	OSPbuf *_buf; /* Pool list */
	uint64_t _fnb; /* Number of function */
	uint64_t _dsz; /* Data size multiple of sizeof(void *) */
	OSPhdl _hdl; /* Event handler */
	OSPfct _fct[]; /* Function array */
	/* Take care, function ID are absolute and positive, it is never relative to a derived object */
} OSPctr;

/* Add a constructor or define default pool size with args : (X, defpoolsize, 0, X) */
OSPctr *OSPCtr(OSPctr *, uint64_t, uint64_t, OSPhdl);
OSPobj *OSPAdd(OSPctr *); /* Allocate an object or free all memory with 0 as argument */
void OSPRun(OSPobj *, uint64_t, ...);
void OSPFre(OSPobj *); /* Runs ctr._hdl method with obj._buf field reseted if ctr._hld defined */

void OSPTrg(OSPobj *, int, uint32_t);
OSPobj *OSPWte(int *);

#endif /* __OSPOBJ_H__ */

