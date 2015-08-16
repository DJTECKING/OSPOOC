#include"OSPobj.h"

#define FUNCALLOCSIZE 1024

typedef struct functelement_s {
    struct functelement_s *_prv; /* Previous function container */
    struct functelement_s *_nxt; /* Previous function container */
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
    
    object->_mtr->_slv = object->_nxt == object ? 0 : object->_nxt;
    object->_mtr = 0;
}

OSPobj *OSPAdd(OSPaddtask task, void *arg) {
    OSPobj *objarg = (OSPobj *) arg;
    functdesc *fctarg = (functdesc *) arg;
    size_t *datarg = (size_t *) arg;
    
    if(!OSProot) {
        OSProot = calloc(1, sizeof(*OSProot));
        OSProot->_functqueue = calloc(FUNCALLOCSIZE, sizeof(functelement));
        OSProot->_functend = OSProot->_functqueue;
        OSProot->_functend->_prv = 0;
        OSProot->_functend->_elementnumber = 0;
        OSProot->_elementnumber = 0;
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
                OSProot->_current->_OSPFct[functcurrent->_function._functionid] =
                                            functcurrent->_function._function;
		functcurrent = functcurrent->_nxt;
            }
        }
        
        OSProot->_current->_OSPdat = &OSProot->_current->_OSPFct[OSProot->_elementnumber];
        
        if(!OSProot->_current->_OSPFct[0]) {
            OSProot->_current->_OSPFct[0] = OSPMng;
        }
        if(!OSProot->_current->_OSPFct[1]) {
            OSProot->_current->_OSPFct[1] = OSPDui;
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
            while(OSProot->_toplevels) OSPDO(OSProot->_toplevels, OSPMNG, 0);
            OSPRESET;
            
            free(OSProot->_functqueue);
            free(OSProot);
        }
    }
    
    return 0;
}
    
void OSPMng(OSPobj *obj, void *mtr) {
    OSPSplit(obj);
    
    if(mtr) {
        OSPMerge(mtr, obj);
    }
    else {
        while(obj->_slv) OSPDO(obj->_slv, OSPMNG, 0);
        OSPSplit(obj);
        free(obj);
    }
}

void OSPDui(OSPobj *obj, void *arg) {
    printf("Object is located at %p\n", obj);
}
