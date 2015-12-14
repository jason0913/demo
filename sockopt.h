#ifndef __SOCKOPT_H
#define __SOCKOPT_H

extern int socketServer(const char *bind_addr,const int port,const char *LogFilePrex);
extern int nbaccept(int sock,int timeout);

#endif