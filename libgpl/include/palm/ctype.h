/* $Id: ctype.h,v 1.1 2004/05/24 19:33:20 alex Exp $ */
/*******************************************************************************

    ctype.h

    Standard C Library Character-Classification Functions.

    NOTE: The macros defined here assume ASCII characters.

*******************************************************************************/

#ifndef  PALM_CTYPE_H		/* Has the file been INCLUDE'd already? */
#define  PALM_CTYPE_H  yes


#include  <PalmTypes.h>			/* Palm SDK header. */

#define  digittoint(c) \
	(isdigit ((c)) ? ((c) - '0') \
	: (isxdigit ((c)) ? (toupper ((c)) - 'A' + 10) : 0))

#define  isalnum(c) \
	(isalpha ((c)) || isdigit ((c)))

#define  isalpha(c) \
	(islower ((c)) || isupper ((c)))

#define  isascii(c) \
	(('\0' <= (c)) && ((c) <= '\0177'))

#define  isblank(c) \
	(((c) == ' ') || ((c) == '\t'))

#define  iscntrl(c) \
	((('\0' <= (c)) && ((c) <= '\037')) || ((c) == '\0177'))

#define  isdigit(c) \
	(('0' <= (c)) && ((c) <= '9'))

#define  isgraph(c) \
	(('!' <= (c)) && ((c) <= '~'))

#define  islower(c) \
	(('a' <= (c)) && ((c) <= 'z'))

#define  isprint(c) \
	(isgraph ((c)) || ((c) == ' '))

#define  ispunct(c) \
	(isgraph((c)) && !isalnum ((c)))

#define  isspace(c) \
	(((c) == ' ') || ((c) == '\t') || ((c) == '\f') \
	|| ((c) == '\n') || ((c) == '\r') || ((c) == '\v'))

#define  isupper(c) \
	(('A' <= (c)) && ((c) <= 'Z'))

#define  isxdigit(c) \
	(isdigit ((c)) || (('a' <= (c)) && ((c) <= 'f')) \
	|| (('A' <= (c)) && ((c) <= 'F')))

#define  toascii(c) \
	((c) & 0x7F)

#define  tolower(c) \
	(isupper((c)) ? (((c) - 'A') + 'a') : (c))

#define  toupper(c) \
	(islower((c)) ? (((c) - 'a') + 'A') : (c))


#endif				/* If this file was not INCLUDE'd previously. */
