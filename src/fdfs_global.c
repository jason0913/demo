#include <pthread.h>
#include "fdfs_global.h"

 int g_server_port = FDFS_STORAGE_SERVER_DEF_PORT;
 int g_max_connections = FDFS_DEF_MAX_CONNECTONS;

 bool g_continue_flag = true;
 char g_base_path[MAX_PATH_SIZE];
 int g_network_timeout = DEFAULT_NETWORK_TIMEOUT;

 FDFSVersion g_version = {1, 2};

 pthread_mutex_t g_tracker_thread_lock;
 int g_tracker_thread_count = 0;

 int g_storage_reserved_mb = 0;
 FDFSGroups g_groups;