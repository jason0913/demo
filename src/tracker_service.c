#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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

static int tracker_check_logined(TrackerClientInfo *pClientInfo)
{

#ifdef __DEBUG__
	printf("tracker_check_logined! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_storage_report(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_storage_report! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_storage_beat(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_storage_beat! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_storage_join(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_storage_join! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_storage_replica_chg(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_storage_replica_chg! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_service_query_fetch(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_service_query_fetch! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_service_query_storage(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_service_query_storage! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_server_list_groups(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_service_query_storage! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_server_list_group_storages( \
		TrackerClientInfo *pClientInfo, const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_server_list_group_storages! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_storage_sync_src_req(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_storage_sync_src_req! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_storage_sync_dest_req(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_storage_sync_dest_req! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_deal_storage_sync_notify(TrackerClientInfo *pClientInfo, \
				const int nInPackLen)
{

#ifdef __DEBUG__
	printf("tracker_deal_storage_sync_notify! file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

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

		header.pkg_len[sizeof(header.pkg_len) -1] = '\0';
		nInPackLen = strtol(header.pkg_len,NULL,16);

		//tracker_check_dirty(&client_info);

		if (TRACKER_PROTO_CMD_STORAGE_BEAT == header.cmd)
		{
			if (0 != tracker_check_logined(&client_info))
			{
				break;
			}
			if (0 != tracker_deal_storage_beat(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_STORAGE_REPORT == header.cmd)
		{
			if (0 != tracker_check_logined(&client_info))
			{
				break;
			}
			if (0 != tracker_deal_storage_report(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_STORAGE_JOIN == header.cmd)
		{
			if (0 != tracker_deal_storage_join(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_STORAGE_REPLICA_CHG == header.cmd)
		{
			if (0 != tracker_check_logined(&client_info))
			{
				break;
			}
			if (0 != tracker_deal_storage_replica_chg(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_SERVICE_QUERY_FETCH == header.cmd)
		{
			if (0 != tracker_deal_service_query_fetch(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_SERVICE_QUERY_STORE == header.cmd)
		{
			if (0 != tracker_deal_service_query_storage(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_SERVER_LIST_GROUP == header.cmd)
		{
			if (0 != tracker_deal_server_list_groups(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_SERVER_LIST_STORAGE == header.cmd)
		{
			if (0 != tracker_deal_server_list_group_storages(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_STORAGE_SYNC_SRC_REQ == header.cmd)
		{
			if (0 != tracker_deal_storage_sync_src_req(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_STORAGE_SYNC_DEST_REQ == header.cmd)
		{
			if (0 != tracker_deal_storage_sync_dest_req(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_STORAGE_SYNC_NOTIFY == header.cmd)
		{
			if (0 != tracker_deal_storage_sync_notify(&client_info,nInPackLen))
			{
				break;
			}
		}
		else if (TRACKER_PROTO_CMD_STORAGE_QUIT == header.cmd)
		{
			logError("file: %s, line: %d, "   \
				"cmd: TRACKER_PROTO_CMD_STORAGE_QUIT%d", \
				__FILE__,__LINE__,header.cmd);
			break;
		}
		else
		{
			logError("file: %s, line: %d, "   \
				"client ip: %s, unkown cmd: %d", \
				__FILE__,__LINE__, client_info.ip_addr, \
				header.cmd);
			break;
		}
		count++;
		printf("thread id %ld\n", pthread_self());
	}

	if (g_continue_flag)
	{
		//
		//
	}

	if (NULL != client_info.pGroup)
	{
		--(*(client_info.pGroup->ref_count));
	}

	if (NULL != client_info.pStorage)
	{
		--(*(client_info.pStorage->ref_count));
	}

	if (pthread_mutex_lock(&g_tracker_thread_lock) != 0)
	{
		logError("file: %s, line: %d, " \
			"call pthread_mutex_lock fail, " \
			"errno: %d, error info:%s.", \
			__FILE__,__LINE__, errno, strerror(errno));
	}
	g_tracker_thread_count--;
	if (pthread_mutex_unlock(&g_tracker_thread_lock) != 0)
	{
		logError("file: %s, line: %d, " \
			"call pthread_mutex_unlock fail, " \
			"errno: %d, error info: %s", \
			__FILE__,__LINE__, errno, strerror(errno));
	}

	close(client_info.sock);

	return NULL;
}