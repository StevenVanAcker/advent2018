#include "tools.h"

void __m5(BYTE *data, size_t datalen, char *outputhash) {
    BYTE rawhash[MD5_BLOCK_SIZE];
    MD5_CTX md5;
    md5_init(&md5);
    md5_update(&md5, data, datalen);
    md5_final(&md5, rawhash);
    for(int i = 0; i < MD5_BLOCK_SIZE; i++) {
        sprintf(outputhash + (i * 2), "%02x", rawhash[i]);
    }
    outputhash[MD5_BLOCK_SIZE * 2] = 0;
}

void __s1(BYTE *data, size_t datalen, char *outputhash) {
    BYTE rawhash[SHA1_BLOCK_SIZE];
    SHA1_CTX sha1;
    sha1_init(&sha1);
    sha1_update(&sha1, data, datalen);
    sha1_final(&sha1, rawhash);
    for(int i = 0; i < SHA1_BLOCK_SIZE; i++) {
        sprintf(outputhash + (i * 2), "%02x", rawhash[i]);
    }
    outputhash[SHA1_BLOCK_SIZE * 2] = 0;
}

void __s2(BYTE *data, size_t datalen, char *outputhash) {
    BYTE rawhash[SHA256_BLOCK_SIZE];
    SHA256_CTX sha256;
    sha256_init(&sha256);
    sha256_update(&sha256, data, datalen);
    sha256_final(&sha256, rawhash);
    for(int i = 0; i < SHA256_BLOCK_SIZE; i++) {
        sprintf(outputhash + (i * 2), "%02x", rawhash[i]);
    }
    outputhash[SHA256_BLOCK_SIZE * 2] = 0;
}

