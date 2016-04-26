/* $Id: std_util.c,v 1.1 2006/04/28 20:07:33 alex Exp $ */
/*******************************************************************************

File:

    std_util.c


Author:    Alex Measday


Purpose:

    These are a collection of Standard C Library functions (e.g., those
    declared in "stdlib.h") for platforms that don't have them.  Missing
    "str" string functions are found in "str_util.c".


Procedures:

    atof() - converts a string to a floating-point number.
    strtod() - converts a string to a floating-point number.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "str_util.h"			/* String manipulation functions. */

#if defined(HAVE_ATOF) && !HAVE_ATOF
/*!*****************************************************************************

Procedure:

    atof ()

    Convert a String to a Floating-Point Number.


Purpose:

    Function atof() converts the text representation of a floating-point
    number to its binary representation.  Despite being part of the ANSI C
    library, atof() is not always available.


    Invocation:

        number = atof (string) ;

    where:

        <string>	- I
            is the text representation of the floating-point number.
        <number>	- O
            returns the binary representation of the number.

*******************************************************************************/


double  atof (

#    if PROTOTYPES
        const  char  *nptr)
#    else
        nptr)

        char  *nptr ;
#    endif

{

    return (strtod (nptr, NULL)) ;

}
#endif

#if defined(HAVE_STRTOD) && !HAVE_STRTOD
/*!*****************************************************************************

Procedure:

    strtod ()

    Convert a String to a Floating-Point Number.


Purpose:

    Function strtod() converts the text representation of a floating-point
    number to its binary representation.  Despite being part of the ANSI C
    library, strtod() is not always available.


    Invocation:

        number = strtod (string, &follow) ;

    where:

        <string>	- I
            is the text representation of the floating-point number.
        <follow>	- O
            returns a pointer to the character in the input string that
            follows the last character used in the conversion.  If this
            argument is NULL, no pointer is returned.
        <number>	- O
            returns the binary representation of the number.

*******************************************************************************/


double  strtod (

#    if PROTOTYPES
        const  char  *string,
        char  **follow)
#    else
        string, follow)

        char  *string ;
        char  **follow ;
#    endif

{


#if defined(__palmos__) && (!defined(HAVE_FLPBUFFERATOF) || HAVE_FLPBUFFERATOF)

/* GCC doesn't handle return value of FlpAToF() correctly.
  FlpBufferAToF() was added in PalmOS 4, but it is not
  available in 3.5 ... must come up with a solution. */

    FlpCompDouble  composite ;
    FlpBufferAToF (&composite.fd, string) ;
    if (follow != NULL)
        *follow = (char *) string + strspn (string, "+-.0123456789eE") ;
    return (composite.d) ;

#else

/* ... to be implemented ... */

    return ((double) strtol (string, follow, 10)) ;

#endif

}
#endif
