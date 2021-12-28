#include "crypto.h"

// outdata is allocated, must free
int encrypt_pk(unsigned char *rsa_pkey_str, unsigned char *indata, size_t inlen, unsigned char **outdata, size_t *outlen)
{
    int retval = 0;
    RSA *rsa_pkey = NULL;
	BIO *biokey = BIO_new(BIO_s_mem());
    EVP_PKEY *pkey = EVP_PKEY_new();
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char buffer_out[CRYPTO_BUFSIZE + EVP_MAX_IV_LENGTH];
    int len_out;
    unsigned char *ek = NULL;
    int eklen;
    uint32_t eklen_n;
    unsigned char iv[EVP_MAX_IV_LENGTH];

	*outdata = NULL;
	*outlen = 0;

	if (!BIO_write(biokey, rsa_pkey_str, strlen(rsa_pkey_str))) {
        DEBUG("Error making RSA Public Key BIO.\n");
        ERR_print_errors_fp(stderr);
        retval = 2;
        goto out;
	}

    if (!PEM_read_bio_RSA_PUBKEY(biokey, &rsa_pkey, NULL, NULL))
    {
        DEBUG("Error loading RSA Public Key File.\n");
        ERR_print_errors_fp(stderr);
        retval = 2;
        goto out;
    }

    if (!EVP_PKEY_assign_RSA(pkey, rsa_pkey))
    {
        DEBUG("EVP_PKEY_assign_RSA: failed.\n");
        retval = 3;
        goto out;
    }

    EVP_CIPHER_CTX_init(ctx);
    ek = myalloc(EVP_PKEY_size(pkey));

    if (!EVP_SealInit(ctx, EVP_aes_128_cbc(), &ek, &eklen, iv, &pkey, 1))
    {
        DEBUG("EVP_SealInit: failed.\n");
        retval = 3;
        goto out_free;
    }

    /* First we write out the encrypted key length, then the encrypted key,
     * then the iv (the IV length is fixed by the cipher we have chosen).
     */

    eklen_n = htonl(eklen);
    *outlen = (sizeof eklen_n) + eklen + EVP_CIPHER_iv_length(EVP_aes_128_cbc());
    if(!(*outdata = myalloc(*outlen))) {
		ERROR("Out of memory");
    }

    memcpy(*outdata, &eklen_n, sizeof eklen_n);
    memcpy(*outdata + (sizeof eklen_n), ek, eklen);
    memcpy(*outdata + (sizeof eklen_n) + eklen, iv, EVP_CIPHER_iv_length(EVP_aes_128_cbc()));

    /* Now we process the input file and write the encrypted data to the
     * output file. */

    if (!EVP_SealUpdate(ctx, buffer_out, &len_out, indata, inlen))
    {
	DEBUG("EVP_SealUpdate: failed.\n");
	retval = 3;
	myfree(*outdata);
	*outlen = 0;
	goto out_free;
    }

    if(!(*outdata = realloc(*outdata, *outlen + len_out))) {
		ERROR("Out of memory");
    }
    memcpy(*outdata + *outlen, buffer_out, len_out);
    *outlen += len_out;

    if (!EVP_SealFinal(ctx, buffer_out, &len_out))
    {
        DEBUG("EVP_SealFinal: failed.\n");
        retval = 3;
		myfree(*outdata);
		*outlen = 0;
        goto out_free;
    }

    if(!(*outdata = realloc(*outdata, *outlen + len_out))) {
		ERROR("Out of memory");
    }
    memcpy(*outdata + *outlen, buffer_out, len_out);
    *outlen += len_out;


    out_free:
	EVP_CIPHER_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    BIO_free(biokey);
    myfree(ek);

    out:
    return retval;
}

// outdata is allocated, must free
int decrypt_pk(unsigned char *rsa_privkey_str, unsigned char *indata, size_t inlen, unsigned char **outdata, size_t *outlen)
{
    int retval = 0;
    RSA *rsa_pkey = NULL;
	BIO *biokey = BIO_new(BIO_s_mem());
    EVP_PKEY *pkey = EVP_PKEY_new();
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    unsigned char buffer_out[CRYPTO_BUFSIZE + EVP_MAX_IV_LENGTH];
    size_t len;
    int len_out;
    unsigned char *ek;
    size_t eklen;
    uint32_t eklen_n;
    unsigned char iv[EVP_MAX_IV_LENGTH];

	*outdata = NULL;
	*outlen = 0;

	if (!BIO_write(biokey, rsa_privkey_str, strlen(rsa_privkey_str))) {
        DEBUG("Error making RSA Private Key BIO.\n");
        ERR_print_errors_fp(stderr);
        retval = 2;
        goto out;
	}

    if (!PEM_read_bio_RSAPrivateKey(biokey, &rsa_pkey, NULL, NULL))
    {
        DEBUG("Error loading RSA Private Key File.\n");
        ERR_print_errors_fp(stderr);
        retval = 2;
        goto out;
    }

    if (!EVP_PKEY_assign_RSA(pkey, rsa_pkey))
    {
        DEBUG("EVP_PKEY_assign_RSA: failed.\n");
        retval = 3;
        goto out;
    }

    EVP_CIPHER_CTX_init(ctx);
    ek = myalloc(EVP_PKEY_size(pkey));

    /* First need to fetch the encrypted key length, encrypted key and IV */

    if(inlen >= sizeof eklen_n) {
       memcpy(&eklen_n, indata, sizeof eklen_n);
    } else {
        perror("input file");
        retval = 4;
        goto out_free;
    }
    eklen = ntohl(eklen_n);

    if (eklen > EVP_PKEY_size(pkey))
    {
        DEBUG("Bad encrypted key length (%u > %d)\n", (unsigned int)eklen,
            EVP_PKEY_size(pkey));
        retval = 4;
        goto out_free;
    }
    if(inlen >= (sizeof eklen_n) + eklen) {
       memcpy(ek, indata + (sizeof eklen_n), eklen);
    } else {
        perror("input file");
        retval = 4;
        goto out_free;
    }
    if(inlen >= (sizeof eklen_n) + eklen + EVP_CIPHER_iv_length(EVP_aes_128_cbc())) {
       memcpy(iv, indata + (sizeof eklen_n) + eklen, EVP_CIPHER_iv_length(EVP_aes_128_cbc()));
    } else {
        perror("input file");
        retval = 4;
        goto out_free;
    }

    if (!EVP_OpenInit(ctx, EVP_aes_128_cbc(), ek, eklen, iv, pkey))
    {
        DEBUG("EVP_OpenInit: failed.\n");
        retval = 3;
        goto out_free;
    }

    len = inlen - ((sizeof eklen_n) + eklen + EVP_CIPHER_iv_length(EVP_aes_128_cbc()));

    if (!EVP_OpenUpdate(ctx, buffer_out, &len_out, indata + (sizeof eklen_n) + eklen + EVP_CIPHER_iv_length(EVP_aes_128_cbc()), len))
    {
	DEBUG("EVP_OpenUpdate: failed.\n");
	retval = 3;
	goto out_free;
    }

    *outlen = len_out;
    if(!(*outdata = myalloc(*outlen))) {
		ERROR("Out of memory");
    }

    memcpy(*outdata, buffer_out, len_out);

    if (!EVP_OpenFinal(ctx, buffer_out, &len_out))
    {
        DEBUG("EVP_SealFinal: failed.\n");
        retval = 3;
		myfree(*outdata);
		*outlen = 0;
        goto out_free;
    }

    if(!(*outdata = realloc(*outdata, *outlen + len_out))) {
		ERROR("Out of memory");
    }

    memcpy(*outdata + *outlen, buffer_out, len_out);
    *outlen += len_out;

    out_free:
    EVP_CIPHER_CTX_free(ctx);
    EVP_PKEY_free(pkey);
    BIO_free(biokey);
    myfree(ek);

    out:
    return retval;
}

void handleErrors(void)
{
  ERR_print_errors_fp(stderr);
  abort();
}

// outdata is allocated, must free
int encrypt_sym(unsigned char *key, unsigned char *iv, unsigned char *indata, size_t inlen, unsigned char **outdata, size_t *outlen) {
  EVP_CIPHER_CTX *ctx;
  int len;
  int calclen = inlen + (16 - (inlen % 16));

  *outlen = 0;
  *outdata = myalloc(calclen);
  if(!*outdata) ERROR("Out of memory");

  if(!(ctx = EVP_CIPHER_CTX_new()))
	  goto fail;

  if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
	  goto fail;

  if(1 != EVP_EncryptUpdate(ctx, *outdata, &len, indata, inlen))
	  goto fail;

  *outlen = len;

  if(1 != EVP_EncryptFinal_ex(ctx, *outdata + len, &len)) 
	  goto fail;

  *outlen += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return *outlen;

fail:
  myfree(*outdata);
  *outlen = 0;
  ERR_print_errors_fp(stderr);
  return 0;
}

// outdata is allocated, must free
int decrypt_sym(unsigned char *key, unsigned char *iv, unsigned char *indata, size_t inlen, unsigned char **outdata, size_t *outlen) {
  EVP_CIPHER_CTX *ctx;
  int len;
  int calclen = inlen;

  *outlen = 0;
  *outdata = myalloc(calclen);
  if(!*outdata) ERROR("Out of memory");

  if(!(ctx = EVP_CIPHER_CTX_new())) 
	  goto fail;

  if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv))
	  goto fail;

  if(1 != EVP_DecryptUpdate(ctx, *outdata, &len, indata, inlen))
	  goto fail;

  *outlen = len;

  if(1 != EVP_DecryptFinal_ex(ctx, *outdata + len, &len))
	  goto fail;

  *outlen += len;

  /* Clean up */
  EVP_CIPHER_CTX_free(ctx);

  return *outlen;

fail:
  myfree(*outdata);
  *outlen = 0;
  ERR_print_errors_fp(stderr);
  return 0;
}
