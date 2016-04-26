/* $Id: crlf_util.c,v 1.8 2016/02/19 04:19:04 alex Exp $ */
/*******************************************************************************

File:

    crlf_util.c

    Carriage Return/Line Feed Utilities.


Author:    Alex Measday


Purpose:

    The CRLF_UTIL functions perform various operations and transformations
    on strings containing carriage return and line feed characters.


Public Procedures:

    crlf2nl() - replaces each carriage return/line feed sequence in a
        string by a new-line character.
    nl2crlf() - replaces each new-line character in a string by a
        carriage return/line feed sequence.
    nlCount() - counts the number of new-line characters in a string.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* Standard C string functions. */
#if defined(HAVE_MEMCPY) && !HAVE_MEMCPY
#    define  memmove(dest,src,length)  bcopy(src,dest,length)
#endif
#include  "crlf_util.h"			/* CR/LF utilities. */

/*******************************************************************************

Procedure:

    crlf2nl ()

    Convert CR/LFs to Newlines.


Purpose:

    Function crlf2nl() scans a string, converting each occurrence of a
    carriage return/line feed sequence to a newline ("\n") character.
    Take note of the following subtleties:

        (1) A carriage return that is NOT followed by a line feed is
            left in the string as-is.

        (2) A lone carriage return at the very end of the string is
            stripped from the string.  If you are scanning buffers
            with successive calls to crlf2nl(), the carriage return
            will be restored in the next call to crlf2nl() (via the
            LASTCHAR argument) and handled normally.  Of course, a
            carriage return at the very end of the very last buffer
            (i.e., you won't be calling crlf2nl() again) must be
            restored manually by you.


    Invocation

        string = crlf2nl (string, length, &lastChar) ;

    where

        <string>	- I/O
            is the string to be converted.  The conversion is done in
            place.  If you may have CR/LF sequences spanning buffers,
            STRING should be sized to allow the insertion of one extra
            character.
        <length>	- I
            specifies the length of the string.  If LENGTH is less than
            zero, crlf2nl() determines the length by scanning the string
            for a null terminator.
        <lastChar>	- I/O
            was/is the last character examined.  On input, this argument
            is the last character examined in the previous string that
            was converted.  On output, this argument returns the last
            character examined in STRING.  This argument is provided for
            cases in which a carriage return/line feed sequence spans a
            buffer boundary.  If this is not a concern for you, this
            argument can be NULL.
        <string>	- O
            returns a pointer to the converted string; i.e., STRING.

*******************************************************************************/


char  *crlf2nl (

#    if PROTOTYPES
        char  *string,
        int  length,
        char  *lastChar)
#    else
        string, length, lastChar)

        char  *string ;
        int  length ;
        char  *lastChar ;
#    endif

{    /* Local variables. */
    char  prevChar, *s ;



    if (string == NULL)  return (NULL) ;
    if (length < 0)  length = strlen (string) ;
    string[length] = '\0' ;
    if (length == 0)  return (string) ;

/* Save the last character in the string for the next call to crlf2nl().
   If the last character is a carriage return, strip it from the string;
   it will be restored if necessary on the next call to crlf2nl(). */

    prevChar = (lastChar != NULL) ? *lastChar : '\0' ;
    if (lastChar != NULL)  *lastChar = string[length-1] ;
    if (string[length-1] == '\r')  string[--length] = '\0' ;

/* If the last character in the previous string was a carriage return
   (always stripped by crlf2nl()), then insert it at the beginning of
   this string. */

    if ((prevChar == '\r') && (string[0] != '\n')) {
        (void) memmove (string+1, string, ++length) ;
        string[0] = '\r' ;
    }

/* It's a beautiful spring day outside and I'm stuck in here putzing
   around with carriage returns and line feeds. */

    for (s = string ;  *s != '\0' ;  s++, length--) {
        if ((s[0] == '\r') && (s[1] == '\n'))
            (void) memmove (s, s+1, length--) ;
    }

    return (string) ;

}

/*******************************************************************************

Procedure:

    nl2crlf ()

    Convert Newlines to CR/LFs.


Purpose:

    Function nl2crlf() scans a string, replacing each newline ("\n")
    character with a carriage return/line feed sequence.


    Invocation

        string = nl2crlf (string, length, maxLength) ;

    where

        <string>	- I/O
            is the string to be converted.  The conversion is done in
            place.  STRING should be sized to allow the insertion of
            "nlCount(string)" extra characters.
        <length>	- I
            specifies the length of the string.  If LENGTH is less than
            zero, nl2crlf() determines the length by scanning the string
            for a null terminator.
        <maxLength>	- I
            is the maximum length to which the string can grow.
        <string>	- O
            returns a pointer to the converted string; i.e., STRING.

*******************************************************************************/


char  *nl2crlf (

#    if PROTOTYPES
        char  *string,
        int  length,
        int  maxLength)
#    else
        string, length, maxLength)

        char  *string ;
        int  length ;
        int  maxLength ;
#    endif

{    /* Local variables. */
    char  *s ;



    if (string == NULL)  return (NULL) ;
    if (length < 0)  length = strlen (string) ;
    string[length] = '\0' ;
    if (length == 0)  return (string) ;

/* Replace each newline character by a carriage return/line feed sequence. */

    for (s = string ;  *s != '\0' ;  s++, length--) {
        if ((s - string + 1) >= (maxLength - 1))  break ;
        if (*s == '\n') {
            (void) memmove (s+1, s, length+1) ;  *s++ = '\r' ;
        }
    }

    return (string) ;

}

/*******************************************************************************

Procedure:

    nlCount ()

    Count the Newlines in a String.


Purpose:

    Function nlCount() counts the number of newline ("\n") characters
    in a string.


    Invocation

        numNL = nlCount (string, length) ;

    where

        <string>	- I
            is the string to be examined.
        <length>	- I
            specifies the length of the string.  If LENGTH is less than
            zero, nlCount() determines the length by scanning the string
            for a null terminator.
        <numNL>		- O
            returns the number of newline ("\n") characters found in the
            string.

*******************************************************************************/


size_t  nlCount (

#    if PROTOTYPES
        const  char  *string,
        int  length)
#    else
        string, length)

        char  *string ;
        int  length ;
#    endif

{    /* Local variables. */
    size_t  numNL ;



    if (string == NULL)  return (0) ;
    if (length < 0)  length = strlen (string) ;

    for (numNL = 0 ;  (length-- > 0) && (*string != '\0') ;  string++)
        if (*string == '\n')  numNL++ ;

    return (numNL) ;

}
