/* $Id: xdr_stdio.c,v 1.3 2004/12/30 22:19:18 alex Exp $ */

/*
 * Sun RPC is a product of Sun Microsystems, Inc. and is provided for
 * unrestricted use provided that this legend is included on all tape
 * media and as a part of the software program in whole or part.  Users
 * may copy or modify Sun RPC without charge, but are not authorized
 * to license or distribute it to anyone else except as part of a product or
 * program developed by the user.
 *
 * SUN RPC IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING THE
 * WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun RPC is provided with no support and without any obligation on the
 * part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY SUN RPC
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char *rcsid = "$OpenBSD: xdr_stdio.c,v 1.6 2001/09/17 18:34:51 jason Exp $";
#endif /* LIBC_SCCS and not lint */

/*
 * xdr_stdio.c, XDR implementation on standard i/o file.
 *
 * Copyright (C) 1984, Sun Microsystems, Inc.
 *
 * This set of routines implements a XDR on a stdio stream.
 * XDR_ENCODE serializes onto the stream, XDR_DECODE de-serializes
 * from the stream.
 */

#include <stdio.h>

#include <rpc/types.h>
#include <rpc/xdr.h>

#if defined (_WIN32)
#    include  <winsock.h>			/* Windows socket definitions. */
#elif defined (__palmos__)
#    include  <Unix/netinet_in.h>		/* htonl(3), ntohl(3), etc. */
#else
#    include  <netinet/in.h>			/* htonl(3), ntohl(3), etc. */
#endif

static bool_t	xdrstdio_getlong(XDR *, long *);
static bool_t	xdrstdio_putlong(XDR *, long *);
static bool_t	xdrstdio_getbytes(XDR *, caddr_t, unsigned int);
static bool_t	xdrstdio_putbytes(XDR *, caddr_t, unsigned int);
static unsigned int	xdrstdio_getpos(XDR *);
static bool_t	xdrstdio_setpos(XDR *, unsigned int);
static int32_t *xdrstdio_inline(XDR *, unsigned int);
static void	xdrstdio_destroy(XDR *);

/*
 * Ops vector for stdio type XDR
 */
static struct xdr_ops	xdrstdio_ops = {
	xdrstdio_getlong,	/* deseraialize a long int */
	xdrstdio_putlong,	/* seraialize a long int */
	xdrstdio_getbytes,	/* deserialize counted bytes */
	xdrstdio_putbytes,	/* serialize counted bytes */
	xdrstdio_getpos,	/* get offset in the stream */
	xdrstdio_setpos,	/* set offset in the stream */
	xdrstdio_inline,	/* prime stream for inline macros */
	xdrstdio_destroy	/* destroy stream */
};

/*
 * Initialize a stdio xdr stream.
 * Sets the xdr stream handle xdrs for use on the stream file.
 * Operation flag is set to op.
 */
void
xdrstdio_create(xdrs, file, op)
	XDR *xdrs;
	FILE *file;
	enum xdr_op op;
{

	xdrs->x_op = op;
	xdrs->x_ops = &xdrstdio_ops;
	xdrs->x_private = (caddr_t)file;
	xdrs->x_handy = 0;
	xdrs->x_base = 0;
}

/*
 * Destroy a stdio xdr stream.
 * Cleans up the xdr stream handle xdrs previously set up by xdrstdio_create.
 */
static void
xdrstdio_destroy(xdrs)
	XDR *xdrs;
{
	(void)fflush((FILE *)xdrs->x_private);
	/* xx should we close the file ?? */
}

static bool_t
xdrstdio_getlong(xdrs, lp)
	XDR *xdrs;
	long *lp;
{

	if (fread((caddr_t)lp, sizeof(int32_t), 1,
	    (FILE *)xdrs->x_private) != 1)
		return (FALSE);
	*lp = (long)ntohl((uint32_t)*lp);
	return (TRUE);
}

static bool_t
xdrstdio_putlong(xdrs, lp)
	XDR *xdrs;
	long *lp;
{
	long mycopy = (long)htonl((uint32_t)*lp);

	if (fwrite((caddr_t)&mycopy, sizeof(int32_t), 1,
	    (FILE *)xdrs->x_private) != 1)
		return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_getbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	unsigned int len;
{

	if ((len != 0) && (fread(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1))
		return (FALSE);
	return (TRUE);
}

static bool_t
xdrstdio_putbytes(xdrs, addr, len)
	XDR *xdrs;
	caddr_t addr;
	unsigned int len;
{

	if ((len != 0) && (fwrite(addr, (int)len, 1, (FILE *)xdrs->x_private) != 1))
		return (FALSE);
	return (TRUE);
}

static unsigned int
xdrstdio_getpos(xdrs)
	XDR *xdrs;
{

	return ((unsigned int) ftell((FILE *)xdrs->x_private));
}

static bool_t
xdrstdio_setpos(xdrs, pos)
	XDR *xdrs;
	unsigned int pos;
{

	return ((fseek((FILE *)xdrs->x_private, (long)pos, 0) < 0) ?
		FALSE : TRUE);
}

/* ARGSUSED */
static int32_t *
xdrstdio_inline(xdrs, len)
	XDR *xdrs;
	unsigned int len;
{

	/*
	 * Must do some work to implement this: must insure
	 * enough data in the underlying stdio buffer,
	 * that the buffer is aligned so that we can indirect through a
	 * long *, and stuff this pointer in xdrs->x_buf.  Doing
	 * a fread or fwrite to a scratch buffer would defeat
	 * most of the gains to be had here and require storage
	 * management on this buffer, so we don't do this.
	 */
	return (NULL);
}
