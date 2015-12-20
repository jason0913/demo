#ifndef __FDFS_DEFINE
#define __FDFS_DEFINE

#include <pthread.h>

#define MAX_PATH_SIZE				256

#define DEFAULT_NETWORK_TIMEOUT			30
#define FDFS_IPADDR_SIZE 16
#define FDFS_TRACKER_SERVER_DEF_PORT		22000
#define FDFS_STORAGE_SERVER_DEF_PORT		23000
#define FDFS_DEF_MAX_CONNECTONS			256
#define FDFS_DEF_STORAGE_RESERVED_MB		1024

#define TRACKER_ERROR_LOG_FILENAME      "trackerd"

#ifndef true
typedef char bool;
#define true 1
#define false 0
#endif

#ifndef byte
#define byte char
#endif

#endif