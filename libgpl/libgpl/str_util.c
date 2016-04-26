/* $Id: str_util.c,v 1.29 2011/07/18 17:34:06 alex Exp $ */
/*******************************************************************************

File:

    str_util.c


Author:    Alex Measday


Purpose:

    These are a collection of string manipulation functions, including
    some Standard C Library functions for platforms that don't have them.


Notes:

    These functions used to be part of the LIBALEX functions.  The following
    changes have been made:

      - Functions that take a length argument used to follow a convention
        that, if the length were zero, the function would determine the
        length by scanning the string for a NUL terminator.  This turned
        out to be a real nuisance if you had a need to handle zero-length
        strings ("").  The new convention is that,if the length argument
        is less than zero, the function will determine the length itself.


Procedures:

    strConvert() - scans a text string, converting "\<num>" sequences to
        the appropriate binary bytes.
    strDestring() - resolves quote-delimited elements in a string.
    strDetab() - converts tabs in a string to blanks.
    strEnv() - translates environment variable references in a string.
    strEtoA() - converts a string of EBCDIC characters to ASCII.
    strInsert() - inserts a substring in a string.
    strMatch() - a string compare function that handles abbreviations.
    strRemove() - removes a substring from a string.
    strToLower() - converts a string to all lower-case.
    strToUpper() - converts a string to all upper-case.
    strTrim() - trims trailing tabs and spaces from a string.

    memdup() - duplicates a memory block.
    stpcpy() - copies a string, points to the end.
    strlcat() - concatenates two strings, limited by length.
    strlcpy() - copies a string, limited by length.
    strcasecmp() - compares two strings, ignoring case.
    strncasecmp() - compares two strings for a specified length, ignoring case.
    strnchr() - finds the first occurrence of a character in a length-limited
        string.
    strncpym() - copies a length-limited string to a sized buffer.
    strdup() - duplicates a NUL-terminated string.
    strndup() - duplicates a string of a specified length.
    strrchr() - finds the last occurrence of a character in a string.
    strspn() - skips characters in a set of characters.
    strcspn() - skips characters not in a set of characters.
    strtok() - extracts tokens from a string.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* Standard C string functions. */
#if defined(HAVE_MEMCPY) && !HAVE_MEMCPY
#    define  memmove(dest,src,length)  bcopy(src,dest,length)
#endif
#include  "get_util.h"			/* "Get Next" functions. */
#include  "str_util.h"			/* String manipulation functions. */

/*!*****************************************************************************

Procedure:

    strConvert()

    Convert Escaped Sequences in a String.


Purpose:

    Function strConvert() scans an input string and replaces the following
    escaped sequences:

        \<num>  -  is replaced by a single character with the specified ASCII
                   code; e.g., "\32" is replaced by the space character (" "),
                   "\006" is replaced by the bell character, and "\0x7F" is
                   replaced by the rubout character.

        \<char> -  is replaced by the specified character; e.g., "\\" is
                   replaced by "\".

    The input string is changed in place; i.e., it is overwritten by the
    output string.


    Invocation:

        result = strConvert (string) ;

    where

        <string>
            is a writeable, NUL-terminated string to be scanned and converted.
        <result>
            returns the input string (which was converted in-place).

*******************************************************************************/


char  *strConvert (

#    if PROTOTYPES
        char  *string)
#    else
        string)

        char  *string ;
#    endif

{    /* Local variables. */
    char  c, *s, *t, *v ;



    s = t = string ;

    while (*s != '\0') {
        v = s ;
        if (*s == '\\') {		/* "\" character? */
            s++ ;
            c = (char) strtol (s, &v, 0) ;
            if (v == s) {		/* (V == S) if conversion error. */
                *t = *s++ ;		/* Escaped literal. */
                switch (*t) {
                case 'a':  *t++ = 0x07 ;  break ;
                case 'b':  *t++ = 0x08 ;  break ;
                case 'e':  *t++ = 0x1B ;  break ;
                case 'f':  *t++ = 0x0C ;  break ;
                case 'l':  *t++ = 0x0A ;  break ;
                case 'n':  *t++ = 0x0A ;  break ;
                case 'r':  *t++ = 0x0D ;  break ;
                case 't':  *t++ = 0x09 ;  break ;
                case 'v':  *t++ = 0x0B ;  break ;
                case 'z':  *t++ = 0x00 ;  break ;
                default:
                    t++ ;  break ;
                }
            } else {
                *t++ = c ;		/* "\nnn" sequence converted. */
                s = v ;
            }
        } else {			/* Non-"\" character. */
            *t++ = *s++ ;
        }
    }

    *t = '\0' ;

    return (string) ;

}

/*!*****************************************************************************

Procedure:

    strDestring ()

    Resolve Quote-Delimited Elements in a String.


Purpose:

    Function strDestring() scans a string, replacing quote-delimited
    substrings by the text within the quotes.  For example, assuming
    that the allowed quote characters were single quotes, double quotes,
    and curly braces, the following conversions would be produced by
    strDestring():

                ab		==>	ab
		"ab cd"		==>	ab cd
		ab"cd"		==>	abcd
		"ab"'cd'	==>	abcd
		"ab'cd"		==>	ab'cd
		{ab"Hello!"cd}	==>	ab"Hello!"cd


    Invocation:

        result = strDestring (string, length, quotes) ;

    where:

        <string>	- I
            is the string to be "destring"ed.
        <length>	- I
            is the length of the string to be "destring"ed.  If LENGTH
            is less than 0, the input string (STRING) is assumed to be
            NUL-terminated and the processing of the string will be done
            in place (i.e., the input string will be modified).  If LENGTH
            is greater than or equal to 0, then it specifies the number of
            characters of STRING that are to be processed.  In the latter
            case, strDestring() will dynamically allocate new storage to
            hold the processed string; the input string will not be touched.
        <quotes>	- I
            is a pointer to a character string that contains the allowable
            quote characters.  For example, single and double quotes (the UNIX
            shell quote characters) would be specified as "\"'".  If a left
            brace, bracket, or parenthesis is specified, strDestring() is
            smart enough to look for the corresponding right brace, bracket,
            or parenthesis.
        <result>	- O
            returns a pointer to the processed string.  If the LENGTH
            argument was less than zero, the "destring"ing was performed
            directly on the input string and RESULT simply returns the
            input STRING argument.  If the LENGTH argument was greater
            than or equal to zero, then the "destring"ing was performed
            on a copy of the input string and RESULT returns a pointer
            to this dynamically-allocated string.  In the latter case,
            the calling routine is responsible for FREE(3)ing the result
            string.  A static empty string ("") is returned in the event
            of an error.

*******************************************************************************/


char  *strDestring (

#    if PROTOTYPES
        char  *string,
        ssize_t  length,
        const  char  *quotes)
#    else
        string, length, quotes)

        char  *string ;
        ssize_t  length ;
        char  *quotes ;
#    endif

{    /* Local variables. */
    char  *eos, rh_quote, *s ;




    if (string == NULL)  return ("") ;
    if (quotes == NULL)  quotes = "" ;

    if (length >= 0) {				/* Make copy of input string. */
        s = strndup (string, length) ;
        if (s == NULL) {
            LGE "(strDestring) Error duplicating: \"%*s\"\nstrndup: ",
                length, string) ;
            return (NULL) ;
        }
        string = s ;
    }


/* Scan the new argument and determine its length. */

    for (s = string ;  *s != '\0' ;  s++) {

        if (strchr (quotes, *s) == NULL)	/* Non-quote character? */
            continue ;

        switch (*s) {				/* Determine right-hand quote. */
        case '{':  rh_quote = '}' ;  break ;
        case '[':  rh_quote = ']' ;  break ;
        case '(':  rh_quote = ')' ;  break ;
        default:
            rh_quote = *s ;  break ;
            break ;
        }

        eos = strchr (s+1, rh_quote) ;		/* Locate right-hand quote. */
        if (eos == NULL)			/* Assume quote at NUL terminator. */
            eos = s + strlen (s) ;
        else					/* Pull down one character. */
            (void) memmove (eos, eos+1, strlen (eos+1) + 1) ;

						/* Pull down one character. */
        (void) memmove (s, s+1, strlen (s+1) + 1) ;
        s = eos - 2 ;				/* 2 quotes gone! */

    }


/* Return the processed string to the calling routine. */

    return (string) ;

}

/*!*****************************************************************************

Procedure:

    strDetab ()


Purpose:

    Function strDetab() converts tabs in a string to blanks.


    Invocation:

        detabbedLength = strDetab (stringWithTabs, length, tabStops,
                                   stringWithoutTabs, maxLength) ;

    where

        <stringWithTabs>	- I/O
            is a pointer to the string containing tabs.
        <length>		- I
            specifies the length of the string containing tabs.  If LENGTH
            is less than zero, strDetab determines the length by scanning
            STRING_WITH_TABS for a terminating NUL character.
        <tabStops>		- I
            specifies the number of characters between tab stops.  The
            default is 8 characters.
        <stringWithoutTabs>	- I/O
            is a pointer to a string buffer that will receive the expanded
            string.  The string will always be NUL-terminated (and truncated
            to a length of MAX_LENGTH-1 if necessary).  If this argument is
            NULL, strDetab() performs the conversion in place on
            STRING_WITH_TABS, subject to the MAX_LENGTH restriction.
        <maxLength>		- I
            is the size of the STRING_WITHOUT_TABS buffer that will receive
            the expanded string.  If the STRING_WITHOUT_TABS pointer is NULL,
            then MAX_LENGTH specifies the maximum size of the STRING_WITH_TABS
            buffer.
        <detabbedLength>	- O
            returns the length of the expanded string.

*******************************************************************************/


size_t  strDetab (

#    if PROTOTYPES
        char  *stringWithTabs,
        ssize_t  length,
        int  tabStops,
        char  *stringWithoutTabs,
        size_t  maxLength)
#    else
        stringWithTabs, length, tabStops, stringWithoutTabs, maxLength)

        char  *stringWithTabs ;
        ssize_t  length ;
        int  tabStops ;
        char  *stringWithoutTabs ;
        size_t  maxLength ;
#    endif

{    /* Local variables. */
    char  *s ;
    int  numSpaces ;
    size_t  i ;



    if (stringWithTabs == NULL) {
        if (stringWithoutTabs != NULL)
            *stringWithoutTabs = '\0' ;
        return (0) ;
    }

    if (length < 0)  length = strlen (stringWithTabs) ;
    if (tabStops <= 0)  tabStops = 8 ;

    if (stringWithoutTabs == NULL)
        stringWithoutTabs = stringWithTabs ;
    else
        strncpym (stringWithoutTabs, stringWithTabs, length, maxLength) ;

/* For each tab character in the string, delete the tab character and insert
   the number of spaces necessary to shift the following text to the next
   tab stop. */

    for (i = 0, s = stringWithoutTabs ;
         (length-- > 0) && (*s != '\0') ;
         i++, s++) {
        if (*s != '\t')  continue ;
        numSpaces = tabStops - (i % tabStops) - 1 ;  *s = ' ' ;
        if (numSpaces > 0) {
            numSpaces = strInsert (NULL, numSpaces, 0, s, maxLength - i) ;
            s = s + numSpaces ;  i = i + numSpaces ;
        }
    }

    return (strTrim (stringWithoutTabs, -1)) ;

}

/*!*****************************************************************************

Procedure:

    strEnv ()


Purpose:

    Function strEnv() translates environment variables ("$<name>") and
    home directory references ("~") embedded in a string.  For example,
    if variable DG has been defined as "/usr/alex/dispgen", strEnv()
    will translate

                    "tpocc:$DG/page.tdl"

    as

                    "tpocc:/usr/alex/dispgen/page.tdl".

    Remember that C-Shell variables (defined using "set name = value") are
    NOT environment variables (defined using "setenv name value") and are
    NOT available to programs.  Define any variables you might need as
    environment variables.

    Environment variables can be nested, i.e., defined in terms of each other.
    Undefined environment variables are not an error and are assumed to have
    a value of "" (a zero-length string).


    Invocation:

        strEnv (string, length, &translation, maxLength) ;

    where

        <string>	- I
            is the string which contains environment variable references.
        <length>	- I
            is the length of the string.  If LENGTH is less than zero,
            strEnv() determines the length by scanning STRING for a
            terminating null character.
        <translation>	- O
            is the address of a buffer which will receive the translated
            string.
        <maxLength>	- I
            is the maximum length of the translation; i.e., the size of
            the translation buffer.

*******************************************************************************/


void  strEnv (

#    if PROTOTYPES
        const  char  *string,
        ssize_t  length,
        char  *translation,
        size_t  maxLength)
#    else
        string, length, translation, maxLength)

        char  *string ;
        ssize_t  length ;
        char  *translation ;
        size_t  maxLength ;
#    endif

{    /* Local variables. */
    char  follow, *name, *s ;
    size_t  i ;



    if (translation == NULL)  return ;
    if (string == NULL) {
        strcpy (translation, "") ;
        return ;
    }

    if (length < 0)  length = strlen (string) ;
    strncpym (translation, string, length, maxLength) ;

/* Scan through the string, replacing "~"s by the user's home directory and
   environment variables ("$<name>") by their values. */

    for (i = 0 ;  translation[i] != '\0' ;  ) {

        if ((translation[i] == '~') &&			/* "~" */
            ((i == 0) || (translation[i-1] == ':'))) {

            s = getenv ("HOME") ;			/* Get home directory. */
            if (s == NULL) {
                i++ ;					/* Insert "~". */
            } else {
                strRemove (1, i, translation) ;		/* Insert home directory. */
                strInsert (s, -1, i, translation, maxLength) ;
            }

        } else if ((translation[i] == '$') &&		/* "$<name>" */
                   ((i == 0) || !isalnum (translation[i-1]))) {

            name = &translation[i] ;			/* Extract "<name>". */
            name += strspn (name, "$./:[") ;
            length = strcspn (name, "$./:[") ;
            follow = name[length] ;  name[length] = '\0' ;
            s = getenv (name) ;				/* Lookup "<name>". */
            name[length] = follow ;
							/* Replace "$<name>" ... */
            strRemove (name - &translation[i] + length, i, translation) ;
            if (s != NULL)				/* ... by "<value>". */
                strInsert (s, -1, i, translation, maxLength) ;

        } else {					/* Normal character. */

            i++ ;

        }

    }

    return ;

}

/*!*****************************************************************************

Procedure:

    strEtoA ()


Purpose:

    Function strEtoA() converts an EBCDIC string to an ASCII string.  The
    conversion table for this program was created using the "dd conv=ascii"
    Unix program, so I hope it's right!


    Invocation:

        asciiString = strEtoA (ebcdicString, length) ;

    where

        <ebcdicString>	- I/O
            is a pointer to the EBCDIC string to be converted.  The
            EBCDIC-to-ASCII conversion is done in-place, so EBCDIC_STRING
            should be in writeable memory.
        <length>	- I
            specifies the length of the EBCDIC string.  If LENGTH is
            less than zero, strEtoA() determines the length by scanning
            EBCDIC_STRING for a terminating null character.
        <asciiString>	- O
            returns a pointer to the converted string.  Since the EBCDIC-to-
            ASCII conversion is done in-place, this pointer simply points to
            the input string, EBCDIC_STRING.

*******************************************************************************/


char  *strEtoA (

#    if PROTOTYPES
        char  *string,
        ssize_t  length)
#    else
        string, length)

        char  *string ;
        ssize_t  length ;
#    endif

{
    static  char  ebcdic_to_ascii[256] = {
        0, 1, 2, 3, -100, 9, -122, 127, -105, -115, -114, 11, 12, 13, 14, 15,
        16, 17, 18, 19, -99, -123, 8, -121, 24, 25, -110, -113, 28, 29, 30, 31,
        -128, -127, -126, -125, -124, 10, 23, 27, -120, -119, -118, -117, -116, 5, 6, 7,
        -112, -111, 22, -109, -108, -107, -106, 4, -104, -103, -102, -101, 20, 21, -98, 26,
        32, -96, -95, -94, -93, -92, -91, -90, -89, -88, 91, 46, 60, 40, 43, 33,
        38, -87, -86, -85, -84, -83, -82, -81, -80, -79, 93, 36, 42, 41, 59, 94,
        45, 47, -78, -77, -76, -75, -74, -73, -72, -71, 124, 44, 37, 95, 62, 63,
        -70, -69, -68, -67, -66, -65, -64, -63, -62, 96, 58, 35, 64, 39, 61, 34,
        -61, 97, 98, 99, 100, 101, 102, 103, 104, 105, -60, -59, -58, -57, -56, -55,
        -54, 106, 107, 108, 109, 110, 111, 112, 113, 114, -53, -52, -51, -50, -49, -48,
        -47, 126, 115, 116, 117, 118, 119, 120, 121, 122, -46, -45, -44, -43, -42, -41,
        -40, -39, -38, -37, -36, -35, -34, -33, -32, -31, -30, -29, -28, -27, -26, -25,
        123, 65, 66, 67, 68, 69, 70, 71, 72, 73, -24, -23, -22, -21, -20, -19,
        125, 74, 75, 76, 77, 78, 79, 80, 81, 82, -18, -17, -16, -15, -14, -13,
        92, -97, 83, 84, 85, 86, 87, 88, 89, 90, -12, -11, -10, -9, -8, -7,
        48, 49, 50, 51, 52, 53, 54, 55, 56, 57, -6, -5, -4, -3, -2, -1
    } ;
    char  *s ;



    if (length < 0)  length = strlen (string) ;

    for (s = string ;  length-- ;  s++)
        *s = ebcdic_to_ascii[0x00FF&*s] ;

    return (string) ;

}

/*!*****************************************************************************

Procedure:

    strInsert ()


Purpose:

    Function strInsert() inserts N characters of text at any position in
    a string.


    Invocation:

        numInserted = strInsert (substring, length, offset, string, maxLength) ;

    where

        <substring>	- I
            points to the substring that will be inserted in STRING.  If
            this argument is NULL, then LENGTH blanks will be inserted in
            STRING.
        <length>	- I
            is the length of SUBSTRING.  If LENGTH is less than zero, the
            length is determined by searching for the NUL terminator in
            SUBSTRING.
        <offset>	- I
            is the character offset (0..N-1) in STRING at which SUBSTRING
            will be inserted.
        <string>	- I/O
            points to the string into which text will be inserted.
        <maxLength>	- I
            is the size of the STRING buffer.  Text that would be shifted
            beyond the end of STRING is truncated.
        <numInserted>	- O
            returns the number of characters actually inserted.  Normally,
            this will just be the length of SUBSTRING.  If, however, the
            size of the STRING buffer is insufficient to accomodate the
            full shift required for the insertion, NUM_INSERTED will be
            less than the length of SUBSTRING.

*******************************************************************************/


size_t  strInsert (

#    if PROTOTYPES
        const  char  *substring,
        ssize_t  subLength,
        size_t  offset,
        char  *string,
        size_t  maxLength)
#    else
        substring, subLength, offset, string, maxLength)

        char  *substring ;
        ssize_t  subLength ;
        size_t  offset ;
        char  *string ;
        size_t  maxLength ;
#    endif

{    /* Local variables. */
    char  *s ;
    size_t  length ;




/* Make sure arguments are all valid. */

    if (string == NULL)  return (0) ;
    if ((substring != NULL) && (subLength < 0))
        subLength = strlen (substring) ;
    if (subLength == 0)  return (0) ;

/* Compute the number of characters following STRING[OFFSET] that can be
   shifted right to make room for SUBSTRING.  Stored in variable LENGTH,
   the number computed includes the NUL terminator at the end of the
   string (or an extraneous character if truncation will occur). */

    length = offset + subLength + strlen (&string[offset]) + 1 ;
    if (length > maxLength)  length = maxLength ;
    length = length - subLength - offset ;

/* If there is room enough in the string buffer for the substring to
   be inserted, then insert it.  Text following STRING[OFFSET] may be
   truncated, if necessary. */

    if (length > 0) {		/* Shift text N columns to the right. */
        for (s = &string[offset+length-1] ;  length-- > 0 ;  s--)
            s[subLength] = *s ;
        s = s + subLength ;
    }

/* If there is insufficient room in the string buffer to insert the full
   text of the substring, then insert whatever will fit from the substring.
   The original text following STRING[OFFSET] will be lost. */

    else {
        subLength = subLength + length - 1 ;
        s = &string[offset+subLength-1] ;
    }

/* Copy the substring into the string.  Variable S points to the end of the
   room made for inserting the substring.  For example, if the substring will
   be copied into character positions 2-7 of the target string, then S points
   to character position 7.  Variable SUB_LENGTH specifies the number of
   characters to copy from SUBSTRING. */

    length = subLength ;
    if (substring == NULL) {	/* Insert N blanks? */
        while (length-- > 0)
            *s-- = ' ' ;
    } else {				/* Insert N characters of text? */
        while (length-- > 0)
            *s-- = substring[length] ;
    }

    string[maxLength-1] = '\0' ;	/* Ensure NUL-termination in case of truncation. */

/* This function was extremely tedious to write.  The next time you extol the
   virtues of C, remember that this function would have been a one-liner in
   FORTRAN 77 or BASIC. */

    return (subLength) ;

}

/*!*****************************************************************************

Procedure:

    strMatch ()


Purpose:

    Function strMatch() matches a possibly-abbreviated target string against
    a model string.  For example, "C", "CO", "COO", and "COOK" are partial
    matches of "COOK"; "COOKS" is NOT a partial match of "COOK".


    Invocation:

        found = strMatch (target, model) ;

    where

        <target>
            is the string to be checked for matching against the model string.
            In the example above, "C", "CO", "COO", etc. are target strings.
        <model>
            is the model string against which the match is tested.  In the
            example above, "COOK" is the model string.
        <found>
            returns true (a non-zero value) if the target string is a partial
            or full match of the model string; false (zero) is returned if the
            target string bears no relation to the model string.

*******************************************************************************/


bool  strMatch (

#    if PROTOTYPES
        const  char  *target,
        const  char  *model)
#    else
        target, model)

        char  *target ;
        char  *model ;
#    endif

{    /* Local variables. */
    size_t  length ;



    length = strlen (target) ;
    if (length > strlen (model))
        return (false) ;			/* Target string is too long. */
    else if (strncmp (target, model, length))
        return (false) ;			/* No match. */
    else
        return (true) ;				/* Matched! */

}

/*!*****************************************************************************

Procedure:

    strRemove ()


Purpose:

    Function strRemove() removes N characters of text from any position in
    a string.


    Invocation:

        length = strRemove (numToRemove, offset, string) ;

    where

        <numToRemove>	- I
            is the number of characters to delete from the string.
        <offset>	- I
            is the character offset (0..N-1) in STRING at which the
            deletion will take place.
        <string>	- I/O
            points to the string from which text will be deleted.
        <length>	- O
            returns the length of the string after the deletion.

*******************************************************************************/


size_t  strRemove (

#    if PROTOTYPES
        size_t  numToRemove,
        size_t  offset,
        char  *string)
#    else
        numToRemove, offset, string)

        size_t  numToRemove ;
        size_t  offset ;
        char  *string ;
#    endif

{    /* Local variables. */
    size_t  length ;



/* Validate the arguments. */

    if (string == NULL)  return (0) ;
    length = strlen (string) ;
    if (offset >= length)  return (0) ;
    if (numToRemove > (length - offset))  numToRemove = length - offset ;

/* Remove the substring. */

    (void) memmove (&string[offset], &string[offset+numToRemove],
                    length - (offset + numToRemove) + 1) ;

    return (strlen (string)) ;

}

/*!*****************************************************************************

Procedures:

    strToLower ()


Purpose:

    Function strToLower() converts the characters in a string to lower
    case.  If the length argument is zero, the string is assumed to be
    NUL-terminated; otherwise, only LENGTH characters are converted.


    Invocation:

        result = strToLower (string, length) ;

    where

        <string>
            points to the string to be converted; the conversion is
            done in-place.
        <length>
            is the number of characters to be converted.  If LENGTH is less
            than zero, the entire string is converted up to the NUL terminator.
        <result>
            returns a pointer to the converted string; i.e., STRING.

*******************************************************************************/


char  *strToLower (

#    if PROTOTYPES
        char  *string,
        ssize_t  length)
#    else
        string, length)

        char  *string ;
        ssize_t  length ;
#    endif

{    /* Local variables. */
    unsigned  char  *s ;



    if (length < 0)  length = strlen (string) ;

    for (s = (unsigned char *) string ;  length-- > 0 ;  s++)
        if (isupper (*s))  *s = tolower (*s) ;

    return (string) ;

}

/*!*****************************************************************************

Procedures:

    strToUpper ()


Purpose:

    Function strToUpper() converts the characters in a string to upper
    case.  If the length argument is zero, the string is assumed to be
    NUL-terminated; otherwise, only LENGTH characters are converted.


    Invocation:

        result = strToUpper (string, length) ;

    where

        <string>
            points to the string to be converted; the conversion is
            done in-place.
        <length>
            is the number of characters to be converted.  If LENGTH is less
            than zero, the entire string is converted up to the NUL terminator.
        <result>
            returns a pointer to the converted string; i.e., STRING.

*******************************************************************************/


char  *strToUpper (

#    if PROTOTYPES
        char  *string,
        ssize_t  length)
#    else
        string, length)

        char  *string ;
        ssize_t  length ;
#    endif

{    /* Local variables. */
    unsigned  char  *s ;



    if (length < 0)  length = strlen (string) ;

    for (s = (unsigned char *) string ;  length-- > 0 ;  s++)
        if (islower (*s))  *s = toupper (*s) ;

    return (string) ;

}

/*!*****************************************************************************

Procedure:

    strTrim ()


Purpose:

    Function strTrim() trims trailing white space (blanks, tabs, and new-line
    characters) from a string.  If the length argument is less than zero, the
    string is assumed to be NUL-terminated; after trimming trailing white
    space, the NUL terminator is relocated to the new end of the string.  If
    the length argument is greater than or equal to zero, the string does NOT
    need to be NUL-terminated; after trimming trailing white space, the null
    terminator is NOT relocated.  In either case, strTrim() returns the length
    of the new string.


    Invocation:

            trimmedLength = strTrim (string, length) ;

    where

        <string>	- I/O
            is the string to be trimmed.  If the length argument is less
            than zero, STRING is assumed to be NUL-terminated and strTrim()
            will ***** relocate the NUL terminator *****.  If LENGTH is
            greater than or equal to zero, strTrim() will not relocate the
            NUL terminator; it simply computes the trimmed length.
        <length>	- I
            is the length, before trimming, of STRING.  If LENGTH is less
            than zero, STRING is assumed to be NUL-terminated.
        <trimmedLength>	- O
            returns the length of STRING with trailing blanks, tabs, and
            new-line characters removed.

*******************************************************************************/


size_t  strTrim (

#    if PROTOTYPES
        char  *string,
        ssize_t  length)
#    else
        string, length)

        char  *string ;
        ssize_t  length ;
#    endif

{    /* Local variables. */
    char  *s ;
    size_t  newLength ;



    newLength = (length < 0) ? strlen (string) : length ;
    s = string + newLength ;

    while ((s-- != string) && ((*s == ' ') || (*s == '\t') ||
                               (*s == '\n') || (*s == '\r')))
        newLength-- ;

    if (length < 0)  *++s = '\0' ;

    return (newLength) ;

}

#if !defined(HAVE_MEMDUP) || !HAVE_MEMDUP
/*!*****************************************************************************

Procedure:

    memdup ()

    Duplicate a Block of Memory.


Purpose:

    Function memdup() duplicates a memory block of a specified length.
    The memory for the copy is dynamically allocated.


    Invocation:

        duplicate = memdup (source, length) ;

    where:

        <source>	- I
            is the memory block to be duplicated.
        <length>	- I
            is the number of bytes to be duplicated.
        <duplicate>	- O
            returns a MALLOC(3)ed copy of the input memory block.  The
            caller is responsible for FREE(3)ing the duplicate block.
            NULL is returned in the event of an error.

*******************************************************************************/


void  *memdup (

#    if PROTOTYPES
        const  void  *source,
        size_t  length)
#    else
        source, length)

        void  *source ;
        size_t  length ;
#    endif

{    /* Local variables. */
    void  *duplicate ;



    duplicate = (void *) calloc ((length > 0) ? length : 1, 1) ;
    if (duplicate == NULL) {
        LGE "(memdup) Error duplicating %u-byte memory block %p.\ncalloc: ",
            length, source) ;
        return (NULL) ;
    }

    if (source != NULL)  (void) memcpy (duplicate, source, length) ;

    return (duplicate) ;

}
#endif

#if defined(HAVE_STPCPY) && !HAVE_STPCPY
/*!*****************************************************************************

Procedure:

    stpcpy ()

    Copy a String, Point to the End.


Purpose:

    Function stpcpy() copies a NUL-terminated string to a destination buffer
    and returns a pointer to the end of the destination string (i.e., the
    NUL terminator).  The function is useful for concatenating literal
    strings:

        char  sentence[128] ;
        stpcpy (stpcpy (stpcpy (sentence, "What "), " About"), " Bob?) ;


    Invocation:

        end = stpcpy (destination, source) ;

    where:

        <destination>	- O
            is the address of the buffer into which the string will be copied.
        <source>	- I
            is the NUL-terminated string to be copied.
        <end>		- O
            returns a pointer to the NUL terminator in the destination buffer.

*******************************************************************************/


char  *stpcpy (

#    if PROTOTYPES
        char  *destination,
        const  char  *source)
#    else
        destination, source)

        char  *destination ;
        char  *source ;
#    endif

{

    if (destination == NULL)  return (NULL) ;

    if (source != NULL) {
        while (*source != '\0') {
            *destination++ = *source++ ;
        }
    }

    *destination = '\0' ;

    return (destination) ;

}
#endif

#if defined(HAVE_STRLCAT) && !HAVE_STRLCAT
/*!*****************************************************************************

Procedure:

    strlcat ()

    Length-Limited String Concatenation.


Purpose:

    Function strlcat() appends a NUL-terminated string to another string.
    The resulting string is always NUL-terminated; if the length returned
    by strlcat() is equal to or greater than the buffer size, then the
    caller knows that the concatenated string was truncated.

    (See Todd C. Miller's and Theo de Raadt's paper, "strlcpy and strlcat -
    consistent, safe, string copy and concatenation".)


    Invocation:

        length = strlcat (destination, source, maximum) ;

    where:

        <destination>	- O
            is the NUL-terminated string to which the source string will be
            appended.
        <source>	- I
            is the NUL-terminated string to be appended to the destination
            string.
        <maximum>	- I
            is the maximum number of characters, including the NUL terminator,
            that the destination buffer can hold.
        <length>	- O
            returns the length of the concatenated string, not including the
            NUL terminator.

*******************************************************************************/


size_t  strlcat (

#    if PROTOTYPES
        char  *destination,
        const  char  *source,
        size_t  maximum)
#    else
        destination, source, maximum)

        char  *destination ;
        char  *source ;
        size_t  maximum ;
#    endif

{    /* Local variables. */
    char  *s ;
    size_t  length ;



    if (destination == NULL)  return (0) ;
    if (source == NULL)  source = "" ;

/* Locate the end of the destination string. */

    length = strlen (destination) ;
    if (length >= maximum)  return (maximum + strlen (source)) ;
    destination += length ;
    maximum -= length ;

/* Append the source string to the destination string. */

    s = (char *) source ;
    while (--maximum > 0) {
        if (*s == '\0')  break ;
        *destination++ = *s++ ;
    }

    *destination = '\0' ;

    if (maximum > 0)
        return (length + (s - source)) ;
    else
        return (length + (s - source) + strlen (s)) ;

}
#endif

#if defined(HAVE_STRLCPY) && !HAVE_STRLCPY
/*!*****************************************************************************

Procedure:

    strlcpy ()

    Length-Limited String Copy.


Purpose:

    Function strlcpy() copies a NUL-terminated string to a destination buffer
    of a specified size.  The copied string is always NUL-terminated; if the
    length returned by strlcpy() is equal to or greater than the buffer size,
    then the caller knows that the copied string was truncated.

    (See Todd C. Miller's and Theo de Raadt's paper, "strlcpy and strlcat -
    consistent, safe, string copy and concatenation".)


    Invocation:

        length = strlcpy (destination, source, maximum) ;

    where:

        <destination>	- O
            is the address of the buffer into which the string will be copied.
        <source>	- I
            is the NUL-terminated string to be copied.
        <maximum>	- I
            is the maximum number of characters, including the NUL terminator,
            that the destination buffer can hold.
        <length>	- O
            returns the length of the source string, not including the NUL
            terminator.

*******************************************************************************/


size_t  strlcpy (

#    if PROTOTYPES
        char  *destination,
        const  char  *source,
        size_t  maximum)
#    else
        destination, source, maximum)

        char  *destination ;
        char  *source ;
        size_t  maximum ;
#    endif

{    /* Local variables. */
    char  *s ;



    if (destination == NULL)  return (0) ;
    if (maximum < 1)  return (0) ;
    if (source == NULL)  source = "" ;

    s = (char *) source ;
    while (--maximum > 0) {
        if (*s == '\0')  break ;
        *destination++ = *s++ ;
    }

    *destination = '\0' ;

    if (maximum > 0)
        return (s - source) ;
    else
        return (s - source + strlen (s)) ;

}
#endif

#if defined(HAVE_STRCASECMP) && !HAVE_STRCASECMP
/*!*****************************************************************************

Procedure:

    strcasecmp ()

    Compare Two Strings, Ignoring Case.


Purpose:

    Function strcasecmp() compares two strings, ignoring the case of the
    individual characters.  strcasecmp() is supported by many C libraries,
    but it is not part of the ANSI C library.


    Invocation:

        comparison = strcasecmp (thisString, thatString) ;

    where:

        <thisString>	- I
        <thatString>	- I
            are the NUL-terminated strings being compared.
        <comparison>	- O
            returns one of three possible values:
                < 0 if THISSTRING is lexically less than THATSTRING,
                = 0 if the two strings are equal except for case, or
                > 0 if THISSTRING is lexically greater than THATSTRING.
            The less-than and greater-than relations are also case-insensitive.

*******************************************************************************/


int  strcasecmp (

#    if PROTOTYPES
        const  char  *thisString,
        const  char  *thatString)
#    else
        thisString, thatString)

        char  *thisString ;
        char  *thatString ;
#    endif

{    /* Local variables. */
    char  *that, *this ;



/* Check for NULL strings. */

    if ((thisString == NULL) && (thatString != NULL))
        return (-1) ;
    else if ((thisString == NULL) && (thatString == NULL))
        return (0) ;
    else if ((thisString != NULL) && (thatString == NULL))
        return (1) ;

/* Compare the two strings, character by character. */

    this = (char *) thisString ;  that = (char *) thatString ;
    while ((*this != '\0') && (*that != '\0')) {
        if (toupper (*this) < toupper (*that))
            return (-1) ;
        else if (toupper (*this) > toupper (*that))
            return (1) ;
        this++ ;  that++ ;
    }

/* The strings are identical (excepting case) as far as the shorter string
   goes.  Therefore, the shorter string is lexically less than the longer
   string. */

    if (*this != '\0')
        return (1) ;
    else if (*that != '\0')
        return (-1) ;
    else
        return (0) ;

}

/*!*****************************************************************************

Procedure:

    strncasecmp ()

    Compare Up to N Characters of Two Strings, Ignoring Case.


Purpose:

    Function strncasecmp() performs a length-limited comparison of two strings,
    ignoring the case of the individual characters.  strncasecmp() is supported
    by many C libraries, but it is not part of the ANSI C library.


    Invocation:

        comparison = strncasecmp (thisString, thatString, length) ;

    where:

        <thisString>	- I
        <thatString>	- I
            are the null- or length-terminated strings being compared.
        <length>	- I
            is the number of characters in each string to examine.
        <comparison>	- O
            returns one of three possible values:
                < 0 if THISSTRING is lexically less than THATSTRING,
                = 0 if the two strings are equal except for case, or
                > 0 if THISSTRING is lexically greater than THATSTRING.
            The less-than and greater-than relations are also case-insensitive.

*******************************************************************************/


int  strncasecmp (

#    if PROTOTYPES
        const  char  *thisString,
        const  char  *thatString,
        size_t  length)
#    else
        thisString, thatString, length)

        char  *thisString ;
        char  *thatString ;
        size_t  length ;
#    endif

{    /* Local variables. */
    char  *that, *this ;



/* Check for NULL strings. */

    if ((thisString == NULL) && (thatString != NULL))
        return (-1) ;
    else if ((thisString == NULL) && (thatString == NULL))
        return (0) ;
    else if ((thisString != NULL) && (thatString == NULL))
        return (1) ;

/* Compare the two strings, character by character. */

    this = (char *) thisString ;  that = (char *) thatString ;
    while ((*this != '\0') && (*that != '\0') && (length > 0)) {
        if (toupper (*this) < toupper (*that))
            return (-1) ;
        else if (toupper (*this) > toupper (*that))
            return (1) ;
        this++ ;  that++ ;  length-- ;
    }

/* The strings are identical (excepting case) for the first LENGTH characters
   or as far as the shorter string goes. */

    if (length == 0)
        return (0) ;		/* First LENGTH characters are equal. */
    else if (*this != '\0')
        return (-1) ;		/* THIS < THAT. */
    else if (*that != '\0')
        return (1) ;		/* THAT < THIS. */
    else
        return (0) ;		/* strlen() < LENGTH, but strings are equal. */

}
#endif

/*!*****************************************************************************

Procedure:

    strnchr ()

    Find the First Occurrence of a Character in a Length-Limited String.


Purpose:

    Function strnchr() locates the first occurrence of a character in a
    length-limited string.  This function is not part of the ANSI C library.


    Invocation:

        occurrence = strnchr (string, c, length) ;

    where:

        <string>	- I
            is the string to be searched.
        <c>		- I
            is the character to be located.
        <length>	- I
            is the maximum number of characters in the string to examine;
            a NUL character in the string terminates the search.
        <occurrence>	- O
            returns a pointer to the first occurrence of the character in
            the string; NULL is returned if the character is not found.

*******************************************************************************/


char  *strnchr (

#    if PROTOTYPES
        const  char  *string,
        int  c,
        size_t  length)
#    else
        string, c, length)

        char  *string ;
        int  c ;
        size_t  length ;
#    endif

{    /* Local variables. */
    char  *occurrence ;



    if (string == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(strnchr) NULL string: ") ;
        return (NULL) ;
    }

    occurrence = (char *) string ;
    while (length-- > 0) {
        if (*occurrence == c)  return (occurrence) ;
        if (*occurrence++ == '\0')  return (NULL) ;
    }

    return (NULL) ;

}

/*!*****************************************************************************

Procedure:

    strncpym ()

    Copy a Length-Limited String to a Size-Limited Buffer.


Purpose:

    Function strncpym() copies a length-limited string (which may or may not be
    NUL-terminated) to a buffer with a specified maximum size.  This function is
    similar to strncpy(), but the destination buffer has a maximum size and the
    destination string is *always* NUL-terminated.

    This function is not part of the ANSI C library.  I frequently find myself
    having to copy a substring to a buffer of a given size.  I used to have a
    strCopy() function, but I got rid of it when I added strlcpy(), a
    semi-standardardized function that I thought would solve all my problems -
    big mistake!  The latter function requires that the source string be
    NUL-terminated, so I was forced back to using strncpy() and explicitly
    NUL-terminating the destination string.


    Invocation:

        copy = strncpym (destination, source, length, maximum) ;

    where:

        <destination>	- O
            is the address of the buffer into which the string will be copied.
        <source>	- I
            is the possibly NUL-terminated string to be copied.
        <length>	- I
            is the maximum number of characters to copy from the source string
            without a NUL character being encountered.
        <maximum>	- I
            is the maximum number of characters, including the NUL terminator,
            that the destination buffer can hold.
        <copy>		- O
            returns a pointer to the destination string.  The destination string
            is always NUL-terminated.

*******************************************************************************/


char  *strncpym (

#    if PROTOTYPES
        char  *destination,
        const  char  *source,
        size_t  length,
        size_t  maximum)
#    else
        destination, source, length, maximum)

        char  *destination ;
        char  *source ;
        size_t  length ;
        size_t  maximum ;
#    endif

{

    if (destination == NULL)  return (NULL) ;
    if (maximum < 1)  return (NULL) ;
    if (source == NULL)  source = "" ;

    if (length > (maximum - 1))  length = maximum - 1 ;
    strncpy (destination, source, length) ;
    destination[length] = '\0' ;

    return (destination) ;

}

#if defined(HAVE_STRDUP) && !HAVE_STRDUP
/*!*****************************************************************************

Procedure:

    strdup ()

    Duplicate a NUL-Terminated String.


Purpose:

    Function strdup() duplicates a NUL-terminated string.  strdup() is
    supported by many C libraries, but it is not part of the ANSI C library.


    Invocation:

        duplicate = strdup (string) ;

    where:

        <string>	- I
            is the string to be duplicated.
        <duplicate>	- O
            returns a MALLOC(3)ed copy of the input string.  The caller
            is responsible for FREE(3)ing the duplicate string.  NULL is
            returned in the event of an error.

*******************************************************************************/


char  *strdup (

#    if PROTOTYPES
        const  char  *string)
#    else
        string)

        char  *string ;
#    endif

{    /* Local variables. */
    char  *duplicate ;



    if (string == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(strdup) NULL string: ") ;
        return (NULL) ;
    }

    duplicate = malloc (strlen (string) + 1) ;
    if (duplicate == NULL) {
        LGE "(strdup) Error duplicating %u-byte string.\n\"%s\"\nmalloc: ",
            strlen (string), string) ;
        return (NULL) ;
    }

    return (strcpy (duplicate, string)) ;

}
#endif

/*!*****************************************************************************

Procedure:

    strndup ()

    Duplicate a String of a Specified Length.


Purpose:

    Function strndup() duplicates a string of a specified length.  strndup()
    is the "n" counterpart of strdup(), as in strncmp(3) and strcmp(3).


    Invocation:

        duplicate = strndup (string, length) ;

    where:

        <string>	- I
            is the string to be duplicated.
        <length>	- I
            is the number of characters to be duplicated.
        <duplicate>	- O
            returns a MALLOC(3)ed copy of the input string; the duplicate
            is NUL-terminated.  The caller is responsible for FREE(3)ing
            the duplicate string.  NULL is returned in the event of an error.

*******************************************************************************/


char  *strndup (

#    if PROTOTYPES
        const  char  *string,
        size_t  length)
#    else
        string, length)

        char  *string ;
        size_t  length ;
#    endif

{    /* Local variables. */
    char  *duplicate ;



    duplicate = (char *) malloc (length + 1) ;
    if (duplicate == NULL) {
        LGE "(strndup) Error duplicating %u-byte string.\n\"%*s\"\nmalloc: ",
            length, length, string) ;
        return (NULL) ;
    }

    if (string == NULL) {
        duplicate[0] = '\0' ;
    } else {
        strncpy (duplicate, string, length) ;
        duplicate[length] = '\0' ;
    }

    return (duplicate) ;

}

#if defined(HAVE_STRRCHR) && !HAVE_STRRCHR
/*!*****************************************************************************

Procedure:

    strrchr ()

    Find the Last Occurrence of a Character in a NUL-Terminated String.


Purpose:

    Function strrchr() locates the last occurrence of a character in a
    NUL-terminated string.  Despite being part of the ANSI C library,
    strrchr() is not always available.


    Invocation:

        occurrence = strrchr (string, c) ;

    where:

        <string>	- I
            is the string to be searched.
        <c>		- I
            is the character to be located.
        <occurrence>	- O
            returns a pointer to the last occurrence of the character in the
            string; NULL is returned if the character is not found.

*******************************************************************************/


char  *strrchr (

#    if PROTOTYPES
        const  char  *string,
        int  c)
#    else
        string, c)

        char  *string ;
        int  c ;
#    endif

{    /* Local variables. */
    char  *occurrence ;



    if (string == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(strrchr) NULL string: ") ;
        return (NULL) ;
    }

    occurrence = (char *) string + strlen (string) ;
    while (occurrence != string) {
        if (*--occurrence == c)  return (occurrence) ;
    }

    return (NULL) ;

}
#endif

#if defined(HAVE_STRSPN) && !HAVE_STRSPN
/*!*****************************************************************************

Procedure:

    strspn ()

    Skip Characters in a Set of Characters.


Purpose:

    Function strspn() spans the initial segment of a string such that the
    characters in the initial segment are all from a caller-specified set
    of characters.  Despite being part of the ANSI C library, strspn() is
    not always available.


    Invocation:

        length = strspn (string, accept) ;

    where:

        <string>	- I
            is the string to be scanned.
        <accept>	- I
            is a string containing the characters allowed in the initial
            segment.
        <length>	- O
            returns the length of the initial segment containing only characters
            from the caller-specified set.

*******************************************************************************/


size_t  strspn (

#    if PROTOTYPES
        const  char  *string,
        const  char  *accept)
#    else
        string, accept)

        char  *string ;
        char  *accept ;
#    endif

{    /* Local variables. */
    char  *s, *t ;



    if ((string == NULL) || (accept == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(strspn) NULL string or accept set: ") ;
        return (0) ;
    }

    for (s = (char *) string ;  *s != '\0' ;  s++) {
        for (t = (char *) accept ;  *t != '\0' ;  t++) {
            if (*t == *s)  break ;		/* Character in set? */
        }
        if (*t != *s)  break ;			/* Character not in set? */
    }

    return (s - string) ;

}
#endif

#if defined(HAVE_STRCSPN) && !HAVE_STRCSPN
/*!*****************************************************************************

Procedure:

    strcspn ()

    Skip Characters Not in a Set of Characters.


Purpose:

    Function strcspn() spans the initial segment of a string such that the
    characters in the initial segment are all from the complement of a
    caller-specified set of characters.  Despite being part of the ANSI C
    library, strcspn() is not always available.


    Invocation:

        length = strcspn (string, reject) ;

    where:

        <string>	- I
            is the string to be scanned.
        <reject>	- I
            is a string containing the characters not allowed in the initial
            segment.
        <length>	- O
            returns the length of the initial segment containing characters
            not in the caller-specified set.

*******************************************************************************/


size_t  strcspn (

#    if PROTOTYPES
        const  char  *string,
        const  char  *reject)
#    else
        string, reject)

        char  *string ;
        char  *reject ;
#    endif

{    /* Local variables. */
    char  *s, *t ;



    if ((string == NULL) || (reject == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(strcspn) NULL string or reject set: ") ;
        return (0) ;
    }

    for (s = (char *) string ;  *s != '\0' ;  s++) {
        for (t = (char *) reject ;  *t != '\0' ;  t++) {
            if (*t == *s)  break ;		/* Character in set? */
        }
        if (*t == *s)  break ;			/* Character in set? */
    }

    return (s - string) ;

}
#endif

#if defined(HAVE_STRTOK) && !HAVE_STRTOK
/*!*****************************************************************************

Procedure:

    strtok ()

    Extract Tokens from a String.


Purpose:

    Function strtok() parses a string into tokens separated by a
    caller-specified set of delimiters.  Successive calls to strtok()
    return the tokens in order, one after another.  Despite being
    part of the ANSI C library, strtok() is not always available.


    Invocation:

        token = strtok (string, delimiters) ;

    where:

        <string>	- I
            is the string to be parsed.  The string must be writeable since
            strtok() NUL-terminates the next token being returned.  The string
            should only be specified on the first call to strtok() to parse
            the string; subsequent calls should pass in NULL so that strtok()
            knows to use its previously stored internal state.
        <delimiters>	- I
            is a string containing the characters allowed as delimiters between
            tokens.  The caller may supply different sets of delimiters as the
            scan of the string progresses.
        <token>		- O
            returns a pointer to the beginning of the next token in the string.
            The token is NUL-terminated, either by the end of the original
            string or by strtok() replacing the following delimiter by NUL.
            A NULL pointer is returned when the end of the string is reached.

*******************************************************************************/


char  *strtok (

#    if PROTOTYPES
        char  *string,
        const  char  *delimiters)
#    else
        string, delimiters)

        char  *string ;
        char  *delimiters ;
#    endif

{    /* Local variables. */
    char  *token ;
    static  char  *next = NULL ;	/* Where to begin scanning on next call. */



/* If this is not the initial call to parse a string, begin scanning after
   the last token returned. */

    if (string == NULL)  string = next ;

    if ((string == NULL) || (delimiters == NULL)) {
        SET_ERRNO (EINVAL) ;
        LGE "(strtok) NULL string or delimiter set: ") ;
        return (NULL) ;
    }

/* Skip any delimiters preceding the next token. */

    token = string + strspn (string, delimiters) ;

    if (*token == '\0') {		/* No more tokens? */
        next = NULL ;
        return (NULL) ;
    }

/* Locate the delimiter following the token. */

    next = token + strcspn (token, delimiters) ;

/* NUL-terminate the token. */

    if (*next != '\0')  *next++ = '\0' ;

    return (token) ;

}
#endif
