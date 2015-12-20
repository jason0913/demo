#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include "tracker_mem.h"
#include "shared_func.h"
#include "tracker_types.h"
#include "fdfs_global.h"
#include "logger.h"

static pthread_mutex_t mem_thread_lock;

#define STORAGE_GROUPS_LIST_FILENAME	"storage_groups.dat"
#define STORAGE_SERVERS_LIST_FILENAME	"storage_servers.dat"
#define STORAGE_DATA_FIELD_SEPERATOR	','

#define TRACKER_MEM_ALLOC_ONCE	5

static int tracker_mem_cmp_by_group_name(const void *p1, const void *p2)
{
	return strcmp((*((FDFSGroupInfo **)p1))->group_name,
			(*((FDFSGroupInfo **)p2))->group_name);
}

FDFSGroupInfo *tracker_mem_get_group(const char *group_name)
{
	FDFSGroupInfo target_groups;
	FDFSGroupInfo *pTargetGroups;
	FDFSGroupInfo **ppGroup;

	memset(&target_groups,0,sizeof(target_groups));
	strcpy(target_groups.group_name,group_name);

	pTargetGroups = &target_groups;
	ppGroup = (FDFSGroupInfo **)bsearch(&pTargetGroups, \
			g_groups.sorted_groups, \
			g_groups.count, sizeof(FDFSGroupInfo *), \
			tracker_mem_cmp_by_group_name);
	if (NULL != ppGroup)
	{
		return *ppGroup;
	}
	else
		return NULL;
}

int tracker_mem_realloc_groups()
{

	FDFSGroupInfo *old_groups;
	FDFSGroupInfo **old_sorted_groups;
	FDFSGroupInfo *new_groups;
	FDFSGroupInfo **new_sorted_groups;
	int new_size;
	FDFSGroupInfo *pGroup;
	FDFSGroupInfo *pEnd;
	FDFSGroupInfo **ppSrcGroup;
	FDFSGroupInfo **ppDestGroup;
	FDFSGroupInfo **ppEnd;
	int *new_ref_count;

	new_size = g_groups.alloc_size +TRACKER_MEM_ALLOC_ONCE;
	new_groups = (FDFSGroupInfo *)malloc(new_size * sizeof(FDFSGroupInfo));

	if (NULL == new_groups)
	{
		return errno != 0 ? errno : ENOMEM;
	}

	new_sorted_groups = (FDFSGroupInfo **)malloc(sizeof(FDFSGroupInfo *) * new_size);
	if (NULL == new_sorted_groups)
	{
		free(new_groups);
		return errno != 0 ? errno : ENOMEM;
	}

	new_ref_count = (int *)malloc(sizeof(int));
	if (NULL == new_ref_count)
	{
		free(new_groups);
		free(new_sorted_groups);
		return errno != 0 ? errno : ENOMEM;
	}

	memset(new_groups,0,sizeof(FDFSGroupInfo)*new_size);
	memcpy(new_groups, g_groups.groups, \
		sizeof(FDFSGroupInfo) * g_groups.count);

	memset(new_sorted_groups,0,sizeof(FDFSGroupInfo *)*new_size);
	ppDestGroup = new_sorted_groups;
	ppEnd = g_groups.sorted_groups + g_groups.count;
	for (ppSrcGroup = g_groups.sorted_groups; ppSrcGroup <ppEnd; ppSrcGroup++)
	{
		*ppDestGroup++ = new_groups + (*ppSrcGroup - g_groups.groups);
	}

	*new_ref_count = 0;
	pEnd = new_groups + new_size;
	for (pGroup = new_groups ; pGroup < pEnd; pGroup++)
	{
		pGroup->ref_count = new_ref_count;
	}

	old_groups = g_groups.groups;
	old_sorted_groups = g_groups.sorted_groups;
	g_groups.alloc_size = new_size;
	g_groups.groups = new_groups;
	g_groups.sorted_groups = new_sorted_groups;

	if (FDFS_STORE_LOOKUP_SPEC_GROUP == g_groups.store_lookup)
	{
		g_groups.pStoreGroup = tracker_mem_get_group( \
					g_groups.store_group);
	}

	sleep(1);

	if (*(old_groups[0].ref_count) <= 0)
	{
		free(old_groups[0].ref_count);
		free(old_groups);
	}
	else
	{
		pEnd = old_groups + g_groups.count;
		for (pGroup=old_groups; pGroup<pEnd; pGroup++)
		{
			pGroup->dirty = true;
		}
	}

	free(old_sorted_groups);

	return 0;
}

static int tracker_mem_init_group(FDFSGroupInfo *pGroup)
{
	int *ref_count;
	FDFSStorageDetail *pServer;
	FDFSStorageDetail *pEnd;

	pGroup->alloc_size =TRACKER_MEM_ALLOC_ONCE;
	pGroup->count =0;
	pGroup->all_servers = (FDFSStorageDetail *) \
			malloc(sizeof(FDFSStorageDetail) * pGroup->alloc_size);
	if (NULL == pGroup->all_servers)
	{
		return errno != 0 ? errno : ENOMEM;
	}

	memset(pGroup->all_servers,0,sizeof(FDFSStorageDetail) *pGroup->alloc_size);
	pGroup->sorted_servers = (FDFSStorageDetail **) \
		malloc(sizeof(FDFSStorageDetail *) * pGroup->alloc_size);
	if (NULL == pGroup->sorted_servers)
	{
		return errno != 0 ? errno:ENOMEM;
	}
	memset(pGroup->sorted_servers,0,sizeof(FDFSStorageDetail *) * pGroup->alloc_size);
	pGroup->active_servers = (FDFSStorageDetail **) \
		malloc(sizeof(FDFSStorageDetail *) * pGroup->alloc_size);
	if (NULL == pGroup->active_servers)
	{
		return errno != 0 ? errno: ENOMEM;
	}

	memset(pGroup->active_servers, 0, \
		sizeof(FDFSStorageDetail *) * pGroup->alloc_size);

	ref_count = (int *) malloc(sizeof(int));
	if (NULL == ref_count)
	{
		return errno != 0 ? errno :ENOMEM;
	}

	*ref_count = 0;
	pEnd = pGroup->all_servers +pGroup->alloc_size;
	for (pServer = pGroup->all_servers; pServer < pEnd; pServer++)
	{
		pServer->ref_count = ref_count;
	}

	return 0;
}

static void tracker_mem_insert_into_sorted_groups( \
		FDFSGroupInfo *pTargetGroup)
{
	FDFSGroupInfo **ppGroup;
	FDFSGroupInfo **ppEnd;

	ppEnd = g_groups.sorted_groups + g_groups.count;
	for (ppGroup = ppEnd; ppGroup > g_groups.sorted_groups; ppGroup--)
	{
		if (strcmp(pTargetGroup->group_name, \
			   (*(ppGroup-1))->group_name) > 0)
		{
			*ppGroup = pTargetGroup;
			return;
		}
		else
		{
			*ppGroup = *(ppGroup -1);
		}
	}

	*ppGroup = pTargetGroup;
}

int tracker_mem_add_group(TrackerClientInfo *pClientInfo, \
			const bool bIncRef, bool *bInserted)
{
	FDFSGroupInfo *pGroup;
	int result;

	*bInserted = false;
	pGroup = tracker_mem_get_group(pClientInfo->group_name);
	if (NULL != pGroup)
	{
	#ifdef __DEBUG__
		printf("g_groups.count=%d, found %s\n,file:%s,line:%d\n",\
			 g_groups.count, pClientInfo->group_name,__FILE__,__LINE__);
	#endif
	}
	else
	{
		if (0 != pthread_mutex_lock(&mem_thread_lock))
		{
			logError("file: %s, line: %d, " \
				"call pthread_mutex_lock fail, " \
				"errno: %d, error info: %s", \
				__FILE__,__LINE__, errno, strerror(errno));

			return errno != 0 ? errno : EAGAIN;
		}
		result =0;
		while(1)
		{
			if (g_groups.count >= g_groups.alloc_size)
			{
				result = tracker_mem_realloc_groups();
				if (result != 0)
				{
				#ifdef __DEBUG__
					printf("tracker_mem_realloc_groups failed,file:%s,line:%d\n", __FILE__,__LINE__);
				#endif
					break;
				}
			}
			pGroup = g_groups.groups + g_groups.count;
			result = tracker_mem_init_group(pGroup);
			if (result != 0)
			{
			#ifdef __DEBUG__
				printf("tracker_mem_init_group failed,file:%s,line:%d\n", __FILE__,__LINE__);
			#endif
				break;
			}
			strcpy(pGroup->group_name,pClientInfo->group_name);
			tracker_mem_insert_into_sorted_groups(pGroup);
			g_groups.count++;

			if ((g_groups.store_lookup == \
				FDFS_STORE_LOOKUP_SPEC_GROUP) && \
				(g_groups.pStoreGroup == NULL) && \
				(strcmp(g_groups.store_group, \
					pGroup->group_name) == 0))
			{
				g_groups.pStoreGroup = pGroup;
			}
			break;
		}
		if (0 != pthread_mutex_unlock(&mem_thread_lock))
		{
			logError("file: %s, line: %d, " \
				"call pthread_mutex_unlock fail, " \
				"errno: %d, error info: %s", \
				__FILE__,__LINE__, errno, strerror(errno));

			return errno != 0 ? errno : EAGAIN;
		}

		if (0 != result)
		{
			return result;
		}
		*bInserted = true;

	}

	if (bIncRef)
	{
		++(*(pGroup->ref_count));

	#ifdef __DEBUG__
		printf("group ref_count=%d,file:%s,line:%d\n",\
		 	*(pGroup->ref_count),__FILE__,__LINE__);
	#endif
	}
	pClientInfo->pGroup = pGroup;
	pClientInfo->pAllocedGroups = g_groups.groups;

#ifdef __DEBUG__
	printf("tracker_mem_add_group to be implete! ===>>>>,\
			file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}

static int tracker_load_groups(const char *data_path)
{
#define STORAGE_DATA_GROUP_FIELDS	2

	FILE *fp;
	char szLine[256];
	char *fields[STORAGE_DATA_GROUP_FIELDS];
	int result;
	TrackerClientInfo clientInfo;
	bool bInserted;

	if (NULL == (fp = fopen(STORAGE_GROUPS_LIST_FILENAME,"r")))
	{
		logError("file: %s, line: %d, " \
			"open file \"%s/%s\" fail, " \
			"errno: %d, error info: %s", \
			__FILE__,__LINE__, data_path, STORAGE_GROUPS_LIST_FILENAME, \
			errno, strerror(errno));

		return errno != 0 ? errno : ENOENT;
	}

	result =0;
	while(fgets(szLine, sizeof(szLine), fp) != NULL)
	{
		if ('\0' == *szLine)
		{
			continue;
		}
		if (splitEx(szLine, STORAGE_DATA_FIELD_SEPERATOR, \
			fields, STORAGE_DATA_GROUP_FIELDS) != \
				STORAGE_DATA_GROUP_FIELDS)
		{
			logError("file: %s, line: %d, " \
				"the format of the file \"%s/%s\" is invalid", \
				__FILE__,__LINE__, data_path, \
				STORAGE_GROUPS_LIST_FILENAME);

			result = errno != 0 ? errno : EINVAL;
			break;
		}

		memset(&clientInfo,0,sizeof(TrackerClientInfo));
		snprintf(clientInfo.group_name,sizeof(clientInfo.group_name),"%s",trim(fields[0]));
		if (0 != (result=tracker_mem_add_group(&clientInfo, \
				false, &bInserted)))
		{
			break;
		}

		if (!bInserted)
		{
			logError("file: %s, line: %d, " \
				"in the file \"%s/%s\", " \
				"group \"%s\" is duplicate", \
				__FILE__,__LINE__, data_path, \
				STORAGE_GROUPS_LIST_FILENAME, \
				clientInfo.group_name);

			result = errno != 0 ? errno : EEXIST;
			break;
		}
		clientInfo.pGroup->storage_port = atoi(trim(fields[1]));
	}
	fclose(fp);
	return result;
}

static int tracker_load_storages(const char *data_path)
{

#ifdef __DEBUG__
	printf("tracker_load_storages to be implete! ===>>>>,\
			file:%s,line:%d\n", __FILE__,__LINE__);
#endif
	return 0;
}
static int tracker_load_data()
{
	char data_path[MAX_PATH_SIZE];
	int result;

	snprintf(data_path,MAX_PATH_SIZE,"%s/data",g_base_path);
	if (!fileExists(data_path))
	{
		if (mkdir(data_path, 0755) != 0)
		{
			logError("file: %s, line: %d, " \
				"mkdir \"%s\" fail, " \
				"errno: %d, error info: %s", \
				__FILE__,__LINE__, data_path, errno, strerror(errno));

			return errno != 0 ? errno : ENOENT;
		}
	}

	if (chdir(data_path) != 0)
	{
		logError("file: %s, line: %d, " \
			"chdir \"%s\" fail, " \
			"errno: %d, error info: %s", \
			__FILE__,__LINE__, data_path, errno, strerror(errno));

		return errno != 0 ? errno : ENOENT;
	}

	if (!fileExists(STORAGE_GROUPS_LIST_FILENAME))
	{
		return 0;
	}

	if (0 != (result = tracker_load_groups(data_path)))
	{
		return result;
	}
	if (0 != (result = tracker_load_storages(data_path)))
	{
		return result;
	}

#ifdef __DEBUG__
	printf("tracker_load_data done! file:%s,line:%d\n", \
				__FILE__,__LINE__);
#endif
	return 0;
}

int tracker_mem_init()
{

	FDFSGroupInfo *pGroup;
	FDFSGroupInfo *pEnd;
	int *ref_count;
	int result;

	if (0 != (result = init_pthread_lock(&mem_thread_lock)))
	{

		return result;
	}

	g_groups.alloc_size = TRACKER_MEM_ALLOC_ONCE;
	g_groups.count = 0;
	g_groups.current_write_group = 0;
	g_groups.pStoreGroup = NULL;
	g_groups.groups = (FDFSGroupInfo *)malloc( \
			sizeof(FDFSGroupInfo) * g_groups.alloc_size);
	if (NULL == g_groups.groups)
	{
		return errno != 0 ? errno : ENOMEM;
	}

	memset(g_groups.groups,0,sizeof(FDFSGroupInfo) *g_groups.alloc_size);
	ref_count = (int *) malloc(sizeof(int));
	if (NULL == ref_count)
	{
		free(g_groups.groups);
		g_groups.groups = NULL;
		return errno != 0 ? errno : ENOMEM;
	}

	*ref_count =0;
	pEnd = g_groups.groups + g_groups.alloc_size;
	for (pGroup = g_groups.groups; pGroup < pEnd; pGroup++)
	{
		pGroup->ref_count = ref_count;
	}

	g_groups.sorted_groups = (FDFSGroupInfo **) \
			malloc(sizeof(FDFSGroupInfo *) * g_groups.alloc_size);
	if (NULL == g_groups.sorted_groups)
	{
		free(g_groups.groups);
		g_groups.groups = NULL;
		return errno != 0 ? errno : ENOMEM;
	}

	memset(g_groups.sorted_groups, 0, \
		sizeof(FDFSGroupInfo *) * g_groups.alloc_size);

	if (0 != (result = tracker_load_data()))
	{
		return result;
	}

#ifdef __DEBUG__
	printf("tracker_mem_init done!\
			file:%s, line:%d\n",__FILE__,__LINE__);
#endif

	return 0;
}

int tracker_mem_destroy()
{
	printf("tracker_mem_destroy done!\n");

	return 0;
}