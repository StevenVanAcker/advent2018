#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "requests.h"
#include "responses.h"
#include "tools.h"


extern int connect_server(char *, uint16_t, struct sockaddr_in *);
extern int listen_server(uint16_t, struct sockaddr_in *);
extern void disconnect_socket(int);
extern int send_request(int, struct sockaddr_in, Request *);
extern Request *recv_request(int, struct sockaddr_in *);

extern Response *get_response(int);
extern int send_response(int, struct sockaddr_in, Response *);


extern Response *client_communicate(int, struct sockaddr_in, Request *);


#endif /* __NETWORK_H__ */
