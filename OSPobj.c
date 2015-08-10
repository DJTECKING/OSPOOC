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
        OSProot = calloc(FUNCALLOCSIZE, sizeof(*OSProot));
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
        break;
    case ADDFCT:
        if(fctarg) {
            uint64_t elementnumber = OSProot->_functend->_elementnumber + 1;
            functelement *newelement;
            if(!elementnumber) return 0;
            
            newelement = elementnumber % FUNCALLOCSIZE ?
                        &OSProot->_functend[1] :
                        calloc(FUNCALLOCSIZE, sizeof(functelement));

            /* newelement->_nxt = 0; done by calloc */
            newelement->_function._function = fctarg->_function;
            newelement->_function._functionid = fctarg->_functionid;
            newelement->_elementnumber = elementnumber;
            newelement->_prv = OSProot->_functend;
            OSProot->_functend->_nxt = newelement;
            OSProot->_functend = newelement;
        }
        else{
            while(OSProot->_functend->_elementnumber) {
                OSProot->_functend = OSProot->_functend->_prv;
                if((OSProot->_functend->_elementnumber + 1) % FUNCALLOCSIZE) {
                    free(OSProot->_functend->_nxt);
                }
            }
        }
        
        break;
    default:;
    }
    
    return 0;
}
