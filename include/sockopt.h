#ifndef __SOCKOPT_H
#define __SOCKOPT_H

#include <arpa/inet.h>

#define getPeerIpaddr(sock, buff, bufferSize) getIpaddr(getpeername, sock, buff, bufferSize);

typedef int (*getnamefunc)(int socket, struct sockaddr *address, socklen_t *address_len);

extern int tcprecvdata(int sock, void* data, int size, int timeout);
extern int tcpsenddata(int sock, void* data, int size, int timeout);
extern int socketServer(const char *bind_addr,const int port,const char *LogFilePrex);
extern int nbaccept(int sock,int timeout, int *err_no);

extern in_addr_t getIpaddr(getnamefunc getname, int sock, char *buff, const int bufferSize);

#endif