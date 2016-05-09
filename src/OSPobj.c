#include"../include/OSPlib.h"

static struct {
	OSPctr *_constructors;

	uint64_t _defbufsize;
	int _eventpoll;
} OSProot = {0, 4, -1};

/* Run an object method */
void OSPRun(OSPobj *obj, uint64_t fctid, ...) {
	va_list arg;

	if(!obj) {
		OSPrint(0, "OSPRun : Object not specified");
		return;
	}

	if(!obj->_fct[fctid]) {
		OSPrint(1, "OSPRun : Function %lu not specified", fctid);
		return;
	}

	va_start(arg, fctid);
	obj->_fct[fctid](obj, arg);
	va_end(arg);
}

/* Get buffer starting point */
void *OSPGetBuffer(OSPbuf *buf) {
	return &buf->_fre[(buf->_fre[0] >> 7) + (buf->_fre[0] & 0x7E ? 2 : 1)];
}

/* Allocate a buffer */
OSPbuf *OSPBuf(OSPctr *ctr, uint64_t cnt) {
	uint64_t no;
	OSPbuf *ret = calloc(1, sizeof(OSPbuf) +
							((cnt + 1) >> 3) +
							(ctr->_dsz * cnt));

	if(!ret) {
		OSPrint(0, "OSPBuf : Unable to allocate a new buffer : %s",
				strerror(errno));
		return 0;
	}

	ret->_fre[0] = (cnt << 1) | 1;
	ret->_ctr = ctr;
	ret->_nxt = ctr->_buf;

	for(no = 0; no < (cnt >> 7); no++) {
		ret->_fre[no + 1] = ~0;
	}

	for(no = 0; no < ((ret->_fre[0] & 0x7E) >> 1); no++) {
		ret->_fre[(cnt >> 7) + 1] |= 1 << no;
	}
	
	return ret;
}

/* Create a constructor */
OSPctr *OSPCtr(OSPctr *mtr, uint64_t fnb, uint64_t dsz, OSPhdl hdl) {
	OSPctr *ret;

	/* Modify cnt element allocated by default if needed */
	if(!dsz) {
		OSPrint(1, "OSPCtr : Changing default pool alloc size "
				"for %llu elements", fnb);
		OSProot._defbufsize = fnb;
		return 0;
	}

	/* Return error if not aligned object */
	if(dsz % sizeof(void *)) {
		OSPrint(0, "OSPCtr : object not aligned on %llu memory cells, "
				"size specified is %llu", sizeof(void *), sizeof(OSPobj));
		return 0;
	}

	/* Checking args */
	if(!mtr) {
		mtr = (OSPctr *) &OSProot;

		if(dsz < sizeof(OSPobj)) {
			OSPrint(0, "OSPCtr : %llu octets specified while minimum %llu required",
					dsz, sizeof(OSPobj));
			return 0;
		}
	}
	else {
		if(fnb < mtr->_fnb) {
			OSPrint(0, "OSPCtr : Parent constructor counts %llu function "
					"while %llu functions are specified", mtr->_fnb, fnb);
			return 0;
		}

		if(dsz < mtr->_dsz) {
			OSPrint(0, "OSPCtr : Parent constructor specify %llu octets "
					"while %llu octets are specified", mtr->_dsz, dsz);
			return 0;
		}
	}

	/* Allocation */
	ret = (OSPctr *) calloc(1, sizeof(OSPctr) + (sizeof(void *) * fnb));

	if(!ret) {
		OSPrint(0, "OSPCtr : Unable to allocate a new constructor : %s",
				strerror(errno));
		return 0;
	}

	ret->_fnb = fnb;
	ret->_dsz = dsz;
	ret->_hdl = hdl;
	ret->_buf = OSPBuf(ret, OSProot._defbufsize); /* Is this realy good to do this here ? */
	/* Since that's automatically done when adding an object */
	ret->_nxt = mtr->_slv;
	mtr->_slv = ret;

	/* Copying parent's methods if needed */
	if(mtr != (OSPctr *) &OSProot._constructors) {
		memcpy(ret->_fct, mtr->_fct, mtr->_fnb * sizeof(OSPfct));
	}

	return ret;
}

/* Free pool list */
void OSPFreeBuf(OSPbuf *buf) {
	/* Recursive free */
	if(buf->_nxt) {
		OSPFreeBuf(buf->_nxt);
	}

	/* If no handler, just free it directly */
	if(buf->_ctr->_hdl) {
		uint64_t no;
		uint64_t cnt = buf->_fre[0];
		uint8_t *element = OSPGetBuffer(buf);

		for(no = 0; no < (cnt >> 7); no++) {
			int flag;

			if(!(~buf->_fre[no + 1])) {
				element += buf->_ctr->_dsz * 64;
				continue;
			}

			for(flag = 0; flag < 64; flag++) {
				if(buf->_fre[no + 1] & (1 << flag)) {
					OSPobj *obj = (OSPobj *) element;
					obj->_buf = 0;
					buf->_ctr->_hdl(&obj);
				}

				element += buf->_ctr->_dsz;
			}
		}

		for(no = 0; no < ((cnt & 0x7E) >> 1); no++) {
			if(buf->_fre[(cnt >> 7) + 1] & (1 << no)) {
				OSPobj *obj = (OSPobj *) element;

				if(obj->_buf) {
					buf->_ctr->_hdl(&obj);
				}

				obj->_buf = 0;
			}

			element += buf->_ctr->_dsz;
		}
	}

	free(buf);
}

/* Free constructors recursively */
void OSPDtr(OSPctr *ctr) {
	if(ctr->_slv) {
		OSPDtr(ctr->_slv);
	}

	if(ctr->_nxt) {
		OSPDtr(ctr->_nxt);
	}

	if(ctr->_buf) {
		OSPFreeBuf(ctr->_buf);
	}

	free(ctr);
}

/* Delete object */
void OSPFre(OSPobj *obj) {
	OSPbuf *buf;
	OSPctr *ctr;
	off_t array;
	off_t element;
	uint64_t no;
	uint64_t *fre;
	uint64_t flag;

	if(!obj) {
		OSPDtr(OSProot._constructors);
		return;
	}

	buf = obj->_buf;
	ctr = buf->_ctr;
	array = (off_t) OSPGetBuffer(buf);
	element = (off_t) obj;
	no = (element - array) / ctr->_dsz;
	fre = &buf->_fre[(no >> 6) + 1];
	flag = 1 << (no & 0x3F);

	if(fre[0] & flag) {
		OSPrint(0, "OSPFre : Double free at %p in pool %p of class %p at slot no %llu",
				obj, buf, ctr, no);
		return;
	}

	if(ctr->_hdl) {
		obj->_buf = 0;
		ctr->_hdl(&obj);
	}

	fre[0] |= flag;
	buf->_fre[0] |= 1;
}

/* Add an object */
OSPobj *OSPAdd(OSPctr *ctr) {
	OSPobj *ret = 0;
	OSPbuf *buf = 0;
	OSPbuf *nxt;

	if(!ctr) {
		return 0;
	}

	for(nxt = ctr->_buf; nxt; nxt = nxt->_nxt) {
		uint64_t no;
		uint64_t cnt = nxt->_fre[0];
		uint8_t *element = OSPGetBuffer(nxt);
		uint8_t mflag;

		if(!(cnt & 1)) {
			continue;
		}

		buf = nxt;

		/* Find a "free" cell with at least one bit free */
		for(no = 0; no < (cnt >> 7); no++) {
			if(!buf->_fre[no + 1]) {
				element += ctr->_dsz * 64;
				break;
			}
		}

		mflag = (no == (cnt >> 7)) ? ((cnt & 0x7E) >> 1) : 64;
		cnt = no + 1;

		for(no = 0; no < mflag; no++) {
			if(buf->_fre[cnt] & (1 << no)) {
				ret = (OSPobj *) element;
				ret->_buf = buf;
				ret->_fct = ctr->_fct;

				buf->_fre[(cnt >> 7) + 1] &= ~(1 << no);

				return ret;
			}

			element += ctr->_dsz;
		}

		nxt->_fre[0] &= ~1;
	}

	/* No free pool, creating a new one, twice the size of the precedent if exist */
	ctr->_buf = OSPBuf(ctr, buf ? ctr->_buf->_fre[0] : OSProot._defbufsize);
	ctr->_buf->_fre[1] &= ~1;

	ret = (OSPobj *) OSPGetBuffer(ctr->_buf);
	ret->_buf = ctr->_buf;
	ret->_fct = ctr->_fct;

	return ret;
}

void OSPTrg(OSPobj *obj, int fd, uint32_t events) {
	struct epoll_event event = {events, {obj}};
	
	if(OSProot._eventpoll < 0) {
		OSProot._eventpoll = epoll_create1(0);

		if(OSProot._eventpoll < 0) {
			OSPrint(0, "OSPTrg : Unable to create epoll fd : %s", strerror(errno));
			return;
		}
	}

	epoll_ctl(OSProot._eventpoll, obj ? EPOLL_CTL_ADD : EPOLL_CTL_DEL, fd, &event);
}

/* Returns the concerned object (if(timestat[0]) object has been freed) */
OSPobj *OSPWte(int *timestat) {
	struct epoll_event event;
	OSPobj *ret = 0;

	if(epoll_wait(OSProot._eventpoll, &event, 1, timestat[0])) {
		ret = event.data.ptr;

		if(ret->_buf->_ctr->_hdl) {
			ret->_buf->_ctr->_hdl(&ret);
		}

		if(ret->_buf) {
			timestat[0] = 0;
		}
		else {
			timestat[0] = 1;
		}
	}
	else {
		timestat[0] = 0;
	}

	return ret;
}

