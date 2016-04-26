#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


#ifdef MPPA
/* benchmarking functions for MPPA */
#include <utask.h>
#include <vbsp.h>
#include <HAL/hal/hal.h>
#include <HAL/hal/cluster/dsu.h>
#include <inttypes.h>
#define CYCLES 800e6
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

#endif /* MPPA */

typedef struct {
  uint32_t key_sz;
  uint32_t val_sz;
  char     key[1];  //variable sized key
  // value follows key
} region_t;

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
make_buf(char *buf, int size, char *target_key, int key_sz, int val_sz)
{
  char     *smallkey;
  region_t *tuple, *last;
  char     *curr;

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
    if (memcmp(tuple->key, key, cmpsz) == 0) {
      found = 1;
      break;
    }
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
	      int val_sz, double *usec)
{
	perf_t   bm;
	char     *ptr;
	region_t *r;

	init_timer(&bm);
	ptr = malloc(size);
	assert(ptr);
	make_buf(ptr, size, key, key_sz, val_sz);
	start_timer(&bm);
	while(rep--) {
		r = search(ptr, size, key, key_sz);
		assert(r);
	}
	stop_timer(&bm);
	*usec = usec_timer(&bm);
}

int
main(int argc, char *argv[])
{
  char *ptr;
  char *key;
  region_t *r;
  double  usec;

  ptr = malloc(64*1024);
  assert(ptr);
  printf("hello world\n");
  key = malloc(10);
  memset(key, 0xff, 10);
  make_buf(ptr, 64*1024, key, 10, 100);
  key[9] = 0xff;
  r = search(ptr, 64*1024, key, 10);
  assert(r);
  search_bench(ptr, 64*1024, 100, key, 10, 100, &usec);
  printf("it took %f usec\n", usec);
  return 0;
}
