#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdint.h>
#include <openssl/sha.h>

extern int globDebugFlag;

extern void LOGFN(const char *, const char *, ...);
#define LOG(...) LOGFN("LOG", __VA_ARGS__)
#define ERROR(...) { LOGFN("ERROR", __VA_ARGS__); _exit(1); }
#define DEBUG(...) LOGFN("DEBUG", __VA_ARGS__)

extern void nop();
extern void *myalloc(size_t);
extern void _myfree(void *);
#define myfree(x) _myfree(x); x = NULL;

extern uint8_t *buf2str(uint8_t *, size_t);
extern void read_file(char *, uint8_t **, size_t *);
extern uint8_t *read_file_as_string(char *);
extern void write_file(char *, uint8_t *, size_t);
extern char *dotfile(char *);


// myalloc() called, must myfree()
#define SERIALIZE_TO_BUFFER(OUTPOINTER, OUTSIZE, ITEM, ITEMTYPE) \
    size_t OUTSIZE = ITEMTYPE ## __get_packed_size(ITEM); \
    uint8_t *OUTPOINTER = (uint8_t *) myalloc(OUTSIZE); \
    ITEMTYPE ## __pack(ITEM, OUTPOINTER);

extern void sha256sum(uint8_t *, size_t, char *);

extern void random_string(uint8_t *, size_t);
extern int OCD_satisfied(uint8_t *, size_t);

extern void hexdump(void *, int);
extern char *strip(char *);
#endif /* __TOOLS_H__ */
