#ifndef __TRACKER_PORTO_H_
#define __TRACKER_PORTO_H_

#define TRACKER_PROTO_CMD_STORAGE_JOIN          81
#define TRACKER_PROTO_CMD_STORAGE_QUIT          82
#define TRACKER_PROTO_CMD_STORAGE_BEAT          83  //heart beat
#define TRACKER_PROTO_CMD_STORAGE_REPORT        84
#define TRACKER_PROTO_CMD_STORAGE_REPLICA_CHG   85  //repl new storage servers
#define TRACKER_PROTO_CMD_STORAGE_SYNC_SRC_REQ  86  //src storage require sync
#define TRACKER_PROTO_CMD_STORAGE_SYNC_DEST_REQ 87  //dest storage require sync
#define TRACKER_PROTO_CMD_STORAGE_SYNC_NOTIFY   88  //sync notify
#define TRACKER_PROTO_CMD_STORAGE_RESP          80

#define TRACKER_PROTO_CMD_SERVER_LIST_GROUP	91
#define TRACKER_PROTO_CMD_SERVER_LIST_STORAGE	92
#define TRACKER_PROTO_CMD_SERVER_RESP      	93
#define TRACKER_PROTO_CMD_SERVICE_QUERY_STORE	101
#define TRACKER_PROTO_CMD_SERVICE_QUERY_FETCH	102
#define TRACKER_PROTO_CMD_SERVICE_RESP		100


#define TRACKER_PROTO_PKG_LEN_SIZE	9
#define TRACKER_PROTO_CMD_SIZE		1

typedef struct
{
	char pkg_len[TRACKER_PROTO_PKG_LEN_SIZE];
	char cmd;
	char status;
} TrackerHeader;

extern int tracker_validate_group_name(const char *group_name);

#endif