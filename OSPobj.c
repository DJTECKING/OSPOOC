#ifndef __OSPOBJ_H__
#define __OSPOBJ_H__

#include"OSPobj.h"

static struct {
    OSPobj *_toplevels;
    int _eventpoll;
    struct epoll_event _event;
} OSProot = 0;

OSPobj *OSPAdd(OSPaddtask task, void *arg) {
    static struct functlink_s {
        uint64_t _elementnumber;
        uint64_t _functionid;
        functionqueue = calloc(1024, sizeof(functionqueue_t));
    
    switch(task) {
    case ADDOBJ:
        if(!arg) {
            if(!OSProot) {
                OSProot = calloc(1, sizeof(OSProot);
            }
        
        break;
    case ADDFCT:
        break;
    default:
    }
}

#endif /* __OSPOBJ_H__ */