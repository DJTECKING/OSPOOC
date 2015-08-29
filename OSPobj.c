#include"OSPobj.h"

#define FUNCALLOCSIZE 1024

typedef struct functelement_s {
	struct functelement_s *_prv; /* Previous function container */
	struct functelement_s *_nxt; /* Next function container */
	uint64_t _elementnumber; /* Container number */
	functdesc _function; /* Container data */
} functelement;

static struct {
	OSPobj *_toplevels;
	int _eventpoll;
	struct epoll_event _event;
	
	functelement *_functqueue;
	functelement *_functend;
	uint64_t _elementnumber;
	size_t _datasize;

	OSPobj *_current;
} *OSProot = 0;

static void OSPMerge(OSPobj *master, OSPobj *slave) {
	slave->_mtr = master;
	
	if(master->_slv) {
		slave->_nxt = master->_slv;
		slave->_prv = master->_slv->_prv;
		if(slave->_nxt) slave->_nxt->_prv = slave;
		if(slave->_prv) slave->_prv->_nxt = slave;
	}
	else {
		slave->_nxt = slave;
		slave->_prv = slave;
	}

	master->_slv = slave;
}

static void OSPSplit(OSPobj *object) {
	if(!object->_mtr) return;
	
	/* Should never go in this */
	if((object->_nxt == object) || !object->_nxt) {
		if((object->_prv == object) || !object->_prv) {
			object->_nxt = object;
			object->_prv = object;
		}
		else {
			OSPobj *pointer = object;
			
			while(pointer->_prv) pointer = pointer->_prv;
			object->_nxt = pointer;
			pointer->_prv = object;
		}
	}
	else {
		if((object->_prv == object) || !object->_prv) {
			OSPobj *pointer = object;
			
			while(pointer->_nxt) pointer = pointer->_nxt;
			object->_prv = pointer;
			pointer->_nxt = object;
		}
	}
	
	object->_prv->_nxt = object->_nxt;
	object->_nxt->_prv = object->_prv;
	object->_mtr->_slv = (object->_nxt == object) ? 0 : object->_nxt;
//	object->_nxt = 0;
//	object->_prv = 0;
	object->_mtr = 0;
}

void OSPMng(OSPobj *obj, void *mtr) {
	OSPSplit(obj);
	
	if(mtr) {
		OSPMerge(mtr, obj);
	}
	else {
		while(obj->_slv) OSPDEL(obj->_slv);
		if(obj->_tfd >= 0) close(obj->_tfd);
		OSPSplit(obj);
		free(obj);
	}
}

void OSPDui(OSPobj *obj, void *arg) {
	printf("Object is located at %p\n", obj);
}

OSPobj *OSPAdd(OSPaddtask task, void *arg) {
	OSPobj *objarg = (OSPobj *) arg;
	functdesc *fctarg = (functdesc *) arg;
	size_t *datarg = (size_t *) arg;
	
	if(!OSProot) {
		OSProot = calloc(1, sizeof(*OSProot));
		
		OSProot->_eventpoll = epoll_create1(0);
		
		OSProot->_functqueue = calloc(FUNCALLOCSIZE, sizeof(functelement));
		OSProot->_functend = OSProot->_functqueue;
	}
	
	switch(task) {
	case ADDOBJ:
		if(!objarg) objarg = (OSPobj *) OSProot;
		
		{
			functelement *functcurrent = OSProot->_functqueue;
			uint64_t elementnumber = OSProot->_elementnumber;
			if(elementnumber < 2) elementnumber = 2;
			
			OSProot->_current = calloc(1, sizeof(OSPobj) +
										(sizeof(void *) * elementnumber) +
										OSProot->_datasize);
			OSPMerge(objarg, OSProot->_current);
		
			while(functcurrent) {
				OSProot->_current->_fct[functcurrent->_function._functionid] =
											functcurrent->_function._function;
				functcurrent = functcurrent->_nxt;
			}
		}
		
		OSProot->_current->_tfd = -1;
		if(OSProot->_datasize) {
			OSProot->_current->_dat = &OSProot->_current->_fct[OSProot->_elementnumber];
		}
		
		if(!OSProot->_current->_fct[0]) {
			OSProot->_current->_fct[0] = OSPMng;
		}
		if(!OSProot->_current->_fct[1]) {
			OSProot->_current->_fct[1] = OSPDui;
		}
		
		return OSProot->_current;
		break;
	case ADDFCT:
		if(fctarg) {
			if(OSProot->_elementnumber) {
				if((OSProot->_functend->_elementnumber + 1) % FUNCALLOCSIZE) {
					OSProot->_functend->_nxt = &OSProot->_functend[1];
					OSProot->_functend->_nxt->_prv = OSProot->_functend;
					OSProot->_functend = OSProot->_functend->_nxt;
				}
				else {
					functelement *functend = OSProot->_functend;
					OSProot->_functend = calloc(FUNCALLOCSIZE, sizeof(functelement));
					OSProot->_functend->_prv = functend;
				}
			}

			if(OSProot->_functend->_prv) {
				OSProot->_functend->_elementnumber = OSProot->_functend->_prv->_elementnumber + 1;
			}
			else {
				OSProot->_functend->_elementnumber = 0;
			}
			
			/* OSProot->_functend->_nxt = 0; done by calloc */
			OSProot->_functend->_function._function = fctarg->_function;
			OSProot->_functend->_function._functionid = fctarg->_functionid;
			
			if(OSProot->_elementnumber <= fctarg->_functionid) {
				OSProot->_elementnumber = fctarg->_functionid + 1;
			}
		}
		else{
			while(OSProot->_functend->_prv) {
				OSProot->_functend = OSProot->_functend->_prv;
				if(!((OSProot->_functend->_elementnumber + 1) % FUNCALLOCSIZE)) {
					free(OSProot->_functend->_nxt);
				}
			}
			
			OSProot->_elementnumber = 0;
			OSProot->_datasize = 0;
		}
		
		break;
	default:
		if(datarg) {
			OSProot->_datasize += datarg[0];
		}
		else {
			while(OSProot->_toplevels) OSPDEL(OSProot->_toplevels);
			OSPRESET;
			
			close(OSProot->_eventpoll);
			
			free(OSProot->_functqueue);
			free(OSProot);
		}
	}
	
	return 0;
}

void OSPTrg(trigdesc *trigdescription) {
	struct epoll_event event = {trigdescription->_events, {trigdescription->_object}};
	if(!trigdescription) return;
	if(!trigdescription->_object) return;
	
	trigdescription->_object->_trg = trigdescription->_functionid;
	trigdescription->_object->_tpt = trigdescription->_pointer;
	
	if(trigdescription->_filedesc < 0) {
		epoll_ctl(OSProot->_eventpoll, EPOLL_CTL_DEL, trigdescription->_object->_tfd, &event);
		trigdescription->_object->_tfd = -1;
	}
	else if(trigdescription->_object->_tfd < 0) {
		epoll_ctl(OSProot->_eventpoll, EPOLL_CTL_ADD, trigdescription->_filedesc, &event);
		trigdescription->_object->_tfd = trigdescription->_filedesc;
	}
	else {
		epoll_ctl(OSProot->_eventpoll, EPOLL_CTL_MOD, trigdescription->_object->_tfd, &event);
	}
}

OSPobj *OSPWte(int miliseconds) {
	struct epoll_event event;
	
	if(epoll_wait(OSProot->_eventpoll, &event, 1, miliseconds)) {
		printf("Event append\n");
		OSPobj *obj = event.data.ptr;
		OSPRUN(obj, obj->_trg, obj->_tpt);
		return obj;
	}
	
	return 0;
}
