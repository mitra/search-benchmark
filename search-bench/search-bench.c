#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

double cycles = 600e6;

typedef struct {
  uint32_t key_sz;
  uint32_t val_sz;
  char     key[1];  //variable sized key
  // value follows key
} region_t;

int dcache;

#ifdef MPPA
/* benchmarking functions for MPPA */
#include <utask.h>
#include <vbsp.h>
#include <HAL/hal/hal.h>
#include <HAL/hal/cluster/dsu.h>
#include <inttypes.h>

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

#define CYCLES cycles
typedef struct {
  uint64_t start;
  uint64_t end;
} perf_t;

uint64_t mppa_read_timer() {
//  return INT64_MAX - utask_timer64_get_time();
	return __k1_read_dsu_timestamp();
}

void init_timer(perf_t *t) {}

void start_timer(perf_t *t) {
  t->start = mppa_read_timer();
}

void stop_timer(perf_t *t) {
  t->end = mppa_read_timer();
}

double usec_timer(perf_t *t) {
	return (1.0 *(t->end - t->start))*(1e6/(1.0*CYCLES));
}

void fix_cache(char *ptr, int sz, int cmpbytes, int cachsz) {
}

char *kill_cache(char *ptr) {
	return ptr;
}
#else

/* benchmarking functions for x86 */
#include "bmw_util.h"
typedef struct {
  BmwClock bm;
} perf_t;

void init_timer(perf_t *t) {}

void start_timer(perf_t *t) {
  bmwStart(&t->bm);
}

void stop_timer(perf_t *t) {
  bmwStop(&t->bm);
}

double usec_timer(perf_t *t) {
  return bmwElapsed(&t->bm)*1e6;
}

/* defeat cache in x86 */
int  cache_num;
char *cache_ptr;
uint64_t  rrcnt;
int  cache_sz;
int  cache_intrn;
int  cache_offset;

void
make_buf(char *buf, int size, char *target_key, int key_sz, int val_sz,
	 int *bycmp);


void fix_cache(char *ptr, int sz, int cmpbytes, int cachsz) {
	int cnt;
	char *curr;
	region_t *reg = (region_t *)ptr;
	int bycmp;

	cache_offset = offsetof(region_t, key) + reg->key_sz;
	cache_intrn = reg->val_sz/cache_offset;
	for (cnt = 0; cnt < cache_intrn; cnt++) {
		make_buf(ptr + cnt*cache_offset, sz, reg->key, reg->key_sz, reg->val_sz, &bycmp);
		assert(bycmp == cmpbytes);
	}

	assert(cmpbytes);
	if (cache_intrn) {
		cache_num = cachsz/(cmpbytes * cache_intrn);
	} else {
		cache_num = cachsz/cmpbytes;
	}

	cache_sz = sz;
	cache_ptr = malloc((uint64_t)sz * (cache_num + 1) + cache_offset);
	assert(cache_ptr);
	cnt = cache_num + 1;
	curr = cache_ptr;
	//printf("%p-%p %ld\n", cache_ptr, &cache_ptr[(uint64_t)sz * cache_num],(uint64_t)sz * cache_num);
	//printf("%d %d/%d = %d\n",sz, cachsz, cmpbytes, cache_num);
	while (cnt) {
		//printf("%p %p %d\n", cache_ptr, curr, cnt);
		memcpy(curr, ptr, sz);
		curr += sz;
		cnt--;
	}
	rrcnt = 0;
}

char *kill_cache(char *ptr) {
	int intrn;
	int page;

	if (dcache)
		return ptr;
	page = rrcnt % cache_num;
	intrn = 0;
	if (cache_intrn) {
		intrn = (rrcnt / cache_num) % cache_intrn;
	}
	rrcnt++;
	return cache_ptr + page * cache_sz + intrn;
}

#endif /* MPPA */

void
print_key(char *buf, int size)
{
  int cnt;

  printf("key: ");
  for (cnt = 0; cnt < size; cnt++) {
    printf("%x", buf[cnt]);
  }
  printf(" ");
}

void
make_buf(char *buf, int size, char *target_key, int key_sz, int val_sz,
	 int *bycmp)
{
  char     *smallkey;
  region_t *tuple, *last;
  char     *curr;

  *bycmp = 0;
  smallkey = malloc(key_sz);
  assert(smallkey);
  memcpy(smallkey, target_key, key_sz);
  smallkey[key_sz-1]--;
  tuple = (region_t *) buf;
  last = tuple;
  curr = (char *)tuple;
  assert( (curr + 8) == tuple->key);
  while ((curr + 8 + key_sz) < (buf + size)) {
    tuple->key_sz = key_sz;
    tuple->val_sz = val_sz;
    memcpy(tuple->key, smallkey, key_sz);
    *bycmp += key_sz;
    curr = tuple->key;
    last = tuple;
    curr = curr + key_sz + val_sz;
    tuple = (region_t *)curr;
  }
  memcpy(last->key, target_key, key_sz);
  free(smallkey);
}

region_t *
search(char *buf,  int size, char *key, int key_sz)
{
  region_t *tuple;
  char     *curr;
  int      found = 0;
  int      cmpsz;

  curr  = buf;
  tuple = (region_t *)buf;
  assert( (curr + 8) == tuple->key);
  while ((curr + 8 + key_sz) < (buf + size)) {
    cmpsz = key_sz;
    if (tuple->key_sz < cmpsz) {
      cmpsz = tuple->key_sz;
    }
    #ifdef MPPA
    if (jsmemcmp(tuple->key, key, cmpsz) == 0) {
      found = 1;
      break;
    }
    #else
    if (memcmp(tuple->key, key, cmpsz) == 0) {
      found = 1;
      break;
    }
    #endif
    curr = tuple->key;
    curr = curr + tuple->key_sz + tuple->val_sz;
    tuple = (region_t *)curr;
  }
  if (!found) {
    tuple = 0;
  }
  return tuple;
}

void
search_bench (char *buf, int size, int rep, char *key, int key_sz,
	      int val_sz, double *usec, int *bycmp)
{
	perf_t   bm;
	char     *ptr, *tmp;
	region_t *r;

	init_timer(&bm);
	ptr = malloc(size + 8 + key_sz + val_sz);
	assert(ptr);
	make_buf(ptr, size, key, key_sz, val_sz, bycmp);
	fix_cache(ptr, size, *bycmp, 256*1024*1024);
	start_timer(&bm);
	while(rep--) {
		tmp = kill_cache(ptr);
		r = search(tmp, size, key, key_sz);
		assert(r || 1);
	}
	stop_timer(&bm);
	*usec = usec_timer(&bm);
}

/* Parameters to main
 * 1) MHz of MPPA processor
 * 2) key size
 * 3) value size
 * 4) block size
 * 5) Rep count 
 * 6) dcache 0-disable 1-enable*/

int
main(int argc, char *argv[])
{
	int key_sz, value_sz, blk_sz, rep_cnt;
	char *ptr;
	char *key;
	double  usec;
	int bycmp;

	if (argc != 7) {
		printf("incorrect arguments, check source code %d\n", argc);
		assert(0);
	}
	/* 1st Param frequency in MHz i.e. 600 */
	cycles = atoi(argv[1]);
	cycles = cycles * 1e6; //convert to MHz

	/* 2nd Param key size in bytes */
	key_sz = atoi(argv[2]);

	/* 3rd Param value size in bytes */
	value_sz = atoi(argv[3]);

	/* 4th param block size */
	blk_sz = atoi(argv[4]);

	/* 5th param rep count */
	rep_cnt = atoi(argv[5]);

	/* 6th param, trash dcache */
	dcache = atoi(argv[6]);

	ptr = malloc(blk_sz);
	assert(ptr);
	key = malloc(key_sz);
	assert(key);
	memset(key, 0xff, key_sz);
	search_bench(ptr, blk_sz, rep_cnt, key, key_sz, value_sz,
		     &usec, &bycmp);
	printf("#python\nbmtime=%f\nbytecmp=%d\n", usec, bycmp);
	return 0;
}
