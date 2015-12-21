#ifndef __TRACKER_PORTO_H_
#define __TRACKER_PORTO_H_

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