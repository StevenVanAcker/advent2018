#ifndef __TOOLS_H__
#define __TOOLS_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <memory.h>
#include "md5.h"
#include "sha1.h"
#include "sha256.h"

extern void __m5(uint8_t *, size_t, char *);
extern void __s1(uint8_t *, size_t, char *);
extern void __s2(uint8_t *, size_t, char *);

#endif /* __TOOLS_H__ */
