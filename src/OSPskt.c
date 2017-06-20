#include"../include/OSPlib.h"

/*	Close TCP server
	This is automatically called for an OSPCON_COUNTED_OPEN container.
	This toggles the container to OSPCON_SOCKET_CLOSED if called manually,
	that means that the container won't open automatically if a client leaves.
*/
void OSPCntLock(OSPobj *obj, va_list arg) {
	OSPcontainer *cnt = (OSPcontainer *) obj;

	switch(cnt->_status) {
	case OSPCON_REGULAR:
	case OSPCON_SOCKET_CLOSED:
	case OSPCON_COUNTED_CLOSED:
		return;

	default:
		OSPTrg(0, cnt->_dev._fd, 0);
		shutdown(cnt->_dev._fd, SHUT_RDWR); /* Avoid time wait */
		close(cnt->_dev._fd);

		cnt->_dev._fd = -1;
		if(!cnt->_guest_num || (cnt->_status == OSPCON_COUNTED_OPEN)) {
			cnt->_status = OSPCON_COUNTED_CLOSED;
		}
		else {
			cnt->_status = OSPCON_SOCKET_CLOSED;
		}
	}
}

/*	Reopen the TCP server
	This is automatically called for an OSPCON_COUNTED_CLOSED container if a client is leaving.
	Calling it manually makes the container OSPCON_COUNTED_OPEN if a pointer to an int is given as argument
	(the pointed integer defines number of empty slot to add to existing connection).
	Calling it manually makes the container OSPCON_SOCKET_OPEN if 0 is given as argument,
	then, the container won't close automatically.
*/
void OSPCntUnLock(OSPobj *obj, va_list arg) {
	OSPcontainer *cnt = (OSPcontainer *) obj;
	int *num = va_arg(arg, int *);

	if(cnt->_dev._fd < 0) {
		switch(cnt->_status) {
		case OSPCON_REGULAR:
		case OSPCON_SOCKET_OPEN:
		case OSPCON_COUNTED_OPEN:
			return;

		default:
			{
				struct sockaddr_in *addr = (struct sockaddr_in *) &cnt->_conf;

				addr->sin_family = AF_INET;
				addr->sin_addr.s_addr = INADDR_ANY;
				addr->sin_port = cnt->_port;
			}

			if((cnt->_dev._fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				OSPrint(0, "OSPCntUnLock : Socket creation failed : %s", strerror(errno));
				return;
			}

			{
				int arg = 1;
				if(setsockopt(cnt->_dev._fd, SOL_SOCKET, SO_REUSEADDR, &arg, sizeof(int)) < 0) {
					OSPrint(0, "OSPCntUnLock : Set socket address reusable failed : %s", strerror(errno));
					close(cnt->_dev._fd);
					return;
				}
			}

			if(bind(cnt->_dev._fd, &cnt->_conf, cnt->_len)) {
				OSPrint(0, "OSPCntUnLock : Socket binding failed : %s", strerror(errno));
				close(cnt->_dev._fd);
				return;
			}

			if(OSPTrg(&cnt->_dev._obj, cnt->_dev._fd, EPOLLIN | EPOLLHUP)) {
				OSPrint(0, "OSPCntUnLock : Socket event subscription failed");
				close(cnt->_dev._fd);
				return;
			}

			if(listen(cnt->_dev._fd, 4)) {
				OSPrint(0, "OSPCntUnLock : Socket listening failed : %s", strerror(errno));
				OSPTrg(0, cnt->_dev._fd, 0);
				close(cnt->_dev._fd);
				return;
			}

			if(num && num[0]) {
				cnt->_guest_num = num[0];
				cnt->_status = OSPCON_COUNTED_OPEN;
			}
			else {
				cnt->_guest_num = 0;
				cnt->_status = OSPCON_SOCKET_OPEN;
			}

			return;
		}
	}
}

void OSPCntHdl(OSPobj **obj) {
	OSPcontainer *mtr = (OSPcontainer *) obj[0];
	OSPendpoint *ret;
	int peer_fd;
	struct sockaddr peer_conf;
	socklen_t peer_len;

	if(!obj[0]->_buf) {
		while(mtr->_slv) {
			OSPFre(&mtr->_slv->_obj);
		}

		if(mtr->_dev._fd >= 0) {
			OSPTrg(0, mtr->_dev._fd, 0);
			shutdown(mtr->_dev._fd, SHUT_RDWR); /* Avoid time wait */
			close(mtr->_dev._fd);
		}

		OSPFre(obj[0]);
		return;
	}

	if(mtr->_type = OSPCON_REGULAR) {
		/* TODO Later, see man inotify */
	}
	else {
		peer_fd = accept(mtr->_dev._fd, &peer_conf, &peer_len);

		if(peer_fd < 0) {
			OSPrint(0, "OSPCntHdl : Connection acceptation failed : %s", strerror(errno));
			return;
		}

		if(peer_conf.sa_family != AF_INET) {
			OSPrint(0, "OSPCntHdl : Connection between INET and not INET endpoints refused");
			shutdown(peer_fd, SHUT_RDWR); /* Avoid time wait */
			close(peer_fd);
			return;
		}

		if(OSPTrg(&ret->_obj, peer_fd, EPOLLIN | EPOLLHUP)) {
			OSPrint(0, "OSPCntHdl : Socket event subscription failed");
			shutdown(peer_fd, SHUT_RDWR); /* Avoid time wait */
			close(peer_fd);
			return;
		}

		switch(mtr->_status) {
		case OSPCON_COUNTED_OPEN:
			if(mtr->_guest_num) mtr->_guest_num--;
			if(mtr->_guest_num) break;
			OSPRun(obj[0], OSPCON_LOCK);
			OSPrint(1, "OSPCntHdl : Max client connection reached, closing socket");
		case OSPCON_REGULAR:
		case OSPCON_SOCKET_CLOSED:
		case OSPCON_COUNTED_CLOSED:
			return;
		}

		if(!(ret = (OSPendpoint *) OSPAdd(OSPNdpCtr()))) {
			OSPrint(0, "OSPCntHdl : Unable to create the client instance");
			shutdown(peer_fd, SHUT_RDWR); /* Avoid time wait */
			close(peer_fd);
			return;
		}

		ret->_dev._fd = peer_fd;
		sprintf(ret->_dev._path, "%s:%d",
			inet_ntoa(((struct sockaddr_in *) &peer_conf)->sin_addr),
			ntohs(mtr->_port));
		ret->_port = mtr->_port;
		ret->_status = OSPNDP_SOCKET;

		memcpy(&ret->_conf, &peer_conf, sizeof(struct sockaddr));
		memcpy(&ret->_len, &peer_len, sizeof(socklen_t));
	}

	ret->_mtr = mtr;
	ret->_nxt = mtr->_slv;
	ret->_prv = ret->_prv ? ret->_prv->_prv : 0;
	if(ret->_nxt) ret->_nxt->_prv = ret;
	if(ret->_prv) ret->_prv->_nxt = ret;
	mtr->_slv = ret;
	while(mtr->_slv->_prv) mtr->_slv = mtr->_slv->_prv;
}

OSPctr *OSPCtrCnt() {
	static OSPctr *ret = 0;

	if(ret) return ret;
	ret = OSPCtr(OSPCtrdev(), 16, sizeof(OSPcontainer), OSPCntHdl);

	ret->_fct[OSPDEVLOCK] = OSPCntLock;
	ret->_fct[OSPDEVUNLOCK] = OSPCntUnLock;

	return ret;
}

/*	Create a folder handler
	for path specified by ptprt arg if type == OSPCON_REGULAR or
	a listening socket for TCP connections if type == OSPCON_SOCKET
	which will listen on port specified by addr arg
	Events arg is optionnal and can be set to 0.
	This arg let the user specify what event to subscribe to
	for the specified file or folder,
	it is ignored if type == OSPCON_SOCKET.
*/
OSPcontainer *OSPCnt(uint8_t type, char *ptprt, uint32_t events) {
	OSPcontainer *ret;
	int fd = -1;
	char *porterror = ptprt;
	uint16_t port = 0;

	switch(type) {
	case OSPCON_REGULAR:
		{
			struct stat buf;

			if(stat(ptprt, &buf)) {
				switch(errno) {
				default:
					OSPrint(1, "OSPCnt : Unable to get %s proprieties : %s", ptprt, strerror(errno));
					return 0;

				case EBADF:
				case ENOENT:
					if(mkdir(ptprt, 0644)) {
						OSPrint(0, "OSPCnt : Unable to create %s : %s", ptprt, strerror(errno));
						return 0;
					}
				}

			}
			else if(!S_ISDIR(buf.st_mode)) {
				OSPrint(0, "OSPCnt : %s is not a directory", ptprt);
				return 0;
			}

			/*	Here, it looks strange, but it is not.
				we use direntry to read content of directory.
				But we use inotify to be notified of any event
				on that directory.
				We could use inotify with only on fd, storing it in OSProot
				and watching everything we need from that fd.
				But we would then loose epoll benefit because
				we can't store library data in inotify events.
			*/
			if(events) {
				if((fd = inotify_init()) < 0) {
					OSPrint(0, "OSPCnt : Unable to create notifier : %s", strerror(errno));
					return 0;
				}

				if((fd = inotify_add_watch(fd, ptprt, events)) < 0) {
					OSPrint(0, "OSPCnt : Unable to add %s to notifier : %s", ptprt, strerror(errno));
					return 0;
				}
			}
		}
		break;

	case OSPCON_SOCKET:
		port = strtol(ptprt, porterror, 0);
		
		if(ptprt == porterror) {
			OSPrint(0, "OSPCnt : %s is not a valid port", ptprt);
			return 0;
		}
		break;

	default:
		OSPrint(0, "OSPCnt : The specified type of container is not valid");
		return 0;
	}

	OSPcontainer *ret = (OSPcontainer *) OSPAdd(OSPCtrCnt());

	if(type == OSPCON_REGULAR) {
		if(!(ret->_dev._path = strdup(ptprt))) {
			OSPFre(&ret->_dev.obj);
			return 0;
		}
	}
	else {
		ret->_port = port;
	}

	ret->_status = type;
	ret->_dev._fd = fd;
	return ret;
}

/*	Handle socket interruptions :
	Destroys itself if hang up (connection closed by host)
	Returns what handler object is defined to return
	Returns itself if handler object returns nothing or no handler object defined
*/
void OSPHdlNdp(OSPobj **obj) {
	uint8_t ret = 0;
	OSPendpoint *ndp = (OSPendpoint *) obj[0];

	if(
	if(!recv(clt->_dev._fd, &ret, 1, MSG_PEEK)) {
		/* Connection closed, notify it in the status */
		clt->_status = 0;
	}

	if(clt->_hdl) {
		/* If !clt->_status, clt->_hdl->_ctr._hdl() must make sure
		nothing will reference this socket anymore
		because socket is being destroyed /!\ */
		ret = clt->_hdl->_ctr._hdl(clt->_hdl, who);
	}

	if(!clt->_status) {
		/* Connection closed, be sure that if parent is a server, freeing a place */
		if(obj->_mtr->_ctr == OSPCtrSrv()) {
			OSPRun(obj->_mtr, OSPDEVUNLOCK);
		}
		/* shutdown(clt->_dev._fd, SHUT_RDWR); /* Avoid time wait */
		OSPDEL(obj);
		return 1;
	}

	return ret;
}

/*	Add a client socket :
	type is either SOCK_STREAM for a TCP connection
	or SOCK_DGRAM for an UDP connection
	addr must be a string in the form CCC.CCC.CCC.CCC:PPPPP
	where CCC must be a value between 0 and 255 included
	and PPPPP must be a value between 0 and 65535 included
	It represent the adresse and the port of the host to connect with
*/
OSPobj *OSPAddClt(int type, const char *addr) {
	OSPclt *ret = (OSPclt *) OSPAdd(OSPCtrClt());
	struct sockaddr_in servaddr;

	ret->_type = type;
	ret->_dev._fd = -1;
	ret->_status = 1;
	
	if(addr) {
		uint8_t *portstart = strchr(addr, ':');
		uint8_t *portend;
		uint16_t port;
		
		if(!portstart[0]) {
			OSPDEL((OSPobj *) ret);
			return 0;
		}
		*(portstart++) = 0;
		port = strtol(portstart, portend, 0);
		
		if(portstart == portend) {
			OSPDEL((OSPobj *) ret);
			return 0;
		}
		
		if((ret->_dev._fd = socket(AF_INET, ret->_type, 0)) < 0) {
			OSPDEL((OSPobj *) ret);
			return 0;
		}
		
		ret->_conf.sin_family = AF_INET;
		if(!inet_aton(addr, &ret->_conf.sin_addr.s_addr) {
			OSPDEL((OSPobj *) ret);
			return 0;
		}
		ret->_conf.sin_port = htons(port);
		
		if(connect(sockfd,(struct sockaddr *) &servaddr,sizeof(sockaddr_in)) < 0) {
			OSPDEL((OSPobj *) ret);
			return 0;
		}
	}
	
	return ret;
}

uint8_t OSPHdlSrv(OSPobj *obj, OSPobj **who) {
	OSPsrv *srv = (OSPsrv *) obj;
	OSPclt *clt = OSPAddClt(obj, srv->_type, 0);

	if((clt->_dev._fd = accept(srv->_dev._fd,
							(struct sockaddr *) srv->_conf,
							sizeof(struct sockaddr))) < 0) {
		OSPDEL((OSPobj *) clt);
		
		*who = obj;
		
		return 0;
	}
	
	*who = clt;
	
	OSPRun(obj, OSPDEVLOCK);
	
	clt->_port = srv->_port;
}

OSPsrv *OSPAddSrv(uint8_t type, int port) {
	OSPobj *obj = OSPAdd(OSPCtrSrv());
	OSPRun(obj, OSPDEVUNLOCK);
	
	return obj;
}

int main(int argc, char *argv[]) {
	
	OSPFREEALL;
	return 0;
}

