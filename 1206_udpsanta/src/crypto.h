#ifndef __CRYPTO_H__
#define __CRYPTO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

#include <arpa/inet.h> /* For htonl() */

#include "tools.h"

#define CRYPTO_BUFSIZE 1024

extern int encrypt_pk(unsigned char *, unsigned char *, size_t, unsigned char **, size_t *);
extern int decrypt_pk(unsigned char *, unsigned char *, size_t, unsigned char **, size_t *);

extern int encrypt_sym(unsigned char *, unsigned char *, unsigned char *, size_t, unsigned char **, size_t *);
extern int decrypt_sym(unsigned char *, unsigned char *, unsigned char *, size_t, unsigned char **, size_t *);

#endif /* __CRYPTO_H__ */
