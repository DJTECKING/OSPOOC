#include"../include/OSPlib.h"

void OSPDevHdl(OSPobj **obj) {
	OSPdev *dev = (OSPdev *) obj[0];

	if(dev->_path) {
		free(dev->_path);
	}

	if(dev->_fd >= 0) {
		close(dev->_fd);
	}
}

/* Made for encapsulate ioctl or fcntl calls */
void OSPDevControl(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevControl : not implemented on this device");
}

/* Made for encapsulate tcsetattr or equivalent */
void OSPDevSetASynq(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevSetASynq : not implemented on this device");
}

/* Made for encapsulate tcsetattr or equivalent */
void OSPDevSetSynq(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevSetSynq : not implemented on this device");
}

/* Made for encapsulate fchown and/or fchmod calls */
void OSPDevSetStat(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevSetStat : not implemented on this device");
}

/* Made for encapsulate fstat */
void OSPDevGetStat(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevGetStat : not implemented on this device");
}

/* Made for encapsulate flock calls */
void OSPDevLock(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevLock : not implemented on this device");
}

/* Made for encapsulate flock calls */
void OSPDevWaitLock(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevWaitLock : not implemented on this device");
}

/* Made for encapsulate flock calls */
void OSPDevUnLock(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevUnLock : not implemented on this device");
}

/* Made for encapsulate read calls */
void OSPDevRead(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevRead : not implemented on this device");
}

/* Made for encapsulate blocking read calls */
void OSPDevFRead(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevFRead : not implemented on this device");
}

/* Made for encapsulate write calls */
void OSPDevWrite(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevWrite : not implemented on this device");
}

/* Made for encapsulate blocking write calls */
void OSPDevFWrite(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevFWrite : not implemented on this device");
}

/* Made for encapsulate fseek calls */
void OSPDevSeek(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevSeek : not implemented on this device");
}

/* Made for encapsulate ftruncate calls */
void OSPDevTrunc(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevTrunc : not implemented on this device");
}

/* Made for encapsulate mmap calls */
void OSPDevMap(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevMap : not implemented on this device");
}

/* Made for encapsulate munmap calls */
void OSPDevUnMap(OSPobj *obj, va_list arg) {
	OSPrint(0, "OSPDevUnMap : not implemented on this device");
}

OSPctr *OSPCtrdev() {
	static OSPctr *ctr = 0;
	
	if(!ctr) {
		ctr = OSPCtr(0, 16, sizeof(OSPdev), OSPDevHdl);
		ctr->_vrt = 1;
		
		ctr->_fct[OSPDEVCONTROL] = OSPDevControl;
		
		ctr->_fct[OSPDEVSETASYNQ] = OSPDevSetASynq;
		ctr->_fct[OSPDEVSETSYNQ] = OSPDevSetSynq;
		
		ctr->_fct[OSPDEVSETSTAT] = OSPDevSetStat;
		ctr->_fct[OSPDEVGETSTAT] = OSPDevGetStat;
		
		ctr->_fct[OSPDEVLOCK] = OSPDevLock;
		ctr->_fct[OSPDEVWAITLOCK] = OSPDevWaitLock;
		ctr->_fct[OSPDEVUNLOCK] = OSPDevUnLock;
		
		ctr->_fct[OSPDEVREAD] = OSPDevRead;
		ctr->_fct[OSPDEVFREAD] = OSPDevFRead;
		ctr->_fct[OSPDEVWRITE] = OSPDevWrite;
		ctr->_fct[OSPDEVFWRITE] = OSPDevFWrite;

		ctr->_fct[OSPDEVSEEK] = OSPDevSeek;
		ctr->_fct[OSPDEVTRUNCATE] = OSPDevTrunc;

		ctr->_fct[OSPDEVMAP] = OSPDevMap;
		ctr->_fct[OSPDEVUNMAP] = OSPDevUnMap;
	}
	
	return ctr;
}

