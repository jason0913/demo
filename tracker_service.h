#ifndef __TRACKER_SERVICE_H_
#define __TRACKER_SERVICE_H_

extern pthread_mutex_t g_tracker_thread_lock;
extern g_tracker_thread_count;

void* tracker_thread_entrance(void* arg);
#endif