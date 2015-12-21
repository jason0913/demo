#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include "tracker_service.h"
#include "fdfs_define.h"
#include "tracker_proto.h"
#include "tracker_types.h"
#include "sockopt.h"
#include "logger.h"
#include "fdfs_global.h"

extern bool g_continue_flag;

void* tracker_thread_entrance(void* arg)
{
/*
package format:
9 bytes length (hex string)
1 bytes cmd (char)
1 bytes status(char)
data buff (struct)
*/

	TrackerClientInfo client_info;
	TrackerHeader header;
	int result;
	int nInPackLen;
	int count;

	memset(&client_info,0,sizeof(TrackerClientInfo));
	client_info.sock = (int) arg;

	getPeerIpaddr(client_info.sock, \
				client_info.ip_addr, FDFS_IPADDR_SIZE);

#ifdef __DEBUG__
	printf("tracker_thread_entrance done!\n");
#endif

	count =0;
	while(g_continue_flag)
	{

		result = tcprecvdata(client_info.sock,&header,\
				sizeof(header),g_network_timeout);
		if (0 == result && count >0)
		{
			continue;
		}
		if (result != 1)
		{
			logError("file: %s, line: %d, " \
				"client ip: %s, recv data fail, " \
				"errno: %d, error info: %s", \
				__FILE__,__LINE__, client_info.ip_addr, \
				errno, strerror(errno));
			break;
		}
		printf("thread id %ld\n", pthread_self());
		sleep(2);
		break;
	}

	return NULL;
}