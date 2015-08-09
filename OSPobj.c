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
    uint64_t _functcount;
    size_t _datasize;

    OSPobj *_current;
} OSProot = 0;

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
    
    if(object->_nxt == object) {
        object->_mtr->_slv = 0;
    }
    else {
        object->_mtr->_slv = object->_nxt;
    }
    
    object->_mtr = 0;
}

OSPobj *OSPAdd(OSPaddtask task, void *arg) {
    OSPobj *objarg = (OSPobj *) arg;
    functdesc *fctarg = (functdesc *) arg;
    
    if(!OSProot) {
        OSProot->_functionqueue = calloc(1024, sizeof(functelement));
        OSProot->_functionend = OSProot->_functionqueue;
    }
    
    switch(task) {
    case ADDOBJ:
        if(!objarg) objarg = OSProot;
        
        OSProot->_current = calloc(1, sizeof(OSPobj) +
                                    (sizeof(void *) * OSProot->_functcount) +
                                    OSProot->_datasize);
        OSPMerge(objarg, OSProot->_current);
        
        {
            functelement *functcurrent;
            for(functcurrent = OSProot->_functqueue;
                functcurrent != OSProot->_functend;
                functcurrent = functcurrent->_nxt) {
                OSProot->_current->_OSPFct[functcurrent->_function->_functionid] =
                                            functcurrent->_function->_function;
            }
        }
        break;
    case ADDFCT:
        break;
    default:
    }
}
