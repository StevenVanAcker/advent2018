#ifndef __RESPONSES_H__
#define __RESPONSES_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tools.h"
#include "crypto.h"
#include "udpsanta.pb-c.h"

extern ResponseInner *respinner_base(Command, char *, char *, int, uint8_t *, size_t);
#define respinner_GETKEY(key, iv) 					respinner_base(COMMAND__GETKEY, key, iv, 0, NULL, 0)
#define respinner_SENDMSG(success)					respinner_base(COMMAND__SENDMSG, NULL, NULL, success, NULL, 0)
#define respinner_LISTMSGS(result, resultlen)		respinner_base(COMMAND__LISTMSGS, NULL, NULL, 0, result, resultlen)
#define respinner_GETMSG(username, message) 		respinner_base(COMMAND__GETMSG, username, message, 0, NULL, 0)
#define respinner_ERROR(message)					respinner_base(COMMAND__ERROR, message, NULL, 0, NULL, 0)

extern void respinner_free(ResponseInner *);

extern void dumpInnerResponseToFile(ResponseInner *, char *);
extern ResponseInner *readInnerResponseFromFile(char *);
extern ResponseInner *parseResponseInner(uint8_t *, size_t);
extern Response *parseResponse(uint8_t *, size_t);
extern void debugResponseInner(ResponseInner *);
extern void debugResponse(Response *);

extern ResponseInner *getResponseInner(Response *, char *, char *, char *);

extern Response *resp_base(char *, char *, char *, char *, ResponseInner *, int);
#if 1
#define resp_GETKEY(replyto, pubkey, key, iv) \
	resp_base(replyto, pubkey, key, iv, respinner_GETKEY(key, iv), 1)
#define resp_SENDMSG(replyto, key, iv, success) \
	resp_base(replyto, NULL, key, iv, respinner_SENDMSG(success), 1)
#define resp_LISTMSGS(replyto, key, iv, data, datalen) \
	resp_base(replyto, NULL, key, iv, respinner_LISTMSGS(data, datalen), 1)
#define resp_GETMSG(replyto, key, iv, username, message) \
	resp_base(replyto, NULL, key, iv, respinner_GETMSG(username, message), 1)
#define resp_ERROR(replyto, key, iv, message)	\
	resp_base(replyto, NULL, key, iv, respinner_ERROR(message), 1)
#endif

extern void resp_free(Response *);

#endif /* __RESPONSES_H__ */
