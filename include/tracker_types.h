#ifndef __TRACKER_TYPES_H_
#define __TRACKER_TYPES_H_

#include <time.h>
#include "fdfs_define.h"

#define FDFS_ONE_MB	(1024 * 1024)

#define FDFS_GROUP_NAME_MAX_LEN		16

#define FDFS_STORE_LOOKUP_ROUND_ROBIN	0  //round robin
#define FDFS_STORE_LOOKUP_SPEC_GROUP	1  //specify group
#define FDFS_STORE_LOOKUP_LOAD_BALANCE	2  //load balance

typedef struct
{
	char sz_total_upload_count[4];
	char sz_success_upload_count[4];
	char sz_total_set_meta_count[4];
	char sz_success_set_meta_count[4];
	char sz_total_delete_count[4];
	char sz_success_delete_count[4];
	char sz_total_download_count[4];
	char sz_success_download_count[4];
	char sz_total_get_meta_count[4];
	char sz_success_get_meta_count[4];
	char sz_last_source_update[4];
	char sz_last_sync_update[4];
} FDFSStorageStatBuff;

typedef struct
{
	int total_upload_count;
	int success_upload_count;
	int total_set_meta_count;
	int success_set_meta_count;
	int total_delete_count;
	int success_delete_count;
	int total_download_count;
	int success_download_count;
	int total_get_meta_count;
	int success_get_meta_count;
	time_t last_source_update;
	time_t last_sync_update;
	/*
	int total_check_count;
	int success_check_count;
	time_t last_check_time;
	*/
} FDFSStorageStat;

typedef struct StructFDFSStorageDetail
{
	char status;
	bool dirty;
	char ip_addr[FDFS_IPADDR_SIZE];

	struct StructFDFSStorageDetail *psync_src_server;
	int sync_until_timestamp;

	int total_mb;  //total disk storage in MB
	int free_mb;  //free disk storage in MB

	int *ref_count;   //group/storage servers referer count
	int version;      //current server version
	FDFSStorageStat stat;
} FDFSStorageDetail;

typedef struct
{
	bool dirty;
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	int free_mb;  //free disk storage in MB
	int alloc_size;
	int count;    //server count
	int storage_port;
	FDFSStorageDetail *all_servers;
	FDFSStorageDetail **sorted_servers;  //order by addr
	int active_count;
	FDFSStorageDetail **active_servers;  //order by addr
	int current_read_server;
	int current_write_server;
	int *ref_count;  //groups referer count
	int version;     //current group version
	time_t last_source_update;
	time_t last_sync_update;
} FDFSGroupInfo;

typedef struct
{
	int alloc_size;
	int count;
	FDFSGroupInfo *groups;
	FDFSGroupInfo **sorted_groups; //order by group_name
	FDFSGroupInfo *pStoreGroup;
	int current_write_group;
	byte store_lookup;
	char store_group[FDFS_GROUP_NAME_MAX_LEN + 1];
} FDFSGroups;

typedef struct
{
	int sock;
	int storage_port;
	char ip_addr[FDFS_IPADDR_SIZE];
	char group_name[FDFS_GROUP_NAME_MAX_LEN + 1];
	FDFSGroupInfo *pGroup;
	FDFSStorageDetail *pStorage;
	FDFSGroupInfo *pAllocedGroups;		//for free
	FDFSStorageDetail *pAllocedStorages;	//for free
} TrackerClientInfo;

#endif