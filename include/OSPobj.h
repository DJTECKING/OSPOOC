#ifndef __OSPOBJ_H__
#define __OSPOBJ_H__

#define PRINT64		"%llu"
#define PRINT64_5	"%5llu"
#define PRINT64_8	"%8llu"

#define PRINT32		"%llu"
#define PRINT32_3	"%3llu"

#if (LONG_BIT == 64)
#undef PRINT64
#undef PRINT64_5
#undef PRINT64_8
#define PRINT64		"%lu"
#define PRINT64_5	"%5lu"
#define PRINT64_8	"%8lu"
#endif

#if (WORD_BIT == 64)
#undef PRINT64
#undef PRINT64_5
#undef PRINT64_8
#define PRINT64		"%u"
#define PRINT64_5	"%5u"
#define PRINT64_8	"%8u"
#endif

#if (LONG_BIT == 32)
#undef PRINT32
#undef PRINT32_3
#define PRINT32		"%lu"
#define PRINT32_3	"%3lu"
#endif

#if (WORD_BIT == 32)
#undef PRINT32
#undef PRINT32_3
#define PRINT32		"%u"
#define PRINT32_3	"%3u"
#endif

struct OSPobj_s;

typedef void (*OSPfct)(struct OSPobj_s *, va_list); // General method definition
typedef void (*OSPhdl)(struct OSPobj_s **); // General event handler including free

typedef struct OSPbuf_s {
	struct OSPbuf_s *_nxt; /* Next pool */
	struct OSPctr_s *_ctr; /* Constructor */
	uint64_t _pcd; /* Number of cumulated instance */
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
	uint64_t _cnt; /* Highest object id + 1 */
	uint64_t _fnb; /* Number of function */
	uint64_t _dsz; /* Data size multiple of sizeof(void *) */
	uint32_t _vrt; /* Is this a virtual class */
	OSPhdl _hdl; /* Event handler */
	OSPfct _fct[]; /* Function array */
	/* Take care, function ID are absolute and positive, it is never relative to a derived object */
} OSPctr;

/* To add a constructor use : mother class constructor, funcnum, datsize, OSPhdl hdl) */
/* To define default pool size use : X, defpoolsize, 0, X */
OSPctr *OSPCtr(OSPctr *, uint64_t, uint64_t, OSPhdl);
OSPobj *OSPAdd(OSPctr *); /* Allocate an object or free all memory with 0 as argument */
void OSPRun(OSPobj *, uint64_t, ...);
uint64_t OSPNum(OSPobj *); /* Get object number of a given constructor */
OSPobj *OSPOid(OSPctr *, uint64_t); /* Get object from it's number in the pool */
void OSPClr(OSPobj *obj); /* Clear an object content */
void OSPFre(OSPobj *); /* Runs ctr._hdl method with obj._buf field reseted if ctr._hld defined */
/* Free all memory if 0 as argument */

int OSPTrg(OSPobj *, int, uint32_t);
OSPobj *OSPWte(int *);

#endif /* __OSPOBJ_H__ */

