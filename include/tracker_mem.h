#ifndef __TRACKER_MEM_H
#define __TRACKER_MEM_H

#include "fdfs_define.h"
#include "tracker_types.h"

extern int tracker_mem_init();
extern int tracker_mem_destroy();
extern int tracker_mem_add_group(TrackerClientInfo *pClientInfo, \
			const bool bIncRef, bool *bInserted);
extern FDFSGroupInfo *tracker_mem_get_group(const char *group_name);
#endif