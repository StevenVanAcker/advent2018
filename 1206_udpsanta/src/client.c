#include <stdio.h>
#include "config.h"
#include "tools.h"
#include "requests.h"
#include "network.h"

uint8_t *globPrivKey, *globPubKey, *globUsername;
unsigned char *globSymKey, *globSymIV;

uint32_t *globMsgIds = NULL;
size_t globMsgCount = 0;

struct sockaddr_in globServerAddress;
int globSock = -1;
int globDebugFlag = 0;

char *getInput(char *prompt) {
	static char buf[1000], *p;
	printf("%s", prompt);
	fflush(0);
	fgets(buf, sizeof(buf), stdin);
	p = strchr(buf, '\n');
	if(p) *p = '\0';
	return buf;
}

void handleResponse(Response *resp) {
	ResponseInner *iresp = NULL;
	debugResponse(resp);

	if(!resp) {
		DEBUG("Response invalid\n");
		goto free_resp;
	} else {
	}

	iresp = getResponseInner(resp, globPrivKey, globSymKey, globSymIV);
	debugResponseInner(iresp);
	if(!iresp) {
		DEBUG("Inner response invalid\n");
		goto free_resp;
	} else {
		switch(iresp->cmd) {
			case COMMAND__GETKEY:
				DEBUG("Received GETKEY result\n");
				if(iresp->result1 && iresp->result2) {
					myfree(globSymKey);
					myfree(globSymIV);
					globSymKey = strdup(iresp->result1);
					globSymIV = strdup(iresp->result2);
				} else {
					DEBUG("Didn't receive result1 and result2 as expected\n");
				}
				break;
			case COMMAND__SENDMSG:
				DEBUG("Received SENDMSG result\n");
				if(iresp->has_resultbool && iresp->resultbool) {
					DEBUG("Message sent successfully.\n");
				} else {
					DEBUG("Sending message failed.\n");
				}
				break;
			case COMMAND__LISTMSGS:
				DEBUG("Received LISTMSGS result\n");
				myfree(globMsgIds);
				globMsgCount = 0;

				if(iresp->has_resultbytes) {
					//hexdump(iresp->resultbytes.data, iresp->resultbytes.len); // EXPLOIT
					globMsgIds = (uint32_t *)myalloc(iresp->resultbytes.len);
					memcpy(globMsgIds, iresp->resultbytes.data, iresp->resultbytes.len);
					globMsgCount = 0;
					for(size_t i = 0; i < 20; i++) 
						if(globMsgIds[i] != -1) 
							globMsgCount++;
				} else {
					DEBUG("Didn't receive resultbytes as expected\n");
				}
				break;
			case COMMAND__GETMSG:
				DEBUG("Received GETMSG result\n");
				if(iresp->result1 && iresp->result2) {
					printf("\nMessage from: %s\n", iresp->result1);
					printf("Message body:\n%s\n\n\n", iresp->result2);
				} else {
					DEBUG("Didn't receive result1 and result2 as expected\n");
				}
				break;
			case COMMAND__ERROR:
				DEBUG("Received ERROR result\n");
				if(iresp->result1) {
					printf("Command failed with error: \"%s\"\n", iresp->result1);
				} else {
					DEBUG("Didn't receive result1 as expected\n");
				}
				break;
			default:
				DEBUG("Unknown command result\n");
				break;
		}
	}
free_resp:
	resp_free(resp);
	respinner_free(iresp);
}

int menuloop() {
	char *selection;

	system("clear");

	/* first, make sure we are authenticated */
	if(!(globSymKey && globSymIV)) {
		printf("Connecting.... ");
		fflush(0);
		handleResponse(client_communicate(globSock, globServerAddress, 
					req_GETKEY(globUsername, globPubKey)));
		if(globSymKey && globSymIV) {
			printf("Success!\n");
		} else {
			printf("Failed!\n");
			_exit(1);
		}
	}

	/* next, fetch our messages */
	handleResponse(client_communicate(globSock, globServerAddress, 
				req_LISTMSGS(globUsername, globSymKey, globSymIV, 20))); // EXPLOIT change to 300


	/* and print them out */
	if(globMsgCount > 0) {
		printf("You have %lu messages with IDs: ", globMsgCount);
		for(size_t i = 0; i < globMsgCount; i++) {
			if(globMsgIds[i] == -1) continue;

			if(i != 0) {
				printf(", ");
			}
			printf("%d", globMsgIds[i]);
		}
		printf(".\n");
		printf("\n");
		printf("Enter message ID to read, enter 'send' to send a message, or 'quit' to quit.\n");
		printf("\n");

	} else {
		printf("You have no messages.\n");
		printf("\n");
		printf("Enter 'send' to send a message, or 'quit' to quit.\n");
		printf("\n");
	}

	selection = getInput("Choice> ");

	if(!strcmp(selection, "send")) {
		char *to = strdup(getInput("Recipient> "));
		char *msg = strdup(getInput("Message> "));
		printf("\nSending message to %s...\n", to);
		handleResponse(client_communicate(globSock, globServerAddress, 
					req_SENDMSG(globUsername, globSymKey, globSymIV, to, msg)));
		myfree(to);
		myfree(msg);
	} else if(!strcmp(selection, "quit")) {
		return 0;
	} else {
		/* it must be an ID */
		int mid = atoi(selection);
		printf("Getting message with ID %d...\n", mid);
		handleResponse(client_communicate(globSock, globServerAddress, 
					req_GETMSG(globUsername, globSymKey, globSymIV, mid)));
	}

	return 1;
}

int main(int argc, char **argv) {
	char *serverip;
	uint16_t serverport = SERVERPORT;

	globPrivKey = read_file_as_string(dotfile(DOTDIR "/private.key"));
	globPubKey = read_file_as_string(dotfile(DOTDIR "/public.key"));
	globUsername = read_file_as_string(dotfile(DOTDIR "/user.name"));

	Response *resp = NULL;

	if(!globUsername) {
		ERROR("Couldn't read user.name file\n");
	}

	if(!globPrivKey || !globPubKey) {
		ERROR("Couldn't read private or public key files\n");
	}

	if(argc > 1) {
		serverip = argv[1];
	} else {
		ERROR("Specify the server IP address: %s <server ip>\n", argv[0]);
	}

	if(argc > 2) {
		globDebugFlag = 1;
	}
	
	if((globSock = connect_server(serverip, serverport, &globServerAddress)) <= 0) {
		ERROR("Couldn't connect to server\n");
	}

	while(menuloop()) getInput("Press enter to continue... ");

	disconnect_socket(globSock);
}

