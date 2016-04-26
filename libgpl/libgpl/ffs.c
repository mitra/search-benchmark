/* $Id: ffs.c,v 1.5 2003/03/26 22:19:37 alex Exp $ */
/*******************************************************************************

Procedure:

    ffs ()

    Find First Bit Set.


Purpose:

    Function FFS returns the index of the first bit set in an integer.  Bits
    are numbered from 1 to N (number of bits in an INT), starting from the
    right.


    Invocation:

        index = ffs (value) ;

    where

        <value>
            is the integer value being examined.
        <index>
            returns the index 1..N of the first bit set, starting from the
            right.  Zero is returned if no bits are set.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */


int  ffs (

#    if PROTOTYPES
        int  value)
#    else
        value)

        int  value ;
#    endif

{    /* Local variables. */
    int  shift ;


    if (value == 0)  return (0) ;
    for (shift = 1 ;  !(value & 1) ;  shift++)
        value = value >> 1 ;
    return (shift) ;

}
