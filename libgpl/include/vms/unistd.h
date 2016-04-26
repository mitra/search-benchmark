/* $Id: unistd.h,v 1.1 2004/05/23 19:37:09 alex Exp $ */
/*******************************************************************************
    OS-/Compiler-specific wrappers around system header files.
*******************************************************************************/


#ifndef  UNISTD_H		/* Has the file been INCLUDE'd already? */
#define  UNISTD_H  yes

#if defined(vaxc)
#    include  <unixio.h>			/* UNIX I/O definitions. */
#    include  <unixlib.h>			/* UNIX library definitions. */
#endif

#endif				/* If this file was not INCLUDE'd previously. */
