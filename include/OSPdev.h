#ifndef __OSPDEV_H__
#define __OSPDEV_H__

/*	OSPobj _obj
		OSPdev Add file descriptor and all unimplemented methods
*/

typedef struct {
	OSPobj _obj;
	char *_path;
	int _fd;
} OSPdev;

#define OSPDEVCONTROL	0

#define OSPDEVSETASYNQ	1
#define OSPDEVSETSYNQ	2

#define OSPDEVSETSTAT	3
#define OSPDEVGETSTAT	4

#define OSPDEVLOCK		5
#define OSPDEVWAITLOCK	6
#define OSPDEVUNLOCK	7

#define OSPDEVREAD		8
#define OSPDEVFREAD		9
#define OSPDEVWRITE		10
#define OSPDEVFWRITE	11

#define OSPDEVSEEK		12
#define OSPDEVTRUNCATE	13

#define OSPDEVMAP		14
#define OSPDEVUNMAP		15

OSPctr *OSPCtrdev();

#endif /* __OSPDEV_H__ */
