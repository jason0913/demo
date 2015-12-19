#ifndef __FDFS_GLOBALH
#define __FDFS_GLOBALH

#include "fdfs_define.h"

 extern int g_server_port;
 extern int g_max_connections;

 extern bool g_continue_flag;

 extern pthread_mutex_t g_tracker_thread_lock;
 extern int g_tracker_thread_count ;

#endif