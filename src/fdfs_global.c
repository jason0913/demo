#include <pthread.h>
#include "fdfs_define.h"

 int g_server_port = FDFS_STORAGE_SERVER_DEF_PORT;
 int g_max_connections = FDFS_DEF_MAX_CONNECTONS;

 bool g_continue_flag = true;
 char g_base_path[MAX_PATH_SIZE];

 pthread_mutex_t g_tracker_thread_lock;
 int g_tracker_thread_count = 0;