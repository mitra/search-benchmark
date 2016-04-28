#ifndef _K_MEMCMP_
#define _K_MEMCMP_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define CONST const
#define VOID  void

int smemcmp(void *s1, void *s2, size_t n);


static inline
int
ismemcmp(void *s1, void *s2, size_t n) {
	unsigned char u1, u2;
	for ( ; n-- ; s1++, s2++) {
		u1 = * (unsigned char *) s1;
		u2 = * (unsigned char *) s2;
		if ( u1 != u2) {
			return (u1-u2);
		}
	}
	return 0;
}

static inline
int jsmemcmp(void * s1, void * s2, unsigned n)
{
  uint64_t u1 = *(const uint64_t*)s1;
  uint64_t u2 = *(const uint64_t*)s2;
  const uint64_t * p1 = (const uint64_t*)((const char*)s1 + (n & 7));
  const uint64_t * p2 = (const uint64_t*)((const char*)s2 + (n & 7));
  n = n >> 3;
  while (n > 0 && u1 == u2) {
    u1 = *(p1 ++);
    u2 = *(p2 ++);
    n --;
  }
  u1 = __builtin_bswap64(u1);
  u2 = __builtin_bswap64(u2);
  return (u1 > u2) - (u1 < u2);
}

#endif
