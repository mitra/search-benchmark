/* $Id: ioctl_ucx.c,v 1.1 2004/03/24 19:34:19 alex Exp $ */
/*******************************************************************************

    ioctl() - VMS/UCX/VAX-C implementation borrowed from the UCX example code
        found in "UCS$EXAMPLE:UCX$IOCTL_ROUTINE.C".  As mentioned below, this
        is not a full-fledged implementation of ioctl().  It is only useful
        for sockets and possibly only with the UCX TCP/IP stack.

*******************************************************************************/

/*====================================================================
*
*			  COPYRIGHT (C) 1989 BY
*	      DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*
* This software is furnished under a license and may be used and  copied
* only  in  accordance  with  the  terms  of  such  license and with the
* inclusion of the above copyright notice.  This software or  any  other
* copies  thereof may not be provided or otherwise made available to any
* other person.  No title to and ownership of  the  software  is  hereby
* transferred.
*
* The information in this software is subject to change  without  notice
* and  should  not  be  construed  as  a commitment by DIGITAL EQUIPMENT
* CORPORATION.
*
* DIGITAL assumes no responsibility for the use or  reliability  of  its
* software on equipment which is not supplied by DIGITAL.
*
*
*
*  FACILITY:
*	INSTALL
*
*
*  ABSTRACT:
*
* ioctl: implements the unix ioctl call.
* This is not intended to be a full ioctl implementation.  Only the
* functions related to inet are of interest.
*
*
*
*  ENVIRONMENT:
*	UCX V1.0 or higher, VMS V4.7 or higher
*
*  AUTHORS:
*	UCX developer
*
*  CREATION DATE:
*	May 23, 1989
*
*  MODIFICATION HISTORY:
*
*	1 February 1993
*	Make sure the event flag is released on error.
*/


/*
*
*  INCLUDE FILES
*
*/
#include <errno.h>
#include <stdio.h>
#include <iodef.h>
#include  <ucx$inetdef.h>



/*
* Functional Description
*
*
* Ioctl's have the command encoded in the lower word,
* and the size of any in or out parameters in the upper
* word.  The high 2 bits of the upper word are used
* to encode the in/out status of the parameter; for now
* we restrict parameters to at most 128 bytes.
* The IOC_VOID field of 0x20000000 is defined so that new ioctls
* can be distinguished from old ioctls.
*
*
* Formal Parameters
*	d...file or socket descriptor
*	request...defined in ioctl.h
*	argp..."in" or "out" parameter
*
*
* Routine Value
*
*	Status code
*/

/*
*
*	MACRO DEFINITIONS
*
*/
#ifndef _IO
#define IOCPARM_MASK    0x7f            /* Parameters are < 128 bytes   */
#define IOC_VOID        (int)0x20000000 /* No parameters                */
#define IOC_OUT         (int)0x40000000 /* Copy out parameters          */
#define IOC_IN          (int)0x80000000/* Copy in parameters           */
#define IOC_INOUT       (int)(IOC_IN|IOC_OUT)
#define _IO(x,y)        (int)(IOC_VOID|('x'<<8)|y)
#define _IOR(x,y,t)     (int)(IOC_OUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define _IOW(x,y,t)     (int)(IOC_IN|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)|y)
#define _IOWR(x,y,t)    (int)(IOC_INOUT|((sizeof(t)&IOCPARM_MASK)<<16)|('x'<<8)`
#endif _IO

#define VMSOK(s) (s & 01)


/*------------------------------------------------------------*/
int ioctl(d, request, argp)
int d, request;
char *argp;
{
    int ef;			/* Event flag number */
    int sdc;			/* Socket device channel */
    unsigned short fun; 	/* Qiow function code  */
    unsigned short iosb[4];	/* Io status block */
    char *p5, *p6;			/* Args p5 & p6 of qiow */
    struct comm
    {
	int command;
	char *addr;
    } ioctl_comm;		/* Qiow ioctl commands. */
    struct it2
    {
	unsigned short len;
	unsigned short opt;
	struct comm *addr;
    } ioctl_desc;		/* Qiow ioctl commands descriptor */
    int status;

    /*
     * Gets an event flag for qio
     */
    status = LIB$GET_EF(&ef);
    if (!VMSOK(status))
    {
	/* No ef available. Use 0 */
	ef = 0;
    }

    /*
     * Get the socket device channel number.
     */
    sdc = vaxc$get_sdc(d);
    if (sdc == 0)
    {
	/* Not an open socket descriptor. */
	errno = EBADF;
	status = LIB$FREE_EF(&ef);
	return -1;
    }

    /*
     * Fill in ioctl descriptor.
     */
    ioctl_desc.opt = UCX$C_IOCTL;
    ioctl_desc.len = sizeof(struct comm);
    ioctl_desc.addr = &ioctl_comm;

    /*
     * Decide qio function code and in/out parameter.
     */
    if (request & IOC_OUT)
    {
	fun = IO$_SENSEMODE;
	p5 = 0;
	(struct it2 *)p6 = &ioctl_desc;
    }
    else
    {
	fun = IO$_SETMODE;
	(struct it2 *)p5 = &ioctl_desc;
	p6 = 0;
    }

    /*
     * Fill in ioctl command.
     */
    ioctl_comm.command = request;
    ioctl_comm.addr = argp;

    /*
     * Do ioctl.
     */
    status = SYS$QIOW(ef, sdc, fun, iosb, 0, 0,
		      0, 0, 0, 0,		/* p1 - p4: not used*/
		      p5, p6);

    if (!VMSOK(status))
    {
#ifdef DEBUG
	printf("ioctl failed: status = %d\n", status);
#endif DEBUG
	errno = status;
	status = LIB$FREE_EF(&ef);
	return -1;
    }

    if (!VMSOK(iosb[0]))
    {
#ifdef DEBUG
	printf("ioctl failed: status = %x, %x, %x%x\n", iosb[0], iosb[1],
	       iosb[3], iosb[2]);
#endif DEBUG
	errno = iosb[0];
    	status = LIB$FREE_EF(&ef);
	return -1;
    }

    status = LIB$FREE_EF(&ef);
    return 0;
}
