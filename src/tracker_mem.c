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

int tracker_mem_add_group(TrackerClientInfo *pClientInfo, \
			const bool bIncRef, bool *bInserted)
{

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