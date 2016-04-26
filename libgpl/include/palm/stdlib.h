/* $Id: stdlib.h,v 1.2 2004/12/30 21:41:00 alex Exp $ */
/*******************************************************************************

    stdlib.h

    Standard C Library Functions.

*******************************************************************************/

#ifndef  PALM_STDLIB_H		/* Has the file been INCLUDE'd already? */
#define  PALM_STDLIB_H  yes


#include  <FloatMgr.h>			/* Palm SDK header. */
#include  <MemoryMgr.h>			/* Palm SDK header. */
#include  <StringMgr.h>			/* Palm SDK header. */
#include  <unix_stdlib.h>		/* Palm SDK header. */
#include  "std_util.h"			/* Missing Standard C functions. */


#define  strtof(nptr,endptr) \
	((float) strtod (nptr, endptr))


/* Use atoi(3) for the strtol(3) and strtoul(3) functions; note that
   they always succeed. */

#define  strtol(nptr,endptr,base) \
	((((endptr) == NULL) ? 0L : (long) (*((char **) (endptr)) = (nptr) + strlen ((nptr)))), \
	(long) atoi ((nptr)))

#define  strtoul(nptr,endptr,base) \
	((((endptr) == NULL) ? 0UL : (unsigned long) (*((char **) (endptr)) = (nptr) + strlen ((nptr)))), \
	(unsigned long) atoi ((nptr)))


#endif				/* If this file was not INCLUDE'd previously. */
