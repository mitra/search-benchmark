/* $Id: wcs_util.c,v 1.4 2011/07/18 17:33:27 alex Exp $ */
/*******************************************************************************

File:

    wcs_util.c


Author:    Alex Measday


Purpose:

    These are a collection of wide-character string manipulation functions.


Procedures:

    wcscmp() - compares two wide-character strings.
    wcsncmp() - compares two wide-character strings for a specified length.
    wcscpy() - copies a NUL-terminated wide-character string.
    wcsncpy() - copies a wide-character string of a specified length.
    wcsdup() - duplicates a NUL-terminated wide-character string.
    wcsndup() - duplicates a wide-character string of a specified length.
    wcslen() - determines the length of a NUL-terminated wide-character string.
    mbstowcs() - converts a multi-byte string to a wide-character string.
    wcstombs() - converts a wide-character string to a multi-byte string.
    wcsNarrow() - converts a wide-character string to a string.
    wcsWiden() - converts a string to a wide-character string.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* Standard C string functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "wcs_util.h"			/* Wide-character string functions. */

#if defined(HAVE_WCSCMP) && !HAVE_WCSCMP
/*!*****************************************************************************

Procedure:

    wcscmp ()

    Compare Two Wide-Character Strings.


Purpose:

    Function wcscmp() compares two wide-character strings.  This is
    an ANSI C function, but it is not supported in all C libraries.


    Invocation:

        comparison = wcscmp (thisString, thatString) ;

    where:

        <thisString>	- I
        <thatString>	- I
            are the NUL-terminated wide-character strings being compared.
        <comparison>	- O
            returns one of three possible values:
                < 0 if THISSTRING is lexically less than THATSTRING,
                = 0 if the two strings are equal except for case, or
                > 0 if THISSTRING is lexically greater than THATSTRING.
            The "lexical" comparison is a simple-minded numerical comparison.

*******************************************************************************/


int  wcscmp (

#    if PROTOTYPES
        const  wchar_t  *thisString,
        const  wchar_t  *thatString)
#    else
        thisString, thatString)

        wchar_t  *thisString ;
        wchar_t  *thatString ;
#    endif

{    /* Local variables. */
    wchar_t  *that, *this ;



/* Check for NULL strings. */

    if ((thisString == NULL) && (thatString != NULL))
        return (-1) ;
    else if ((thisString == NULL) && (thatString == NULL))
        return (0) ;
    else if ((thisString != NULL) && (thatString == NULL))
        return (1) ;

/* Compare the two strings, character by character. */

    this = (wchar_t *) thisString ;  that = (wchar_t *) thatString ;
    while ((*this != (wchar_t) 0) && (*that != (wchar_t) 0)) {
        if (*this < *that)
            return (-1) ;
        else if (*this > *that)
            return (1) ;
        this++ ;  that++ ;
    }

/* The strings are identical as far as the shorter string goes.  Therefore,
   the shorter string is lexically less than the longer string. */

    if (*this != (wchar_t) 0)
        return (1) ;
    else if (*that != (wchar_t) 0)
        return (-1) ;
    else
        return (0) ;

}
#endif

#if defined(HAVE_WCSNCMP) && !HAVE_WCSNCMP
/*!*****************************************************************************

Procedure:

    wcsncmp ()

    Compare Up to N Characters of Two Wide-Character Strings.


Purpose:

    Function wcsncmp() performs a length-limited comparison of two
    wide-character strings.  This is an ANSI C function, but it is
    not supported in all C libraries.


    Invocation:

        comparison = wcsncmp (thisString, thatString, length) ;

    where:

        <thisString>	- I
        <thatString>	- I
            are the NUL-terminated wide-character strings being compared.
        <length>	- I
            is the number of wide characters in each string to examine.
        <comparison>	- O
            returns one of three possible values:
                < 0 if THISSTRING is lexically less than THATSTRING,
                = 0 if the two strings are equal except for case, or
                > 0 if THISSTRING is lexically greater than THATSTRING.
            The "lexical" comparison is a simple-minded numerical comparison.

*******************************************************************************/


int  wcsncmp (

#    if PROTOTYPES
        const  wchar_t  *thisString,
        const  wchar_t  *thatString,
        size_t  length)
#    else
        thisString, thatString, length)

        wchar_t  *thisString ;
        wchar_t  *thatString ;
        size_t  length ;
#    endif

{    /* Local variables. */
    wchar_t  *that, *this ;



/* Check for NULL strings. */

    if ((thisString == NULL) && (thatString != NULL))
        return (-1) ;
    else if ((thisString == NULL) && (thatString == NULL))
        return (0) ;
    else if ((thisString != NULL) && (thatString == NULL))
        return (1) ;

/* Compare the two strings, character by character. */

    this = (wchar_t *) thisString ;  that = (wchar_t *) thatString ;
    while ((*this != (wchar_t) 0) && (*that != (wchar_t) 0) && (length > 0)) {
        if (*this < *that)
            return (-1) ;
        else if (*this > *that)
            return (1) ;
        this++ ;  that++ ;  length-- ;
    }

/* The strings are identical for the first LENGTH characters or
   as far as the shorter string goes.  */

    if (length == 0)
        return (0) ;		/* First LENGTH characters are equal. */
    else if (*this != (wchar_t) 0)
        return (-1) ;		/* THIS < THAT. */
    else if (*that != (wchar_t) 0)
        return (1) ;		/* THAT < THIS. */
    else
        return (0) ;		/* wcslen() < LENGTH, but strings are equal. */

}
#endif

#if defined(HAVE_WCSCPY) && !HAVE_WCSCPY
/*!*****************************************************************************

Procedure:

    wcscpy ()

    Copy a NUL-Terminated Wide-Character String.


Purpose:

    Function wcscpy() copies a NUL-terminated wide-character string.  This
    is an ANSI C function, but it is not supported in all C libraries.


    Invocation:

        copy = wcscpy (destination, source) ;

    where:

        <destination>	- O
            receives the copy of the source string, including the NUL
            terminator.
        <source>	- I
            is the string to be copied to the destination string.
        <copy>		- O
            returns a pointer to the copied string, i.e., the destination
            string.

*******************************************************************************/


wchar_t  *wcscpy (

#    if PROTOTYPES
        wchar_t  *destination,
        const  wchar_t  *source)
#    else
        destination, source)

        wchar_t  *destination ;
        wchar_t  *source ;
#    endif

{

    if (destination == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(wcscpy) NULL destination: ") ;
        return (NULL) ;
    }

    if (source == NULL) {
        destination[0] = 0 ;
        return (destination) ;
    }

    (void) memcpy (destination, source,
                   (wcslen (source) + 1) * sizeof (wchar_t)) ;

    return (destination) ;

}
#endif

#if defined(HAVE_WCSNCPY) && !HAVE_WCSNCPY
/*!*****************************************************************************

Procedure:

    wcsncpy ()

    Copy a Wide-Character String of a Specified Length.


Purpose:

    Function wcsncpy() copies a specified number of characters from
    one wide-character string to another.  This is an ANSI C function,
    but it is not supported in all C libraries.


    Invocation:

        copy = wcsncpy (destination, source, length) ;

    where:

        <destination>	- O
            receives the copy of the source string.  If the source string
            is NUL-terminated and its length is less than the number of
            characters to be copied, then the destination string will be
            NUL-terminated.  Otherwise, the destination string will *not*
            be NUL-terminated.
        <source>	- I
            is the string to be copied to the destination string.
        <length>	- I
            is the number of characters to be copied.
        <copy>		- O
            returns a pointer to the copied string, i.e., the destination
            string.

*******************************************************************************/


wchar_t  *wcsncpy (

#    if PROTOTYPES
        wchar_t  *destination,
        const  wchar_t  *source,
        size_t  length)
#    else
        destination, source, length)

        wchar_t  *destination ;
        wchar_t  *source ;
        size_t  length ;
#    endif

{    /* Local variables. */
    size_t  i ;



    if (destination == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(wcsncpy) NULL destination: ") ;
        return (NULL) ;
    }

    if (source == NULL) {
        destination[0] = 0 ;
        return (destination) ;
    }

/* Copy the specified number of characters from the source to the destination.
   If a NUL terminator is encountered in the source string before the desired
   number of characters has been copied, then pad the remainder of the
   destination string with NUL characters. */

    for (i = 0 ;  i < length ;  i++) {		/* Copy characters. */
        if (source[i] == 0)  break ;
        destination[i] = source[i] ;
    }

    while (i < length) {			/* Pad with NUL characters. */
        destination[i++] = 0 ;
    }

    return (destination) ;

}
#endif

#if defined(HAVE_WCSDUP) && !HAVE_WCSDUP
/*!*****************************************************************************

Procedure:

    wcsdup ()

    Duplicate a NUL-Terminated Wide-Character String.


Purpose:

    Function wcsdup() duplicates a NUL-terminated wide-character string.
    wcsdup() is supported by some C libraries, but it is not part of the
    ANSI C library.


    Invocation:

        duplicate = wcsdup (wideString) ;

    where:

        <wideString>	- I
            is the wide-character string to be duplicated.
        <duplicate>	- O
            returns a MALLOC(3)ed copy of the input string.  The caller
            is responsible for FREE(3)ing the duplicate string.  NULL is
            returned in the event of an error.

*******************************************************************************/


wchar_t  *wcsdup (

#    if PROTOTYPES
        const  wchar_t  *wideString)
#    else
        wideString)

        wchar_t  *wideString ;
#    endif

{    /* Local variables. */
    wchar_t  *duplicate ;



    if (wideString == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(wcsdup) NULL string: ") ;
        return (NULL) ;
    }

    duplicate = (wchar_t *) calloc (wcslen (wideString) + 1, sizeof (wchar_t)) ;
    if (duplicate == NULL) {
        LGE "(wcsdup) Error duplicating %u-character wide string.\ncalloc: ",
            wcslen (wideString)) ;
        return (NULL) ;
    }

    return (wcscpy (duplicate, wideString)) ;

}
#endif

/*!*****************************************************************************

Procedure:

    wcsndup ()

    Duplicate a Wide-Character String of a Specified Length.


Purpose:

    Function wcsndup() duplicates a wide-character string of a specified
    length.  wcsndup() is the "n" counterpart of wcsdup().


    Invocation:

        duplicate = wcsndup (wideString, length) ;

    where:

        <wideString>	- I
            is the wide-character string to be duplicated.
        <length>	- I
            is the number of characters to be duplicated.
        <duplicate>	- O
            returns a MALLOC(3)ed copy of the input string; the duplicate is
            NUL-terminated.  The caller is responsible for FREE(3)ing the
            duplicate string.  NULL is returned in the event of an error.

*******************************************************************************/


wchar_t  *wcsndup (

#    if PROTOTYPES
        const  wchar_t  *wideString,
        size_t  length)
#    else
        wideString, length)

        wchar_t  *wideString ;
        size_t  length ;
#    endif

{    /* Local variables. */
    wchar_t  *duplicate ;



    duplicate = (wchar_t *) calloc (length + 1, sizeof (wchar_t)) ;
    if (duplicate == NULL) {
        LGE "(wcsndup) Error duplicating %u-character wide string.\ncalloc: ",
            length) ;
        return (NULL) ;
    }

    if (wideString == NULL) {
        duplicate[0] = 0 ;
    } else {
        wcsncpy (duplicate, wideString, length) ;
        duplicate[length] = 0 ;
    }

    return (duplicate) ;

}

#if defined(HAVE_WCSLEN) && !HAVE_WCSLEN
/*!*****************************************************************************

Procedure:

    wcslen ()

    Determine the Length of a NUL-Terminated Wide-Character String.


Purpose:

    Function wcslen() determines the length of a NUL-terminated wide-character
    string.  This is an ANSI C function, but it is not supported in all C
    libraries.


    Invocation:

        length = wcslen (wideString) ;

    where:

        <wideString>	- I
            is the wide-character string being measured.
        <length>	- O
            returns the number of wide characters in the string, not including
            the NUL terminator.

*******************************************************************************/


size_t  wcslen (

#    if PROTOTYPES
        const  wchar_t  *wideString)
#    else
        wideString)

        wchar_t  *wideString ;
#    endif

{    /* Local variables. */
    size_t  length ;



    if (wideString == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(wcslen) NULL wide-character string: ") ;
        return (0) ;
    }

    length = 0 ;
    while (wideString[length] != 0) {
        length++ ;
    }

    return (length) ;

}
#endif

#if defined(HAVE_MBSTOWCS) && !HAVE_MBSTOWCS
/*!*****************************************************************************

Procedure:

    mbstowcs ()

    Convert a Multi-Byte String to a Wide-Character String.


Purpose:

    Function mbstowcs() converts a NUL-terminated multi-byte string to
    a NUL-terminated wide-character string.  This is an ANSI C function,
    but it is not supported in all C libraries.

    This implementation of mbstowcs() simply converts a UTF-8-encoded
    string to a UNICODE string; as such, the implementation does NOT
    conform to the broader ANSI C definition of the function.


    Invocation:

        numWide = mbstowcs (wideString, narrowString, maxWide) ;

    where:

        <wideString>	- O
            specifies a wide-character buffer that will receive the converted
            wide-character string.  The buffer must be large enough to hold
            the converted string plus the NUL-terminating wide character.
            If this argument is NULL, mbstowcs() computes and returns the
            length of the wide-character string, but it does not actually
            perform the conversion.
        <narrowString>	- I
            is the multi-byte character string to be converted.
        <maxWide>	- I
            specifies the maximum number of wide characters, including the
            terminating NUL character, to be written out.  If the multi-byte
            character string would produce more wide characters than the
            maximum, the resulting wide-character string will NOT be
            NUL-terminated.  This argument is ignored if the wide string
            argument is NULL.
        <numWide>	- O
            returns the number of wide characters, not including the NUL
            terminator, resulting from the conversion.  -1 is returned
            if an error occurs.

*******************************************************************************/


size_t  mbstowcs (

#    if PROTOTYPES
        wchar_t  *wideString,
        const  char  *narrowString,
        size_t  maxWide)
#    else
        wideString, narrowString, maxWide)

        wchar_t  *wideString ;
        char  *narrowString ;
        size_t  maxWide ;
#    endif

{    /* Local variables. */
    size_t  i, numWide ;



    if (narrowString == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(mbstowcs) NULL multi-byte string: ") ;
        return ((size_t) -1) ;
    }

/*******************************************************************************
    Assume ASCII characters until I implement a full UTF-8 conversion.
*******************************************************************************/

    numWide = strlen (narrowString) ;
    if (wideString == NULL)  return (numWide) ;

    for (i = 0 ;  i <= numWide ;  i++) {
        if (i >= maxWide)  break ;
        wideString[i] = (wchar_t) narrowString[i] ;
    }

    return (i) ;

}
#endif

#if defined(HAVE_WCSTOMBS) && !HAVE_WCSTOMBS
/*!*****************************************************************************

Procedure:

    wcstombs ()

    Convert a Wide-Character String to a Multi-Byte String.


Purpose:

    Function wcstombs() converts a NUL-terminated wide-character string
    to a NUL-terminated multi-byte string.  This is an ANSI C function,
    but it is not supported in all C libraries.

    This implementation of wcstombs() simply converts a UNICODE string
    to a UTF-8-encoded string; as such, the implementation does NOT
    conform to the broader ANSI C definition of the function.


    Invocation:

        numNarrow = wcstombs (narrowString, wideString, maxNarrow) ;

    where:

        <narrowString>	- O
            specifies a multi-byte buffer that will receive the converted
            multi-byte string.  The buffer must be large enough to hold
            the converted string plus the NUL-terminating character.
            If this argument is NULL, wcstombs() computes and returns the
            length of the multi-byte string, but it does not actually
            perform the conversion.
        <wideString>	- I
            is the wide-character string to be converted.
        <maxNarrow>	- I
            specifies the maximum number of bytes, including the terminating
            NUL character, to be written out.  If the wide-character string
            would produce more bytes than the maximum, the resulting multi-byte
            string will NOT be NUL-terminated.  This argument is ignored if the
            narrow string argument is NULL.
        <numNarrow>	- O
            returns the number of bytes, not including the NUL terminator,
            resulting from the conversion.  -1 is returned if an error occurs.

*******************************************************************************/


size_t  wcstombs (

#    if PROTOTYPES
        char  *narrowString,
        const  wchar_t  *wideString,
        size_t  maxNarrow)
#    else
        narrowString, wideString, maxNarrow)

        char  *narrowString ;
        wchar_t  *wideString ;
        size_t  maxNarrow ;
#    endif

{    /* Local variables. */
    size_t  i, numNarrow ;



    if (wideString == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(wcstombs) NULL wide-character string: ") ;
        return ((size_t) -1) ;
    }

/*******************************************************************************
    Assume ASCII characters until I implement a full UTF-8 conversion.
*******************************************************************************/

    numNarrow = wcslen (wideString) ;
    if (narrowString == NULL)  return (numNarrow) ;

    for (i = 0 ;  i <= numNarrow ;  i++) {
        if (i >= maxNarrow)  break ;
        narrowString[i] = (char) wideString[i] ;
    }

    return (i) ;

}
#endif

/*!*****************************************************************************

Procedure:

    wcsNarrow ()

    Convert a Wide-Character String to a Multibyte-Character String.


Purpose:

    Function wcsNarrow() converts a wide-character string to a
    multibyte-character string.  The non-reentrant wcstombs(3)
    function is used to perform the conversion.


    Invocation:

        string = wcsNarrow (wideString, dynamic) ;

    where

        <wideString>	- I
            is the NUL-terminated wide-character string.
        <dynamic>	- I
            controls the disposition of the result.  If this argument is
            true (non-zero), wcsNarrow() returns a dynamically allocated
            string that the caller is responsible for free(3)ing.  If this
            argument is false (zero), the string is stored in memory local
            to wcsNarrow() and it should be used or duplicated before
            calling wcsNarrow() again.
        <string>	- O
            returns the multibyte-character string resulting from the
            conversion; NULL is returned in the event of an error.

*******************************************************************************/


char  *wcsNarrow (

#    if PROTOTYPES
        const  wchar_t  *wideString,
        bool  dynamic)
#    else
        wideString, dynamic)

        wchar_t  *wideString ;
        bool  dynamic ;
#    endif

{    /* Local variables. */
    char  *narrowString ;
    size_t  length ;
    static  char  *result = NULL ;
    static  size_t  resultMax = 0 ;



/* Determine the length of the narrowed string. */

    length = wcstombs (NULL, wideString, 0) ;
    if (length == (size_t) -1) {
        SET_ERRNO (EINVAL) ;
        LGE "(wcsNarrow) Unconvertable character in \"%ls\"\n", wideString) ;
        return (NULL) ;
    }
    length++ ;				/* For NUL terminator. */

/* Dynamically allocate a string for the result. */

    if (dynamic || (length > resultMax)) {
        narrowString = (char *) malloc (length) ;
        if (narrowString == NULL) {
            LGE "(wcsNarrow) Error allocating %u-byte string for \"%ls\"\nmalloc: ",
                length, wideString) ;
            return (NULL) ;
        }
        if (!dynamic) {
            if (result != NULL)  free (result) ;
            result = narrowString ;  resultMax = length ;
        }
    } else {
        narrowString = result ;
    }

/* Narrow the wide string. */

    wcstombs (narrowString, wideString, length) ;

    return (narrowString) ;

}

/*!*****************************************************************************

Procedure:

    wcsWiden ()

    Convert a Multibyte-Character String to a Wide-Character String.


Purpose:

    Function wcsWiden() converts a multibyte-character string to a
    wide-character string.  The non-reentrant mbstowcs(3) function
    is used to perform the conversion.


    Invocation:

        wideString = wcsWiden (narrowString, dynamic) ;

    where

        <narrowString>	- I
            is the NUL-terminated string to be widened.
        <dynamic>	- I
            controls the disposition of the result.  If this argument is
            true (non-zero), wcsWiden() returns a dynamically-allocated
            string that the caller is responsible for free(3)ing.  If this
            argument is false (zero), the string is stored in memory local
            to wcsWiden() and it should be used or duplicated before calling
            wcsWiden() again.
        <wideString>	- O
            returns the wide-character string resulting from the conversion;
            NULL is returned in the event of an error.

*******************************************************************************/


wchar_t  *wcsWiden (

#    if PROTOTYPES
        const  char  *narrowString,
        bool  dynamic)
#    else
        narrowString, dynamic)

        char  *narrowString ;
        bool  dynamic ;
#    endif

{    /* Local variables. */
    wchar_t  *wideString ;
    size_t  length ;
    static  wchar_t  *result = NULL ;
    static  size_t  resultMax = 0 ;



/* Dynamically allocate a wide string for the result. */

    length = strlen (narrowString) + 1 ;

    if (dynamic || (length > resultMax)) {
        wideString = (wchar_t *) calloc (length, sizeof (wchar_t)) ;
        if (wideString == NULL) {
            LGE "(wcsWiden) Error allocating %u-character wide string for \"%s\"\ncalloc: ",
                length, narrowString) ;
            return (NULL) ;
        }
        if (!dynamic) {
            if (result != NULL)  free (result) ;
            result = wideString ;  resultMax = length ;
        }
    } else {
        wideString = result ;
    }

/* Widen the string. */

    if (mbstowcs (wideString, narrowString, length) == (size_t) -1) {
        SET_ERRNO (EINVAL) ;
        LGE "(wcsWiden) Invalid multibyte character in \"%s\"\n",
            narrowString) ;
        return (NULL) ;
    }

    return (wideString) ;

}
