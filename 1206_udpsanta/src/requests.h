#ifndef __REQUESTS_H__
#define __REQUESTS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tools.h"
#include "crypto.h"
#include "udpsanta.pb-c.h"


/* RequestInner messages */
extern RequestInner *reqinner_base(Command, char *, char *, size_t, size_t);
#define reqinner_GETKEY(pubkey)		reqinner_base(COMMAND__GETKEY, pubkey, NULL, 0, 0)
#define reqinner_SENDMSG(to, msg)	reqinner_base(COMMAND__SENDMSG, to, msg, 0, 0)
#define reqinner_LISTMSGS(count)	reqinner_base(COMMAND__LISTMSGS, NULL, NULL, count, 1)
#define reqinner_GETMSG(id)			reqinner_base(COMMAND__GETMSG, NULL, NULL, id, 1)

extern void reqinner_free(RequestInner *);

extern void dumpInnerRequestToFile(RequestInner *, char *);
extern RequestInner *readInnerRequestFromFile(char *);

extern void debugRequestInner(RequestInner *);
extern void debugRequest(Request *);
extern RequestInner *parseRequestInner(uint8_t *, size_t);
extern Request *parseRequest(uint8_t *, size_t);
extern RequestInner *getRequestInner(Request *, char *, char *);

/* Request messages */
extern Request *req_base(char *, char *, char *, RequestInner *, int);
#define req_GETKEY(from, pubkey) 				req_base(from, NULL, NULL, reqinner_GETKEY(pubkey), 1)
#define req_SENDMSG(from, key, iv, to, msg) 	req_base(from, key, iv, reqinner_SENDMSG(to, msg), 1)
#define req_LISTMSGS(from, key, iv, count)		req_base(from, key, iv, reqinner_LISTMSGS(count), 1)
#define req_GETMSG(from, key, iv, id)			req_base(from, key, iv, reqinner_GETMSG(id), 1)

extern void req_free(Request *);

extern char *getRequestHash(Request *);


#endif /* __REQUESTS_H__ */
