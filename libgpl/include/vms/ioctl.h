/* $Id: ioctl.h,v 1.1 2004/05/23 19:37:09 alex Exp $ */
/*******************************************************************************
    OS-/Compiler-specific wrappers around system header files.
*******************************************************************************/


#ifndef  IOCTL_H		/* Has the file been INCLUDE'd already? */
#define  IOCTL_H  yes

#ifndef UCX$C_TCP
#    include  <ucx$inetdef.h>		/* VMS/Ultrix Connection definitions. */
#endif

extern  int  ioctl (int, int, char*) ;

#endif				/* If this file was not INCLUDE'd previously. */
