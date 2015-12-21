#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include "tracker_func.h"
#include "logger.h"
#include "shared_func.h"
#include "ini_file_reader.h"
#include "fdfs_global.h"
#include "tracker_proto.h"

static int tracker_load_store_lookup(const char *filename, \
		IniItemInfo *items, const int nItemCount)
{
	char *pGroupName;

	g_groups.store_lookup = iniGetIntValue("store_lookup", \
			items, nItemCount, FDFS_STORE_LOOKUP_ROUND_ROBIN);
	if (FDFS_STORE_LOOKUP_ROUND_ROBIN == g_groups.store_lookup)
	{
		g_groups.store_group[0] = '\0';
		return 0;
	}

	if (FDFS_STORE_LOOKUP_LOAD_BALANCE == g_groups.store_lookup)
	{
		g_groups.store_group[0] = '\0';
		return 0;
	}

	if (FDFS_STORE_LOOKUP_SPEC_GROUP != g_groups.store_lookup)
	{
		logError("file: %s, line: %d, " \
			"conf file \"%s\", the value of \"store_lookup\" is " \
			"invalid, value=%d!", \
			__FILE__,__LINE__, filename, g_groups.store_lookup);

		return EINVAL;
	}

	pGroupName = iniGetStrValue("store_group", items, nItemCount);
	if (NULL == pGroupName)
	{
		logError("file: %s, line: %d, " \
			"conf file \"%s\", must have items ", \
				__FILE__,__LINE__, filename);
		return ENOENT;
	}
	if ('\0' == pGroupName)
	{
		logError("file: %s, line: %d, " \
			"conf file \"%s\", store_group is empty", \
				__FILE__,__LINE__, filename);
		return EINVAL;
	}
	snprintf(g_groups.store_group, sizeof(g_groups.store_group), \
			"%s", pGroupName);
	if (0 != tracker_validate_group_name(g_groups.store_group))
	{
		logError("file: %s, line: %d, " \
			"conf file \"%s\", " \
			"the group name \"%s\" is invalid!", \
			__FILE__,__LINE__, filename, g_groups.store_group);
		return EINVAL;
	}
	return 0;
}

int tracker_load_from_conf_file(const char *conf_filename,char *bind_addr,const int addr_size)
{

	char * pBasePath;
	char *pBindAddr;
	char *pStorageReserved;
	char *pReservedEnd;
	IniItemInfo *items;
	int nItemCount;
	int result;

	if (0 != (result=iniLoadItems(conf_filename,&items,&nItemCount)))
	{
		logError("file:%s,line:%d," \
			"load from \"%s\" fail, ret code %d",
			__FILE__,__LINE__,conf_filename,result);

		return result;
	}

	while(1)
	{
		if (iniGetBoolValue("disabled", items, nItemCount))
		{
			logError("file:%s,line:%d," \
			"conf file \"%s\" disabled = true,exit",
			__FILE__,__LINE__,conf_filename);

			result = ECANCELED;
			break;
		}

		pBasePath = iniGetStrValue("base_path",items,nItemCount);
		if (NULL == pBasePath)
		{
			logError("file:%s,line:%d," \
			"conf file \"%s\" no has items,exit",
			__FILE__,__LINE__,conf_filename);

			result = ENOENT;
			break;
		}

		snprintf(g_base_path,sizeof(g_base_path),"%s",pBasePath);
		chopPath(g_base_path);
		if (!fileExists(g_base_path))
		{
			logError("file:%s,line:%d," \
			"can access file \"%s\" errno = %d,err info = %s",
			__FILE__,__LINE__,g_base_path,errno,strerror(errno));

			result = errno != 0 ? errno : ENOENT;
			break;
		}
		if (!isDir(g_base_path))
		{
			logError("file:%s,line:%d," \
			"\"%s\" is no dir\n",
			__FILE__,__LINE__,g_base_path,errno);

			result = ENOTDIR;
			break;
		}

		g_network_timeout = iniGetIntValue("network_tmeout",items,nItemCount,\
			DEFAULT_NETWORK_TIMEOUT);
		if (g_network_timeout <= 0)
		{
			g_network_timeout = DEFAULT_NETWORK_TIMEOUT;
		}

		g_server_port = iniGetIntValue("port",items,nItemCount,FDFS_TRACKER_SERVER_DEF_PORT);
		if (g_server_port <= 0)
		{
			g_server_port = FDFS_TRACKER_SERVER_DEF_PORT;
		}

		pBindAddr = iniGetStrValue("bind_addr", items, nItemCount);
		if (pBindAddr == NULL)
		{
			bind_addr[0] = '\0';
		}
		else
		{
			snprintf(bind_addr, addr_size, "%s", pBindAddr);
		}

		if (0 !=(result=tracker_load_store_lookup(conf_filename, \
			items, nItemCount)))
		{
			break;
		}

		pStorageReserved = iniGetStrValue("reserved_storage_space",items,nItemCount);
		if (NULL == pStorageReserved)
		{
			g_storage_reserved_mb = FDFS_DEF_STORAGE_RESERVED_MB;
		}
		else
		{
			pReservedEnd = NULL;
			g_storage_reserved_mb = strtol(pStorageReserved,&pReservedEnd,10);
			if (g_storage_reserved_mb <0)
			{
				g_storage_reserved_mb = 0;
			}

			if (g_storage_reserved_mb > 0)
			{
				if (NULL == pReservedEnd || '\0' == *pReservedEnd)
				{
					g_storage_reserved_mb /= FDFS_ONE_MB;
				}
				else if ('G' == *pReservedEnd || 'g' == *pReservedEnd)
				{
					g_storage_reserved_mb *= 1024;
				}
				else if ('M' == *pReservedEnd || 'm' == *pReservedEnd)
				{
					;
				}
				else if ('k' == *pReservedEnd || 'K' == *pReservedEnd)
				{
					g_storage_reserved_mb /= 1024;
				}
				else
				{
					g_storage_reserved_mb /= FDFS_ONE_MB;
				}
			}
		}

		g_max_connections = iniGetIntValue("max_connections", \
				items, nItemCount, FDFS_DEF_MAX_CONNECTONS);
		if (g_max_connections <=0)
		{
			g_max_connections = FDFS_DEF_MAX_CONNECTONS;
		}

		logInfo(TRACKER_ERROR_LOG_FILENAME, \
			"FastDFS v%d.%d, base_path=%s, " \
			"network_timeout=%d, "    \
			"port=%d, bind_addr=%s, " \
			"max_connections=%d, "    \
			"store_lookup=%d, store_group=%s, " \
			"reserved_storage_space=%dMB", \
			g_version.major, g_version.minor,  \
			g_base_path, \
			g_network_timeout, \
			g_server_port, bind_addr, g_max_connections, \
			g_groups.store_lookup, g_groups.store_group, \
			g_storage_reserved_mb);

		break;
	}

	iniFreeItems(items);

#ifdef __DEBUG__
	printf("tracker_load_from_conf_file done! \
		file:%s,line:%d,\n",__FILE__,__LINE__);
#endif

	return result;
}