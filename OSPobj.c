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
    }
    
    switch(task) {
    case ADDOBJ:
        if(!objarg) objarg = (OSPobj *) OSProot;
        
        OSProot->_current = calloc(1, sizeof(OSPobj) +
                                    (sizeof(void *) * OSProot->_elementnumber) +
                                    OSProot->_datasize);
        OSPMerge(objarg, OSProot->_current);
        
        {
            functelement *functcurrent = OSProot->_functqueue;
            while(functcurrent) {
                OSProot->_current->_OSPFct[functcurrent->_function._functionid] =
                                            functcurrent->_function._function;
            }
            
            OSProot->_current->_OSPFct[functcurrent->_function._functionid] =
                                            functcurrent->_function._function;
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
            functelement *newelement;
            
            if(OSProot->_elementnumber % FUNCALLOCSIZE) {
                newelement = &OSProot->_functend[1];
            }
            else if(OSProot->_elementnumber) {
                newelement = calloc(FUNCALLOCSIZE, sizeof(functelement));
            }
            else {
                newelement = OSProot->_functend;
            }
            
            /* newelement->_nxt = 0; done by calloc */
            newelement->_function._function = fctarg->_function;
            newelement->_function._functionid = fctarg->_functionid;
            newelement->_elementnumber = OSProot->_elementnumber;
            if(!(OSProot->_elementnumber++)) {
                newelement->_prv = 0;
                newelement = &newelement[1];
            }
            newelement->_prv = OSProot->_functend;

            OSProot->_functend->_nxt = newelement;
            OSProot->_functend = newelement;
            
            if(OSProot->_elementnumber < (fctarg->_functionid + 1)) {
                OSProot->_elementnumber = fctarg->_functionid + 1;
            }
        }
        else{
            while(OSProot->_functend->_elementnumber) {
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
            OSPAdd(ADDFCT, 0);
            
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
        OSPDO(obj, OSPMNG, 0);
    }
}

void OSPDui(OSPobj *obj, void *arg) {
    printf("Object is located at %p\n", obj);
}
