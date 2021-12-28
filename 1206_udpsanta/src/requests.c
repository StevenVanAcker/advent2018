#include "requests.h"
#include "config.h"

// myalloc called, must free (only called from req_base)
RequestInner *reqinner_base(Command cmd, char *arg1, char *arg2, size_t arg2num, size_t has_arg2num) {
	RequestInner *r = (RequestInner *)myalloc(sizeof(RequestInner));
	request_inner__init(r);

	r->has_cmd = 1;
	r->has_timestamp = 1;
	r->has_padding = 1;

	r->cmd = cmd;
	r->timestamp = time(NULL);

	if(arg1) r->arg1 = strdup(arg1);
	if(arg2) r->arg2 = strdup(arg2);
	if(has_arg2num) {
		r->has_arg2num = 1;
		r->arg2num = arg2num;
	}

	return r;
}

void reqinner_free(RequestInner *r) {
	if(!r) return;

	myfree(r->arg1);
	myfree(r->arg2);
	if(r->has_padding) myfree(r->padding.data);
	myfree(r);
}

void dumpInnerRequestToFile(RequestInner *r, char *filename) {
	FILE *outfile;

    size_t bufsize = request_inner__get_packed_size(r); 
    uint8_t *buf = (uint8_t *) myalloc(bufsize);
    request_inner__pack(r, buf);

	outfile = fopen(filename, "wb");
	fwrite(buf, bufsize, 1, outfile);
	fclose(outfile);
	myfree(buf);
}

/* parse RequestInner from string and validate it.
 * NULL on failure
 */
RequestInner *parseRequestInner(uint8_t *data, size_t size) {
	return request_inner__unpack(NULL, size, data);
}

/* parse Request from string and validate it.
 * NULL on failure
 */
Request *parseRequest(uint8_t *data, size_t size) {
	return request__unpack(NULL, size, data);
}

RequestInner *readInnerRequestFromFile(char *filename) {
	FILE *infile;
	uint8_t buf[10000]; 
	size_t size = 0;

	infile = fopen(filename, "rb");
	size = fread(buf, 1, sizeof(buf), infile);
	fclose(infile);

	return parseRequestInner(buf, size);
}

void debugRequestInner(RequestInner *r) {
	if(!r) {
		DEBUG("--> RequestInner is NULL\n");
		return;
	} else {
		DEBUG("--> RequestInner at %p\n", r);
	}

	if(r->has_cmd) {
		char *cmdname = "<unknown>";
		switch(r->cmd) {
			case COMMAND__GETKEY:
				cmdname = "GETKEY";
				break;
			case COMMAND__SENDMSG:
				cmdname = "SENDMSG";
				break;
			case COMMAND__LISTMSGS:
				cmdname = "LISTMSGS";
				break;
			case COMMAND__GETMSG:
				cmdname = "GETMSG";
				break;
		}

		DEBUG("        cmd = %d (%s)\n", r->cmd, cmdname);
	} else {
		DEBUG("        cmd not present\n");
	}

	if(r->arg1) DEBUG("        arg1 = %s\n", r->arg1);
	else DEBUG("        arg1 not present\n");

	if(r->arg2) DEBUG("        arg2 = %s\n", r->arg2);
	else DEBUG("        arg2 not present\n");

	if(r->has_arg2num) DEBUG("        arg2num = %u\n", r->arg2num);
	else DEBUG("        arg2num not present\n");

	if(r->has_timestamp) DEBUG("        timestamp = %u\n", r->timestamp);
	else DEBUG("        timestamp not present\n");

	if(r->has_padding) {
		DEBUG("        padding present (%u bytes)\n", r->padding.len);
	} else DEBUG("        padding not present\n");

}

// myalloc called, must req_free (called from client.c, freed in network.c)
Request *req_base(char *from, char *symmkey, char *iv, RequestInner *inner, int freeInner) {
	Request *r = (Request *)myalloc(sizeof(Request));
	request__init(r);

	r->from = strdup(from);
	r->has_encrypted = 1;
	r->encrypted = symmkey == NULL ? 0 : 1;
	r->has_innerrequest = 1;

    size_t innersize = 0;
    uint8_t *innerdata = NULL;

    size_t outsize = 0;
    uint8_t *outdata = NULL;

	/*
	 * in this loop, we fill in padding in inner, and calculate the sha256 of outer
	 * every time, we free the buffers we allocate, except on exit.
	 * innerdata should not be free()'d because r points to it
	 */
	do {
		myfree(outdata);

		// use padding to make sha256 work
		myfree(inner->padding.data);
		inner->has_padding = 1;
		inner->padding.data = myalloc(PADDINGSIZE);
		random_string(inner->padding.data, PADDINGSIZE);
		inner->padding.len = PADDINGSIZE;

		innersize = request_inner__get_packed_size(inner);
		innerdata = (uint8_t *) myalloc(innersize);
		request_inner__pack(inner, innerdata);

		if(symmkey) {
			// encrypt if needed. We replace innerdata with the encrypted version and free the former
			size_t innersize_enc = 0;
			uint8_t *innerdata_enc = NULL;
			encrypt_sym(symmkey, iv, innerdata, innersize, &innerdata_enc, &innersize_enc);
			myfree(innerdata);

			innerdata = innerdata_enc;
			innersize = innersize_enc;
		}

		myfree(r->innerrequest.data);
		r->innerrequest.data = innerdata;
		r->innerrequest.len = innersize;
		innerdata = NULL; /* don't need this anymore, 
							 set it to NULL so we don't accidently free it later */

		outsize = request__get_packed_size(r);
		outdata = (uint8_t *) myalloc(outsize);
		request__pack(r, outdata);

	} while(!OCD_satisfied(outdata, outsize));

	myfree(outdata);

	if(freeInner) reqinner_free(inner);

	return r;
}

void req_free(Request *r) {
	if(!r) return;

	myfree(r->from);
	if(r->has_innerrequest) myfree(r->innerrequest.data);
	myfree(r);
}

void debugRequest(Request *r) {
	if(!r) {
		DEBUG("--> Request is NULL\n");
		return;
	} else {
		DEBUG("--> Request at %p\n", r);
	}

	if(r->from) DEBUG("        from = %s\n", r->from);
	else DEBUG("    from not present\n");

	if(r->has_encrypted) DEBUG("        encrypted = %d\n", r->encrypted);
	else DEBUG("        encrypted not present\n");

	if(r->has_innerrequest) {
		DEBUG("        innerrequest present (%u bytes)\n", r->innerrequest.len);
	} else DEBUG("        innerrequest not present\n");

}

RequestInner *getRequestInner(Request *req, char *key, char *iv) {
	unsigned char *outdata;
   	size_t outlen;
	RequestInner *ireq;

	if(!req) return NULL;

	if(req->has_encrypted && req->encrypted) {
		decrypt_sym(key, iv, req->innerrequest.data, req->innerrequest.len, &outdata, &outlen);
		ireq = parseRequestInner(outdata, outlen);
		myfree(outdata);
	} else {
		ireq = parseRequestInner(req->innerrequest.data, req->innerrequest.len);
	}

	return ireq;
}

char *getRequestHash(Request *r) {
	static char hash[65];
    size_t outsize = 0;
    uint8_t *outdata = NULL;

	outsize = request__get_packed_size(r);
	outdata = (uint8_t *) myalloc(outsize);
	request__pack(r, outdata);

	sha256sum(outdata, outsize, hash);

	myfree(outdata);
	return hash;
}
