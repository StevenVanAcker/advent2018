#include "responses.h"
#include "config.h"

// myalloc called, must free (only called from resp_base)
ResponseInner *respinner_base(Command cmd, char *result1, char *result2, int boolresult, uint8_t *resultbytes, size_t resultlen) {
	ResponseInner *r = (ResponseInner *)myalloc(sizeof(ResponseInner));
	response_inner__init(r);

	r->has_cmd = 1;
	r->cmd = cmd;

	r->has_padding = 1;
	r->has_timestamp = 1;
	r->timestamp = time(NULL);

	if(result1) r->result1 = strdup(result1);
	if(result2) r->result2 = strdup(result2);
	if(boolresult >= 0) {
		r->has_resultbool = 1;
		r->resultbool = boolresult;
	}
	if(resultbytes) {
		uint8_t *copy = myalloc(resultlen);
		memcpy(copy, resultbytes, resultlen);
		r->has_resultbytes = 1;
		r->resultbytes.data = copy;
		r->resultbytes.len = resultlen;
	}

	return r;
}

void respinner_free(ResponseInner *r) {
	if(!r) return;
	if(r->result1) myfree(r->result1);
	if(r->result2) myfree(r->result2);
	if(r->has_resultbytes) myfree(r->resultbytes.data);
	if(r->has_padding) myfree(r->padding.data);

	myfree(r);
}

void dumpInnerResponseToFile(ResponseInner *r, char *filename) {
    FILE *outfile;

    size_t bufsize = response_inner__get_packed_size(r);
    uint8_t *buf = (uint8_t *) myalloc(bufsize);
    response_inner__pack(r, buf);

    outfile = fopen(filename, "wb");
    fwrite(buf, bufsize, 1, outfile);
    fclose(outfile);
    myfree(buf);
}

ResponseInner *readInnerResponseFromFile(char *filename) {
    FILE *infile;
    uint8_t buf[10000]; 
    size_t size = 0;

    infile = fopen(filename, "rb");
    size = fread(buf, 1, sizeof(buf), infile);
    fclose(infile);

    return parseResponseInner(buf, size);
}

/* parse ResponseInner from string and validate it.
 * NULL on failure
 */
ResponseInner *parseResponseInner(uint8_t *data, size_t size) {
    return response_inner__unpack(NULL, size, data);
}

/* parse Response from string and validate it.
 * NULL on failure
 */
Response *parseResponse(uint8_t *data, size_t size) {
    return response__unpack(NULL, size, data);
}

void debugResponseInner(ResponseInner *r) {
    if(!r) {
        DEBUG("<-- ResponseInner is NULL\n");
        return;
    } else {
        DEBUG("<-- ResponseInner at %p\n", r);
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

    if(r->result1) DEBUG("        result1 = %s\n", r->result1);
    else DEBUG("        result1 not present\n");

    if(r->result2) DEBUG("        result2 = %s\n", r->result2);
    else DEBUG("        result2 not present\n");

    if(r->has_resultbool) DEBUG("        resultbool = %u\n", r->resultbool);
    else DEBUG("        resultbool not present\n");

    if(r->has_timestamp) DEBUG("        timestamp = %u\n", r->timestamp);
    else DEBUG("        timestamp not present\n");

    if(r->has_resultbytes) {
        DEBUG("        resultbytes present (%u bytes)\n", r->resultbytes.len);
    } else DEBUG("        resultbytes not present\n");

    if(r->has_padding) {
        DEBUG("        padding present (%u bytes)\n", r->padding.len);
    } else DEBUG("        padding not present\n");

}

void debugResponse(Response *r) {
    if(!r) {
        DEBUG("<-- Response is NULL\n");
        return;
    } else {
        DEBUG("<-- Response at %p\n", r);
	}

    if(r->replyto) DEBUG("        replyto = %s\n", r->replyto);
    else DEBUG("        replyto not present\n");

    if(r->has_pki_encrypted) DEBUG("        pki_encrypted = %d\n", r->pki_encrypted);
    else DEBUG("        pki_encrypted not present\n");

    if(r->has_innerresponse) {
        DEBUG("        innerresponse present (%u bytes)\n", r->innerresponse.len);
    } else DEBUG("        innerresponse not present\n");

}


// myalloc called, must resp_free (called and freed in server.c)
Response *resp_base(char *replyto, char *pubkey, char *symmkey, char *iv, ResponseInner *inner, int freeInner) {
	Response *r = (Response *)myalloc(sizeof(Response));
	response__init(r);

	r->replyto = strdup(replyto);
	r->has_pki_encrypted = 1;
	r->pki_encrypted = pubkey == NULL ? 0 : 1;
	r->has_innerresponse = 1;

    size_t innersize = 0;
    uint8_t *innerdata = NULL;

    size_t outsize = 0;
    uint8_t *outdata = NULL;

	{
		myfree(outdata);
		myfree(innerdata);

		// use padding to make sha256 work
		uint8_t *padding = myalloc(PADDINGSIZE);
		random_string(padding, PADDINGSIZE);

		inner->has_padding = 1;
		inner->padding.data = padding;
		inner->padding.len = PADDINGSIZE;

		innersize = response_inner__get_packed_size(inner);
		innerdata = (uint8_t *) myalloc(innersize);
		response_inner__pack(inner, innerdata);

		if(pubkey) {
			size_t innersize_enc = 0;
			uint8_t *innerdata_enc = NULL;
			encrypt_pk(pubkey, innerdata, innersize, &innerdata_enc, &innersize_enc);
			myfree(innerdata);

			innerdata = innerdata_enc;
			innersize = innersize_enc;
		} else {
			// encrypt with symmetric key
			size_t innersize_enc = 0;
			uint8_t *innerdata_enc = NULL;
			encrypt_sym(symmkey, iv, innerdata, innersize, &innerdata_enc, &innersize_enc);
			myfree(innerdata);

			innerdata = innerdata_enc;
			innersize = innersize_enc;
		}

		r->innerresponse.data = innerdata;
		r->innerresponse.len = innersize;

		outsize = response__get_packed_size(r);
		outdata = (uint8_t *) myalloc(outsize);
		response__pack(r, outdata);

	} 

	myfree(outdata);

	if(freeInner) respinner_free(inner);

	return r;
}

void resp_free(Response *r) {
	if(!r) return;
	if(r->replyto) myfree(r->replyto);
	if(r->has_innerresponse) myfree(r->innerresponse.data);

	myfree(r);
}

ResponseInner *getResponseInner(Response *resp, char *privkey, char *key, char *iv) {
    unsigned char *outdata;
    size_t outlen;
    ResponseInner *iresp;

    if(!resp) return NULL;
    if(!privkey) { DEBUG("Private key is NULL\n"); return NULL; }

    if(resp->has_pki_encrypted && resp->pki_encrypted) {
        decrypt_pk(privkey, resp->innerresponse.data, resp->innerresponse.len, &outdata, &outlen);
        iresp = parseResponseInner(outdata, outlen);
        myfree(outdata);
    } else {
		if(!key) { DEBUG("Symmetric key is NULL\n"); return NULL; }
		if(!iv) { DEBUG("Symmetric key IV is NULL\n"); return NULL; }
        decrypt_sym(key, iv, resp->innerresponse.data, resp->innerresponse.len, &outdata, &outlen);
        iresp = parseResponseInner(outdata, outlen);
        myfree(outdata);
    }

    return iresp;
}

