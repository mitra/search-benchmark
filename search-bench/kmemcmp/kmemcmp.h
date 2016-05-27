#ifndef KMEMCMP_H_
#define KMEMCMP_H_

#include <stddef.h>
#include <stdint.h>

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

static inline
int kmemcmp(const void *s1, const void *s2, size_t n)
{
  const uint64_t *u1 = (void *)s1;
  const uint64_t *u2 = (void *)s2;
  int i;
#define USZ	sizeof(*u1)
//#define ASM_CACHED
#ifdef ASM_CACHED
	  volatile int different = 0;
	  __asm__ __volatile__ (
		  "copy $r4 = %1\n\t"
		  ";;\n\t"  
		  "srl $r4 = $r4, 6\n\t"
		  ";;\n\t"  
		  "cb.eqz $r4, __not_done_%=\n\t"
		  "make $r59 = 0\n\t"
		  "make $r5 = 0\n\t"
		  ";;\n\t"  
		  "ldu.add.x8 $r32r33 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r34r35 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"  
		  "ldu.add.x8 $r36r37 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r38r39 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"  
		  "ldu.add.x8 $r40r41 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r42r43 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"  
		  "ldu.add.x8 $r44r45 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r46r47 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r48r49 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r50r51 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"  
		  "ldu.add.x8 $r52r53 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r54r55 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"  
		  "ldu.add.x8 $r56r57 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r58r59 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  "goto __start_loop_%=\n\t"
		  ";;\n\t"  
		  ".align 16\n\t"
		  "__start_loop_%=:\n\t"
		  "ldu.add.x8 $r60r61 = $r5[%2]\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r62r63 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t" 


		  "loopnez $r4, __loop_end_%=\n\t"  
		  ";;\n\t" 
		  "ldu.add.x8 $r32r33 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r32r33, $r34r35\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r34r35 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"

		  "ldu.add.x8 $r36r37 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r36r37, $r38r39\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r38r39 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t" 

		  "ldu.add.x8 $r40r41 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r40r41, $r42r43\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r42r43 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t" 

		  "ldu.add.x8 $r44r45 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r44r45, $r46r47\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r46r47 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"

		  "ldu.add.x8 $r48r49 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r48r49, $r50r51\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r50r51 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"

		  "ldu.add.x8 $r52r53 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r52r53, $r54r55\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r54r55 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"

		  "ldu.add.x8 $r56r57 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r56r57, $r58r59\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r58r59 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"

		  "ldu.add.x8 $r60r61 = $r5[%2]\n\t"
		  "compdl.ne $r6 = $r60r61, $r62r63\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t" 
		  "ldu.add.x8 $r62r63 = $r5[%3]\n\t"
		  "add $r5 = $r5, 1\n\t"
		  ";;\n\t"

		  "__loop_end_%=:\n\t"
		  "compdl.ne $r6 = $r32r33, $r34r35\n\t"
		  ";;\n\t"
		  "compdl.ne $r7 = $r36r37, $r38r39\n\t"	  
		  ";;\n\t"
		  "compdl.ne $r6 = $r40r41, $r42r43\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t"
		  "compdl.ne $r7 = $r44r45, $r46r47\n\t"
		  "cb.nez $r7, __done_%=\n\t"
		  ";;\n\t"
		  "compdl.ne $r6 = $r48r49, $r50r51\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t"
		  "compdl.ne $r7 = $r52r53, $r54r55\n\t"
		  "cb.nez $r7, __done_%=\n\t"
		  ";;\n\t"
		  "compdl.ne $r6 = $r56r57, $r58r59\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t"
		  "compdl.ne $r7 = $r60r61, $r62r63\n\t"
		  "cb.nez $r7, __done_%=\n\t"
		  ";;\n\t"
		  "cb.nez $r6, __done_%=\n\t"
		  ";;\n\t"
		  "cb.nez $r7, __done_%=\n\t"
		  ";;\n\t"
		  "goto __not_done_%=\n\t"
		  ";;\n\t"

		  "__done_%=:\n\t"
		  "make %0 = 1;\n\t"
		  ";;\n\t"
		  "__not_done_%=:\n\t"
		  "nop\n\t"
		  ";;\n\t"


		  : "=r" (different)
		  : "r" (n), "r" (u2), "r" (u1)
		  : "r4", "r5", "r6", "r7", "r32", "r33", "r34", "r35", "r36", "r37", "r38", "r39",	"r40", "r41", "r42", "r43", "r44", "r45", "r46", "r47", "r48", "r49", "r50", "r51", "r52", "r53", "r54", "r55",	"r56", "r57", "r58", "r59", "r60", "r61", "r62", "r63" );
	  if(different)
		  return 1;
#else

	  for (i=0; (ssize_t)i<((ssize_t)(n/USZ)); i+=8) {
#define CMP(x)  (u2[i+x] != u1[i+x])
		  if (CMP(0) + CMP(1) + CMP(2) + CMP(3)
			  + CMP(4) + CMP(5) + CMP(6) + CMP(7)) return 1;
#undef CMP
	  }
#endif
  
 
  for (i=((n/USZ)/8)*8; i<n/USZ; i++) {
    if (u1[i] != u2[i]) return 1;
  }

  return i*8 < n && ((u1[i] ^ u2[i]) & (((uint64_t)1 << (8 * (n % USZ))) - 1));
}

#undef USZ

#endif	/* KMEMCMP_H_ */


/*
	  "ld.add.x8 $r32r33 = $r63[%2]\n\t"
	  ";;\n\t"  
	  "ld.add.x8 $r34r35 = $r63[%3]\n\t"
	  "add $r63 = $r63, 1\n\t"
	  ";;\n\t" 

	  "ld.add.x8 $r36r37 = $r63[%2]\n\t"
	  ";;\n\t"
	  "ld.add.x8 $r38r39 = $r63[%3]\n\t"
	  "compdl.ne $r6 = $r32r33, $r34r35\n\t"
	  "add $r63, $r63, 1\n\t"
	  ";;\n\t"
	  "ld.add.x8 $r32r33 = $r63[%2]\n\t"
	  ";;\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  "compdl.ne $r6 = $r36r37, $r38r39\n\t"
	  "ld.add.x8 $r34r35 = $r63[%3]\n\t"
	  "add $r63, $r63, 1\n\t"
	  ";;\n\t"

	  "ld.add.x8 $r36r37 = $r63[%2]\n\t"
	  ";;\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  "compdl.ne $r6 = $r32r33, $r34r35\n\t"
	  "ld.add.x8 $r38r39 = $r63[%3]\n\t"
	  "add $r63, $r63, 1\n\t"
	  ";;\n\t"
	  "ld.add.x8 $r32r33 = $r63[%2]\n\t"
	  ";;\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  "compdl.ne $r6 = $r36r37, $r38r39\n\t"
	  "ld.add.x8 $r34r35 = $r63[%3]\n\t"
	  "add $r63, $r63, 1\n\t"
	  ";;\n\t"

	  "ld.add.x8 $r36r37 = $r63[%2]\n\t"
	  ";;\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  "compdl.ne $r6 = $r32r33, $r34r35\n\t"
	  "ld.add.x8 $r38r39 = $r63[%3]\n\t"
	  "add $r63, $r63, 1\n\t"
	  ";;\n\t"

	  "ld.add.x8 $r32r33 = $r63[%2]\n\t"
	  ";;\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  "compdl.ne $r6 = $r36r37, $r38r39\n\t"
	  "ld.add.x8 $r34r35 = $r63[%3]\n\t"
	  "add $r63, $r63, 1\n\t"
	  ";;\n\t"

	  "ld.add.x8 $r36r37 = $r63[%2]\n\t"
	  ";;\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  "compdl.ne $r6 = $r32r33, $r34r35\n\t"
	  "ld.add.x8 $r38r39 = $r63[%3]\n\t"
	  "add $r63, $r63, 1\n\t"
	  ";;\n\t"

	  "compdl.ne $r6 = $r36r37, $r38r39\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  ";;\n\t"
	  "cb.nez $r6, __done_%=\n\t"
	  "comp.lt $r58, $r63, $r62\n\t"
	  ";;\n\t"

*/
