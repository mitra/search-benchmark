/* $Id: anise.h,v 1.7 2012/05/06 22:26:01 alex Exp $ */
/*******************************************************************************

  anise.h

  All-In-One Server (ANISE) Definitions.

*******************************************************************************/

#ifndef  ANISE_H		/* Has the file been INCLUDE'd already? */
#define  ANISE_H  yes


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

extern  int  debug ;			/* Debug switch. */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  debug


#endif				/* If this file was not INCLUDE'd previously. */
