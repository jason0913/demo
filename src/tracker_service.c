#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "tracker_service.h"

void* tracker_thread_entrance(void* arg)
{
	printf("tracker_thread_entrance done!\n");

	while(1)
	{
		printf("thread id %x\n", pthread_self());
		sleep(2);
	}
}