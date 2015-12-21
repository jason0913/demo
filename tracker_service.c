#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include "tracker_service.h"
#include "fdfs_define.h"
//#include "fdfs_global.h"

extern bool g_continue_flag;

void* tracker_thread_entrance(void* arg)
{
	printf("tracker_thread_entrance done!\n");

	while(g_continue_flag)
	{
		printf("thread id %x\n", pthread_self());
		sleep(2);
	}
}