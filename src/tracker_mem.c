#include "tracker_mem.h"
#include <stdio.h>

int tracker_mem_init()
{

#ifdef __DEBUG__
	printf("tracker_mem_init to be finish! ===>>>\
			file:%s, line:%d\n",__FILE__,__LINE__);
#endif

	return 0;
}

int tracker_mem_destroy()
{
	printf("tracker_mem_destroy done!\n");

	return 0;
}