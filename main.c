#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "logger.h"
#include "sockopt.h"
#include "tracker_func.h"
#include "shared_func.h"
#include "fdfs_global.h"
#include "tracker_service.h"
#include "tracker_mem.h"

bool bReloadFlag = false;

void sigHupHandler(int);
void sigIntHandler(int);
void sigTermHandler(int);
void sigQuitHandler(int);

int main(int argc, char const *argv[])
{
	char *conf_filename;
	char bind_addr[FDFS_IPADDR_SIZE];
	pthread_attr_t pthd_attr;
	int comesock;

	int result;
	int sock;
	pthread_t pid;

	if (argc <2)
	{
		printf("Usage: fdfs_tracker <conf_file_name\n");
		return 1;
	}
	conf_filename = argv[1];
	memset(bind_addr,0,sizeof(bind_addr));

	if ((result = tracker_load_from_conf_file(conf_filename,bind_addr,sizeof(bind_addr))) !=0)
	{
		printf("tracker_load_from_conf_file failed!\n");
		return result;
	}

	if ((result = check_and_mk_log_dir()) != 0)
	{
		printf("check_and_mk_dir_log failed!\n");
		return result;
	}

	sock = socketServer(bind_addr,g_server_port,TRACKER_ERROR_LOG_FILENAME);
	if (sock <0)
	{
		printf("socketServer failed!\n");
		return 5;
	}

	daemon_init(false);
	umask(0);

	if ((result = init_pthread_lock(&g_tracker_thread_lock)) !=0)
	{
		printf("init_pthread_lok failed!\n");
		return result;
	}

	g_tracker_thread_count =0;
	pthread_attr_init(&pthd_attr);
	pthread_attr_setdetachstate(&pthd_attr,PTHREAD_CREATE_DETACHED);

	signal(SIGINT,sigIntHandler);
	signal(SIGQUIT,sigQuitHandler);
	signal(SIGTERM,sigTermHandler);
	signal(SIGHUP,sigHupHandler);
	signal(SIGPIPE,SIG_IGN);

	while(g_continue_flag)
	{
		comesock = nbaccept(sock,60,&result);
		if (comesock <0)
		{
			printf("nbaccept failed!\n");
			break;
		}

		if (pthread_mutex_lock(&g_tracker_thread_lock) !=0)
		{
			printf("pthread_mutex_lock failed!\n");
			break;
		}
		if (g_tracker_thread_count > g_max_connections)
		{
			printf("g_tracker_thread_count exceed g_max_connections\n");
			close(sock);
		}
		else
		{
			if (2 > g_tracker_thread_count)
			{
				result = pthread_create(&pid,&pthd_attr,tracker_thread_entrance,(void*)comesock);
				if (0 != result)
				{
					printf("create pthread failed!\n");
					close(comesock);
				}
				else
				{
					g_tracker_thread_count++;
				}
			}
			else
				printf("g_tracker_thread_count : %d\n",g_tracker_thread_count);

		}
		if (pthread_mutex_unlock(&g_tracker_thread_lock) !=0)
		{
			printf("pthread_mutex_unlock failed!\n");
			break;
		}

		printf("g_continue_flag. main sleep.... \n");
		sleep(3);
	}

	while(g_tracker_thread_count !=0)
	{
		sleep(1);
	}

	tracker_mem_destroy();
	pthread_attr_destroy(&pthd_attr);
	pthread_mutex_destroy(&g_tracker_thread_lock);

	printf("exit normally\n");

	return 0;
}

void sigQuitHandler(int sig)
{
	g_continue_flag = false;
}

void sigIntHandler(int sig)
{
	g_continue_flag = false;
}

void sigHupHandler(int sig)
{
	bReloadFlag = true;
}

void sigTermHandler(int sig)
{
	;
}