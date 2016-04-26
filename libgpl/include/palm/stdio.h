/* $Id: stdio.h,v 1.2 2005/02/12 00:07:44 alex Exp $ */
/*******************************************************************************

    stdio.h

    Standard C Library I/O Functions.

*******************************************************************************/

#ifndef  PALM_STDIO_H		/* Has the file been INCLUDE'd already? */
#define  PALM_STDIO_H  yes


/* Symbol _STDIO_H is #define'd in "StdioPalm.h" to prevent "unix_stdio.h"
   from #define'ing stdin, stdout, and stderr as invalid file descriptors.
   In case "unix_stdio.h" happens to be #include'd before this "stdio.h",
   remove the definitions of stdin, stdout, and stderr. */

#if !defined(IGNORE_STDIO_STUBS) && !defined(_STDIO_H)
#    undef  stdin
#    undef  stdout
#    undef  stderr
#endif

#include  <SystemMgr.h>			/* Palm SDK header. */
#include  <StdIOPalm.h>			/* Palm SDK header. */
#include  <unix_stdio.h>		/* Palm SDK header. */

/* Additional supported/unsupported functions. */

#define  fileno(stream)  (((stream) == stdin) ? STDIN_FILENO : -1)
#define  fseek(stream, offset, whence)  (-1)
#define  ftell(stream)  (-1)


#endif				/* If this file was not INCLUDE'd previously. */
