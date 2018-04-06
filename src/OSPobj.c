#include"OSPlib.h"

static struct {
	OSPctr *_constructors;

	uint64_t _defbufsize;
	int _eventpoll;
} OSProot = {0, 4, -1};

/* Run an object method */
void OSPRun(OSPobj *obj, uint64_t fctid, ...) {
	va_list arg;

	if(!obj) {
		OSPrint(0, "OSPAdd : Object needed as argument");
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

/* Clear an object content */
void OSPClr(OSPobj *obj) {
	OSPbuf *buf;
	uint64_t dsz;

	if(!obj) {
		OSPrint(0, "OSPClr : No object given as argument");
		return;
	}

	if(!(buf = obj->_buf)) {
		OSPrint(0, "OSPClr : Cannot clear an already freed object");
		return;
	}

	dsz = buf->_ctr->_dsz - sizeof(OSPobj);
	memset(&obj[1], 0, dsz);
}

/* Get buffer starting point */
void *OSPGetBuffer(OSPbuf *buf) {
	return &buf->_fre[(buf->_fre[0] >> 7) + (buf->_fre[0] & 0x7E ? 2 : 1)];
}

/* Allocate a buffer */
OSPbuf *OSPBuf(OSPctr *ctr, uint64_t cnt) {
	uint64_t no;
	OSPbuf *ret;

	if(!ctr) {
		OSPrint(0, "OSPBuf : Constructor needed as argument");
		return 0;
	}
	
	/* 0x10 is 16 octet space : 8 for fre[0] and 8 for fre[0] always allocated */
	ret = calloc(1, sizeof(OSPbuf) +
					0x10 + (((cnt - 1) >> 3) & ~0x07LLU) +
							(ctr->_dsz * cnt));

	if(!ret) {
		OSPrint(0, "OSPBuf : Unable to allocate a new buffer : %s",
				strerror(errno));
		return 0;
	}

	ret->_fre[0] = (cnt << 1) | 1;
	ret->_ctr = ctr;
	if((ret->_nxt = ctr->_buf)) ret->_pcd = ret->_nxt->_pcd;
	ret->_pcd += cnt;

	for(no = 0; no < (cnt >> 6); no++) {
		ret->_fre[no + 1] = ~0LLU;
	}

	for(no = 0; no < ((ret->_fre[0] & 0x7ELLU) >> 1); no++) {
		ret->_fre[(cnt >> 6) + 1] |= 1LLU << no;
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
		OSPrint(0, "OSPCtr : object not aligned on %llu octets memory cells, "
				"size specified is %llu", sizeof(void *), dsz);
		return 0;
	}

	/* Checking args */
	if(mtr) {
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

		if(!hdl) hdl = mtr->_hdl;
	}
	else {
		mtr = (OSPctr *) &OSProot;

		if(dsz < sizeof(OSPobj)) {
			OSPrint(0, "OSPCtr : %llu octets specified while minimum %llu required",
					dsz, sizeof(OSPobj));
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
				if(!(buf->_fre[no + 1] & (1LLU << flag))) {
					OSPobj *obj = (OSPobj *) element;

					obj->_buf = 0;
					buf->_ctr->_hdl(&obj);
				}

				element += buf->_ctr->_dsz;
			}
		}

		for(no = 0; no < ((cnt & 0x7E) >> 1); no++) {
			if(!(buf->_fre[(cnt >> 7) + 1] & (1LLU << no))) {
				OSPobj *obj = (OSPobj *) element;

					obj->_buf = 0;
					buf->_ctr->_hdl(&obj);
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
	uint64_t num;

	if(!obj) {
		OSPDtr(OSProot._constructors);
		return;
	}

//	printf("Libere un objet pour 0x%p\n", obj->_buf->_ctr);

	buf = obj->_buf;
	ctr = buf->_ctr;
	array = (off_t) OSPGetBuffer(buf);
	element = (off_t) obj;
	no = (element - array) / ctr->_dsz;
	fre = &buf->_fre[(no >> 6) + 1];
	flag = 1LLU << (no & 0x3F);

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
	num = no + (buf->_nxt ? buf->_nxt->_pcd : 0);

	/* Freed the lastest element of this constructor,
		searching the new one */
	flag >>= 1;
	num--;
	if((num + 2) == ctr->_cnt) {
		while(buf) {
			OSPbuf *nxt;

			while(flag) {
				if(!(fre[0] & flag)) {
					ctr->_cnt = num + 1;
					return;
				}

				flag >>= 1;
				num--;
			}

			fre--;
			num--;

			while(fre > buf->_fre) {
				if(fre[0] != ~0LLU) {
					/* Do a dichotomy here */
					flag = 1LLU << 63;
					while(fre[0] & flag) {
						flag >>= 1;
						num--;
					}

					ctr->_cnt = num + 1;
					return;
				}

				fre--;
				num -= 64;
			}

			/* The pool is totally free, destroy it */
//			printf("Liberer le buffer\n");
			nxt = buf->_nxt;
			buf->_nxt = 0; /* Cut OSPFreeBuf recursivity */
			ctr->_buf = nxt;
			OSPFreeBuf(buf);
			buf = nxt;
			if(buf) {
				no = (buf->_fre[0] >> 1) - 1;
				fre = &buf->_fre[(no >> 6) + 1];
				flag = 1LLU << (no & 0x3F);
			}
		}
	}
}

/* Add an object */
OSPobj *OSPAdd(OSPctr *ctr) {
	OSPobj *ret = 0;
	OSPbuf *buf;
	uint64_t num;

	if(!ctr) {
		OSPrint(0, "OSPAdd : Constructor needed as argument");
		return 0;
	}

	if(ctr->_vrt) {
		OSPrint(0, "OSPAdd : Can't instanciate a virtual class");
		return 0;
	}

	for(buf = ctr->_buf; buf; buf = buf->_nxt) {
		uint64_t no;
		uint64_t cnt = buf->_fre[0];
		uint8_t *element;
		uint8_t mflag;

		if(!(cnt & 1)) continue;

		cnt >>= 1;
		element = OSPGetBuffer(buf);
		num = buf->_nxt ? buf->_nxt->_pcd : 0;

		/* Find a "free" cell with at least one bit free */
		for(no = 0; no < (cnt >> 6); no++) {
			if(buf->_fre[no + 1]) break;
				element += ctr->_dsz * 64;
		}

		if(no == (cnt >> 6)) {
			if(!buf->_fre[no + 1]) {
				buf->_fre[0] &= ~1LLU;
				continue;
			}

			mflag = cnt & 0x3FLLU;
		}
		else mflag = 64;

		cnt = no + 1;
		num += no << 6;


		for(no = 0; no < mflag; no++) {
			if(buf->_fre[cnt] & (1LLU << no)) {
				ret = (OSPobj *) element;
				ret->_buf = buf;
				ret->_fct = ctr->_fct;

				buf->_fre[cnt] &= ~(1LLU << no);

				num += no + 1;
				if(num > ctr->_cnt) ctr->_cnt = num;
				return ret;
			}

			element += ctr->_dsz;
		}

		/* Finally found that buffer is full */
		buf->_fre[0] &= ~1LLU;
	}

	/* No free pool, creating a new one, twice the size of the precedent if exist */
	num = ctr->_buf ? ctr->_buf->_pcd : 0;
	ctr->_cnt = num + 1;
	ctr->_buf = OSPBuf(ctr, ctr->_buf ? ctr->_buf->_fre[0] : OSProot._defbufsize);
	ctr->_buf->_fre[1] &= ~1LLU;

	ret = (OSPobj *) OSPGetBuffer(ctr->_buf);
	ret->_buf = ctr->_buf;
	ret->_fct = ctr->_fct;

	return ret;
}

/* Get object number in the pool */
uint64_t OSPNum(OSPobj *obj) {
	OSPbuf *buf;
	OSPctr *ctr;
	off_t array;
	off_t element;
	uint64_t no;

	if(!obj) {
		OSPrint(0, "OSPNum : No object given as argument");
		return 0;
	}

	if(!(buf = obj->_buf)) {
		OSPrint(0, "OSPNum : Object already freed, can't get number");
		return 0;
	}

	ctr = buf->_ctr;
	array = (off_t) OSPGetBuffer(buf);
	element = (off_t) obj;
	no = (element - array) / ctr->_dsz;
	if(buf->_nxt) return no + buf->_nxt->_pcd;
	return no;
}

/* Get object from it's number in the pool */
OSPobj *OSPOid(OSPctr *ctr, uint64_t id) {
	OSPbuf *buf;

	if(!ctr) {
		OSPrint(0, "OSPOid : No constructor given as argument");
		return 0;
	}

	if(!(buf = ctr->_buf)) {
		OSPrint(1, "OSPOid : No object yet instanciated");
		return 0;
	}

	if(id > buf->_pcd) {
		OSPrint(1, "OSPOid : Id %ld out of object range", id);
		return 0;
	}

	while(buf) {
		if(!buf->_nxt) {
			uint8_t *element;

			if(buf->_fre[(id >> 6) + 1] & (1LLU << (id & 0x3FLLU))) {
//				OSPrint(1, "OSPOid : Object no %ld not yet instanciated or already freed", id);
				return 0;
			}

			element = OSPGetBuffer(buf);
			return (OSPobj *) &element[id * ctr->_dsz];
		}

		if(id >= buf->_nxt->_pcd) {
			if(id < buf->_pcd) {
				uint8_t *element;
				id -= buf->_nxt->_pcd;

				if(buf->_fre[(id >> 6) + 1] & (1LLU << (id & 0x3F))) {
//					OSPrint(1, "OSPOid : Object no %ld not yet instanciated or already freed",
//							id + buf->_nxt->_pcd);
					return 0;
				}

				element = OSPGetBuffer(buf);
				return (OSPobj *) &element[id * ctr->_dsz];
			}
		}

		buf = buf->_nxt;
	}

	OSPrint(0, "OSPOid : Internal error", id);
	return 0;
}

/*	Subscribes/Unsubscribes an object to events comming from an fd.
	Give an object as obj argument to subscribe it to fd.
	Give 0 as obj argument to unsubscribe anything to fd.
	events argument is a bitmask defined as follow :

	EPOLLIN
		The associated file is available for read(2) operations.

	EPOLLOUT
		The associated file is available for write(2) operations.

	EPOLLRDHUP (since Linux 2.6.17)
		Stream socket peer closed connection, or shut down writing half  of  connection.   (This
		flag  is  especially  useful  for writing simple code to detect peer shutdown when using
		Edge Triggered monitoring.)

	EPOLLPRI
		There is urgent data available for read(2) operations.

	EPOLLERR
		Error condition happened on the associated file descriptor.  epoll_wait(2)  will  always
		wait for this event; it is not necessary to set it in events.

	EPOLLHUP
		Hang  up happened on the associated file descriptor.  epoll_wait(2) will always wait for
		this event; it is not necessary to set it in events.  Note  that  when  reading  from  a
		channel  such  as  a  pipe or a stream socket, this event merely indicates that the peer
		closed its end of the channel.  Subsequent reads from the channel will return 0 (end  of
		file) only after all outstanding data in the channel has been consumed.

	EPOLLET
		Sets the Edge Triggered behavior for the associated file descriptor.  The default behav‐
		ior for epoll is Level Triggered.  See epoll(7) for more detailed information about Edge
		and Level Triggered event distribution architectures.

	EPOLLONESHOT (since Linux 2.6.2)
		Sets the one-shot behavior for the associated file descriptor.  This means that after an
		event is pulled out with epoll_wait(2) the associated file descriptor is internally dis‐
		abled  and  no other events will be reported by the epoll interface.  The user must call
		epoll_ctl() with EPOLL_CTL_MOD to rearm the file descriptor with a new event mask.

	EPOLLWAKEUP (since Linux 3.5)
		If EPOLLONESHOT and EPOLLET are clear and the process has the CAP_BLOCK_SUSPEND capabil‐
		ity,  ensure that the system does not enter "suspend" or "hibernate" while this event is
		pending or being processed.  The event is considered as being "processed" from the  time
		when  it  is returned by a call to epoll_wait(2) until the next call to epoll_wait(2) on
		the same epoll(7) file descriptor, the closure of that file descriptor, the  removal  of
		the  event  file  descriptor  with EPOLL_CTL_DEL, or the clearing of EPOLLWAKEUP for the
		event file descriptor with EPOLL_CTL_MOD.  See also BUGS.

	EPOLLEXCLUSIVE (since Linux 4.5)
		Sets an exclusive wakeup mode for the epoll file descriptor that is  being  attached  to
		the  target  file  descriptor,  fd.   When a wakeup event occurs and multiple epoll file
		descriptors are attached to the same target file using EPOLLEXCLUSIVE, one  or  more  of
		the  epoll  file  descriptors  will receive an event with epoll_wait(2).  The default in
		this scenario (when EPOLLEXCLUSIVE is not set) is for  all  epoll  file  descriptors  to
		receive  an  event.  EPOLLEXCLUSIVE is thus useful for avoiding thundering herd problems
		in certain scenarios.

		If the same file descriptor is in multiple epoll instances, some with the EPOLLEXCLUSIVE
		flag,  and others without, then events will provided to all epoll instances that did not
		specify EPOLLEXCLUSIVE, and at least  one  of  the  epoll  instances  that  did  specify
		EPOLLEXCLUSIVE.

		The  following  values  may  be  specified  in conjunction with EPOLLEXCLUSIVE: EPOLLIN,
		EPOLLOUT, EPOLLWAKEUP, and EPOLLET.  EPOLLHUP and EPOLLERR can also  be  specified,  but
		this  is not required: as usual, these events are always reported if they occur, regard‐
		less of whether they are specified in events.   Attempts  to  specify  other  values  in
		events  yield  an error.  EPOLLEXCLUSIVE may be used only in an EPOLL_CTL_ADD operation;
		attempts to employ it with EPOLL_CTL_MOD yield an  error.   If  EPOLLEXCLUSIVE  has  set
		using  epoll_ctl(2), then a subsequent EPOLL_CTL_MOD on the same epfd, fd pair yields an
		error.  A call to epoll_ctl(2) that specifies EPOLLEXCLUSIVE in events and specifies the
		target  file descriptor fd as an epoll instance will likewise fail.  The error in all of
		these cases is EINVAL.

	(Informations taken from man 2 epoll)

	Once object is subscribed to an fd, 
*/
int OSPTrg(OSPobj *obj, int fd, uint32_t events) {
	struct epoll_event event = {events, {obj}};
	
	if(OSProot._eventpoll < 0) {
		OSProot._eventpoll = epoll_create1(0);

		if(OSProot._eventpoll < 0) {
			OSPrint(0, "OSPTrg : Unable to create epoll fd : %s", strerror(errno));
			return -1;
		}
	}

	if(epoll_ctl(OSProot._eventpoll, obj ? EPOLL_CTL_ADD : EPOLL_CTL_DEL, fd, &event)) {
		OSPrint(0, "OSPTrg : Unable to %ssubscribe object to fd events : %s",
				obj ? "" : "un", strerror(errno));
		return -1;
	}

	return 0;
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

		timestat[0] = !ret->_buf;
	}
	else {
		timestat[0] = 0;
	}

	return ret;
}

