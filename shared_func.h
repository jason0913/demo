#ifndef __SHARED_FUNC
#define __SHARED_FUNC

extern void daemon_init(bool bCloseFile);
extern int init_pthread_lock(pthread_mutex_lock *);

#endif