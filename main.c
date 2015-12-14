#include <stdio.h>
#include "logger.h"
#include "sockopt.h"
#include "tracker_func.h"
#include "shared_func.h"



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
		exit(-1);
	}
	conf_filename = argv[1];
	memset(bind_addr,0,sizeof(bind_addr));

	if ((result = tracker_load_from_conf_file(conf_filename,bind_addr,sizeof(bind_addr))) !=0)
	{
		printf("tracker_load_from_conf_file failed!\n");
		return result;
	}

	return 0;
}