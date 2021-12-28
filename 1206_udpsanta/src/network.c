#include "config.h"
#include "network.h"


int listen_server(uint16_t port, struct sockaddr_in *serveraddr) {
	int sockfd = -1;
	struct sockaddr_in *s = (struct sockaddr_in *)serveraddr;
	memset(serveraddr, 0, sizeof(struct sockaddr_in));

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		DEBUG("Couldn't get socket\n");
		return -1;
	}

	s->sin_family = AF_INET;
	s->sin_port = htons(port);
	//s->sin_addr = 0; // implied because of memset

	if (bind(sockfd, (struct sockaddr *)s, sizeof(struct sockaddr_in)) == -1) {
		close(sockfd);
		DEBUG("Couldn't bind socket\n");
		return -1;
	}

	return sockfd;
}

int connect_server(char *ip, uint16_t port, struct sockaddr_in *serveraddr) {
	int sockfd = -1;
	struct sockaddr_in *s = (struct sockaddr_in *)serveraddr;
	memset(serveraddr, 0, sizeof(struct sockaddr_in));

	/* we speak IPv4 here ... */
	s->sin_family = AF_INET;
	s->sin_port = htons(port);
	inet_aton(ip, &s->sin_addr);

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		DEBUG("Couldn't get socket\n");
		return -1;
	}

	return sockfd;
}

void disconnect_socket(int sockfd) {
	close(sockfd);
}


int send_request(int sockfd, struct sockaddr_in destination, Request *req) {
	char hashbuf[65];
	int retval = 0;
	SERIALIZE_TO_BUFFER(data, datalen, req, request);
	retval = sendto(sockfd, data, datalen, 0, (struct sockaddr *)&destination, sizeof(struct sockaddr_in));
	myfree(data);
	return retval;
}

Request *recv_request(int sockfd, struct sockaddr_in *source) {
	uint8_t recvbuf[NETBUFMAXLEN];
	size_t recvbuflen;
	int sourcelen = sizeof(struct sockaddr_in);

	char hashbuf[65];

	/* read the packet */
	if(-1 == (recvbuflen = recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr *)source, &sourcelen))) {
		DEBUG("Error reading request\n");
		return NULL;
	}

	if(sourcelen != sizeof(struct sockaddr_in)) {
		DEBUG("Unexpected sockaddr type\n");
		return NULL;
	}

	/* check checksum */
	if(!OCD_satisfied(recvbuf, recvbuflen)) {
		DEBUG("Request does not match OCD check\n");
		return NULL;
	}

	/* parse and return request  */
	return parseRequest(recvbuf, recvbuflen);
}

int send_response(int sockfd, struct sockaddr_in destination, Response *resp) {
	char hashbuf[65];
	struct sockaddr_in *d = (struct sockaddr_in *)&destination;
	int retval = 0;

	SERIALIZE_TO_BUFFER(data, datalen, resp, response);
	DEBUG("Sending response to %s:%d\n", inet_ntoa(d->sin_addr), ntohs(d->sin_port));

	retval = sendto(sockfd, data, datalen, 0, (struct sockaddr *)d, sizeof(struct sockaddr_in));
	myfree(data);
	return retval;
}

Response *get_response(int sockfd) {
	uint8_t recvbuf[NETBUFMAXLEN];
	size_t recvbuflen;
	char hashbuf[65];

	/* read the packet */
	if(-1 == (recvbuflen = recv(sockfd, recvbuf, sizeof(recvbuf), 0))) {
		DEBUG("Error reading response\n");
		return NULL;
	}

	/* parse and return response */
	return parseResponse(recvbuf, recvbuflen);
}

Response *client_communicate(int sock, struct sockaddr_in destination, Request *req) {
	if(-1 == send_request(sock, destination, req)) {
		DEBUG("Error sending request\n");
		return NULL;
	}
	/* I know the guy who wrote this, and he's OK with freeing it here... */
	req_free(req);
	return get_response(sock);
}

