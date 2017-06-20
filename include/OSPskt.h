#ifndef __OSPSKT_H__
#define __OSPSKT_H__

#define OSPCON_LOCK				0
#define OSPCON_UNLOCK			1

#define OSPCON_REGULAR			0
#define OSPCON_SOCKET			1
#define OSPCON_SOCKET_OPEN		1
#define OSPCON_COUNTED_OPEN		2
#define OSPCON_SOCKET_CLOSED	3 /* Manually closed */
#define OSPCON_COUNTED_CLOSED	4 /* Automatically closed */

#define OSPNDP_REGULAR			0
#define OSPNDP_SOCKET			1

typedef struct OSPcontainer_s {
	OSPdev _dev;

	struct OSPendpoint_s *_slv; /* Can be an OSPcontainer * */

	uint16_t _port;
	uint16_t _status;
	/*	Folder => OSPCON_REGULAR
		Socket => OSPCON_SOCKET_OPEN
		Socket => OSPCON_COUNTED_OPEN (with counted slots) */
	uint64_t _guest_num;
	struct sockaddr _conf;
	socklen_t _len;
} OSPcontainer;

typedef struct OSPendpoint_s {
	OSPobj _obj;

	OSPcontainer *_mtr;
	struct OSPendpoint_s *_prv; /* Can be an OSPcontainer * */
	struct OSPendpoint_s *_nxt; /* Can be an OSPcontainer * */

	int _fd;
	char *_path;
	int _type;
	/*	Not applicable for regular file,
		TCP => SOCK_STREAM
		UDP => SOCK_DGRAM */
	uint16_t _port;
	uint16_t _status;
	/*	File => OSPNDP_REGULAR
		Socket => OSPNDP_SOCKET */
	struct sockaddr _conf;
	socklen_t _len;
} OSPendpoint;

#define OSPTCP 0
#define OSPUDP 1

OSPctr *OSPCntCtr();
OSPobj *OSPCnt(uint8_t, char *);

OSPctr *OSPNdpCtr();
OSPobj *OSPNdp(uint8_t, int);

#endif /* __OSPSKT_H__ */
