#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
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
	conf_filename = (char *)argv[1];
	memset(bind_addr,0,sizeof(bind_addr));

	if ((result = tracker_load_from_conf_file(conf_filename,bind_addr,sizeof(bind_addr))) !=0)
	{
		printf("tracker_load_from_conf_file failed!\
				file:%s,line:%d\n",__FILE__,__LINE__);
		return result;
	}

	snprintf(g_error_file_prefix, sizeof(g_error_file_prefix), \
			"%s", TRACKER_ERROR_LOG_FILENAME);

	if ((result = check_and_mk_log_dir()) != 0)
	{
		printf("check_and_mk_dir_log failed!\
			file:%s,line:%d\n",__FILE__,__LINE__);
		return result;
	}

	if (0 != (result = tracker_mem_init()))
	{
		printf("tracker_mem_init failed!\
			file:%s,line:%d\n",__FILE__,__LINE__);
		return result;
	}

	sock = socketServer(bind_addr,g_server_port,TRACKER_ERROR_LOG_FILENAME);
	if (sock <0)
	{
		printf("socketServer failed!\
			file:%s,line:%d\n",__FILE__,__LINE__);
		return 5;
	}

	// daemon_init(false);
	umask(0);

	if ((result = init_pthread_lock(&g_tracker_thread_lock)) !=0)
	{
		printf("init_pthread_lok failed!\
			file:%s,line:%d\n",__FILE__,__LINE__);
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
			if (ETIMEDOUT == result || EINTR == result || EAGAIN == result)
			{
				printf("nbaccept timeout,continue, file:%s,line:%d\n",\
				 	__FILE__,__LINE__);
				continue;
			}
			if (EBADF == result)
			{
				logError("file: %s, line: %d, " \
					"accept failed, " \
					"errno: %d, error info: %s", \
					__FILE__,__LINE__, result, strerror(result));
				break;
			}

			logError("file: %s, line: %d, " \
					"accept failed, " \
					"errno: %d, error info: %s", \
					__FILE__,__LINE__, result, strerror(result));
			printf("nbaccept failed\n");
			continue;

		}

		if (pthread_mutex_lock(&g_tracker_thread_lock) !=0)
		{
			logError("file: %s, line: %d, " \
				"call pthread_mutex_lock fail, " \
				"errno: %d, error info:%s.", \
				__FILE__,__LINE__, errno, strerror(errno));

		#ifdef __DEBUG__
			printf("pthread_mutex_lock faile, file:%s,line:%d\n",\
					__FILE__,__LINE__);
		#endif
		}
		if (g_tracker_thread_count > g_max_connections)
		{
			logError("file: %s, line: %d, " \
				"create thread failed, " \
				"current thread count %d exceed the limit %d", \
				__FILE__,__LINE__, \
				g_tracker_thread_count + 1, g_max_connections);

		#ifdef __DEBUG__
			printf("g_tracker_thread_count exceed g_max_connections\n");
		#endif
			close(comesock);
		}
		else
		{
			result = pthread_create(&pid,&pthd_attr,tracker_thread_entrance,(void*)comesock);
			if (0 != result)
			{
				logError("file: %s, line: %d, " \
				"call pthread_create fail, " \
				"errno: %d, error info:%s.", \
				__FILE__,__LINE__, errno, strerror(errno));

				close(comesock);
			}
			else
			{
				g_tracker_thread_count++;
			}
		#ifdef __DEBUG__
			printf("g_tracker_thread_count : %d,file:%s,line:%d\n",\
					g_tracker_thread_count,__FILE__,__LINE__);
		#endif

		}
		if (pthread_mutex_unlock(&g_tracker_thread_lock) !=0)
		{
			logError("file: %s, line: %d, " \
				"call pthread_mutex_unlock fail, " \
				"errno: %d, error info:%s.", \
				__FILE__,__LINE__, errno, strerror(errno));

		#ifdef __DEBUG__
			printf("pthread_mutex_unlock file:%s,line:%d\n",\
					__FILE__,__LINE__);
		#endif
		}

		printf("g_continue_flag true, in main\n");

	}

	while(g_tracker_thread_count !=0)
	{

	#ifdef __DEBUG__
		printf("still has g_tracker_thread,sleep...\n");
	#endif
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