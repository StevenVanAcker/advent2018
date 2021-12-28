#include <stdio.h>
#include "config.h"
#include "tools.h"
#include "requests.h"
#include "responses.h"
#include "network.h"
#include "userdata.h"

int globDebugFlag = 0;

Response *make_response(Request *req) {
	RequestInner *ireq;
	Response *retval = NULL;
	char *reqhash = getRequestHash(req);
	UserKey *userkey = NULL;
	size_t sendback = 0;

	if(!req || !req->from)
		goto free_out;

	userkey = get_user_key(req->from);

	if(!userkey) 
		goto free_out;

	/* at this stage, the "from" user exists and he has keys */

	debugRequest(req);
	ireq = getRequestInner(req, userkey->key, userkey->iv);
	if(ireq) {
		debugRequestInner(ireq);
	} else {
		DEBUG("Couldn't parse Inner request!!!\n");
		goto free_out;
	}

	/* at this stage, the inner request has also been validated */

	if(ireq->has_cmd) {
        switch(ireq->cmd) {
			case COMMAND__GETKEY:
				if(!ireq->arg1) {
					// user won't be able to decrypt this
					retval = resp_ERROR(reqhash, userkey->key, userkey->iv, "Invalid parameters.");
					goto free_out;
				}

				/* only send back the keys if the public key matches */
				if(!strcmp(userkey->pubkey, ireq->arg1)) {
					retval = resp_GETKEY(reqhash, userkey->pubkey, userkey->key, userkey->iv);
					goto free_out;
				} else {
					retval = resp_ERROR(reqhash, userkey->key, userkey->iv, "That's not your public key.");
					goto free_out;
				}
			case COMMAND__SENDMSG:
				retval = resp_ERROR(reqhash, userkey->key, userkey->iv, "Due to an unprecedented amount of requests, Santa's messaging server has run out of diskspace.");
				goto free_out;
			case COMMAND__LISTMSGS:
				if(!ireq->has_arg2num) {
					retval = resp_ERROR(reqhash, userkey->key, userkey->iv, "Invalid parameters.");
					goto free_out;
				}
				/* copy the data in the buffer, located before the keys datastructure */
				copy_msgids(req->from);
				/* now allow the user to specify an insanely large amount of bytes to send back */
				sendback = ireq->arg2num * sizeof(uint32_t); 
				retval = resp_LISTMSGS(reqhash, userkey->key, userkey->iv, (uint8_t *)globMsgIdList, sendback);
				goto free_out;
			case COMMAND__GETMSG:
				if(!ireq->has_arg2num) {
					retval = resp_ERROR(reqhash, userkey->key, userkey->iv, "Invalid parameters.");
					goto free_out;
				}
				UserMsg *x = get_user_msg(req->from, ireq->arg2num);
				if(!x) {
					retval = resp_ERROR(reqhash, userkey->key, userkey->iv, "Message does not exist.");
					goto free_out;
				}

				retval = resp_GETMSG(reqhash, userkey->key, userkey->iv, x->from, x->msg);
				goto free_out;
		}
	}

free_out:
	reqinner_free(ireq);
	return retval;
}

int main(int argc, char **argv) {
	int sockfd;
	struct sockaddr_in serveraddr;
#ifndef DEBUGREMOVE
	char *ans1 = ((void *)globMsgStore < (void *)globMsgIdList ? "Yes" : "no");
	char *ans2 = ((void *)globMsgIdList < (void *)globKeyStore ? "Yes" : "no");
#endif

	if(argc > 1) globDebugFlag = 1;

	read_userdata(USERDATADIR "/userlist.txt", USERDATADIR);
	dump_userdata();

#ifndef DEBUGREMOVE
	DEBUG("globMsgStore at %p\n", globMsgStore);
	DEBUG("globMsgIdList at %p\n", globMsgIdList);
	DEBUG("globKeyStore at %p\n", globKeyStore);
	DEBUG("globMsgStore < globMsgIdList: %s\n", ans1);
	DEBUG("globMsgIdList < globKeyStore: %s\n", ans2);
#endif

	sockfd = listen_server(SERVERPORT, &serveraddr);

	while(1) {
		Request *req;
		Response *resp;
		struct sockaddr_in client_addr;

		memset(&client_addr, 0, sizeof(client_addr));

		DEBUG("Waiting for packet...\n");
		req = recv_request(sockfd, &client_addr);
		if(req) {
		    DEBUG("Got request from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

			resp = make_response(req);
			if(resp) {
				DEBUG("Sending reply...\n");
				send_response(sockfd, client_addr, resp);
				resp_free(resp);
				DEBUG("Done.\n");
			}
		} else {
			DEBUG("Faulty packet\n");
		}

		req_free(req);
	}
}


