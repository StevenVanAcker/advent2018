#ifndef __USERDATA_H__
#define __USERDATA_H__

#include "tools.h"

#define NAMESIZE 	16
#define KEYSIZE 	(256/8)
#define IVSIZE 		(128/8)

#define MAXUSERS 20
#define MSGSLISTSIZE 20

#define MSGLEN 500

typedef struct userdata_t {
	char name[NAMESIZE+1];
	char *pubkey;
	char key[KEYSIZE+1];
	char iv[IVSIZE+1];
} UserKey;

typedef struct usermsg_t {
	char name[NAMESIZE+1];
	char from[NAMESIZE+1];
	uint32_t id;
	char msg[MSGLEN+1];
} UserMsg;

typedef struct alldata_t {
	UserMsg embedMsgStore[MAXUSERS * MSGSLISTSIZE];
	uint32_t embedMsgIdList[MSGSLISTSIZE];
	UserKey embedKeyStore[MAXUSERS];
} AllDataBlob;

extern AllDataBlob AllData;
extern UserMsg *globMsgStore;
extern UserKey *globKeyStore;
extern uint32_t *globMsgIdList;


extern void read_userdata(char *, char *);
extern void dump_userdata();

extern void copy_msgids(char *);
extern UserMsg *get_user_msg(char *, uint32_t);
extern UserKey *get_user_key(char *);

#endif /* __USERDATA_H__ */
