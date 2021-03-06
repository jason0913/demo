#ifndef __SHARED_FUNC
#define __SHARED_FUNC

#include <pthread.h>
#include "fdfs_define.h"

extern void daemon_init(bool bCloseFile);
extern int init_pthread_lock(pthread_mutex_t * pthread_lock);

extern bool fileExists(const char *filename);
extern void chopPath(char *filePath);
extern bool isDir(const char *filename);

extern char * trim_right(char *pStr);
extern char * trim_left(char *pStr);
extern char *trim(char *pStr);
extern int splitEx(char *src, const char seperator, char **pCols, const int nMaxCols);

#endif