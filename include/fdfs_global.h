#ifndef __FDFS_GLOBALH
#define __FDFS_GLOBALH

#include "fdfs_define.h"
#include "tracker_types.h"

typedef struct
{
	char major;
	char minor;
} FDFSVersion;

 extern int g_server_port;
 extern int g_max_connections;

 extern bool g_continue_flag;
 extern char g_base_path[MAX_PATH_SIZE];
 extern int g_network_timeout;
 extern FDFSVersion g_version;

 extern pthread_mutex_t g_tracker_thread_lock;
 extern int g_tracker_thread_count ;
 extern int g_storage_reserved_mb;
 extern FDFSGroups g_groups;

#endif