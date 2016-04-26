/* $Id: id3_util.h,v 1.3 2009/09/09 22:38:13 alex Exp alex $ */
/*******************************************************************************

    id3_util.h

    ID3 Tag Utilities.

*******************************************************************************/

#ifndef  ID3_UTIL_H		/* Has the file been INCLUDE'd already? */
#define  ID3_UTIL_H  yes

#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
extern  "C"  {
#endif


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */


/*******************************************************************************
    ID3 Tag (Client View) and Definitions.
*******************************************************************************/

typedef  struct  _Id3Tag  *Id3Tag ;	/* Tag handle. */

/* ID3 version specification is represented using the least significant 3 bytes
   of a long integer.  According to the informal ID3v2 standard, the first field
   has no name (e.g., v2), the second field is the major version number (e.g.,
   v2.3), and the third field is the revision number (e.g., v2.3.0). */

#define  ID3V_VERSION(version)  (((version) >> 16) & 0x0FFL)
#define  ID3V_MAJOR(version)  (((version) >> 8) & 0x0FFL)
#define  ID3V_REVISION(version)  ((version) & 0x0FFL)

typedef  long  Id3Version ;

#define  ID3V(version,major,revision)	\
	((((version) & 0x0FFL) << 16) |	\
	(((major) & 0x0FFL) << 8) |	\
	((revision) & 0x0FFL))

#define  ID3V_V1	ID3V (1, 255, 255)
#define  ID3V_V1_1	ID3V (1, 1, 0)
#define  ID3V_V2	ID3V (2, 255, 255)
#define  ID3V_V2_2	ID3V (2, 2, 0)
#define  ID3V_V2_3	ID3V (2, 3, 0)

#define  ID3_V1_TRAILER_SIZE  128
#define  ID3_V2_HEADER_SIZE  10


/*******************************************************************************
    Miscellaneous declarations.
*******************************************************************************/

					/* Global debug switch (1/0 = yes/no). */
extern  int  id3_util_debug  OCD ("id3_util") ;


/*******************************************************************************
    Public functions (tag creation, destruction, and file I/O).
*******************************************************************************/

extern  errno_t  id3Assign P_((Id3Tag tag,
                               const char *song,
                               const char *artist,
                               const char *album,
                               const char *comment,
                               long year,
                               ssize_t track,
                               ssize_t genre))
    OCD ("id3_util") ;

extern  errno_t  id3Create P_((Id3Version version,
                               Id3Tag *tag))
    OCD ("id3_util") ;

extern  errno_t  id3Destroy P_((Id3Tag tag))
    OCD ("id3_util") ;

extern  void  id3Encode P_((const Id3Tag tag,
                            bool initialize,
                            char *buffer))
    OCD ("id3_util") ;

extern  errno_t  id3Get P_((const char *pathname,
                            Id3Version version,
                            Id3Tag *tag))
    OCD ("id3_util") ;

extern  errno_t  id3GetF P_((FILE *file,
                             Id3Version version,
                             Id3Tag *tag))
    OCD ("id3_util") ;

extern  bool  id3IsEmpty P_((const Id3Tag tag))
    OCD ("id3_util") ;

extern  errno_t  id3Set P_((const char *pathname,
                            const Id3Tag tag))
    OCD ("id3_util") ;

extern  errno_t  id3SetF P_((FILE *file,
                             const Id3Tag tag))
    OCD ("id3_util") ;

extern  errno_t  id3Strip P_((const char *pathname,
                              Id3Version version))
    OCD ("id3_util") ;

extern  errno_t  id3StripF P_((FILE *file,
                               Id3Version version))
    OCD ("id3_util") ;

extern  Id3Version  id3Version P_((Id3Tag tag))
    OCD ("id3_util") ;


/*******************************************************************************
    Public functions (get/set fields).
*******************************************************************************/

extern  const  char  *id3Album P_((Id3Tag tag,
                                   const char *album))
    OCD ("id3_util") ;

extern  const  char  *id3Artist P_((Id3Tag tag,
                                    const char *artist))
    OCD ("id3_util") ;

extern  const  char  *id3Comment P_((Id3Tag tag,
                                     const char *comment))
    OCD ("id3_util") ;

extern  ssize_t  id3Genre P_((Id3Tag tag,
                              ssize_t number))
    OCD ("id3_util") ;

extern  const  char  *id3Song P_((Id3Tag tag,
                                  const char *song))
    OCD ("id3_util") ;

extern  ssize_t  id3Track P_((Id3Tag tag,
                              ssize_t track))
    OCD ("id3_util") ;

extern  long  id3Year P_((Id3Tag tag,
                          long track))
    OCD ("id3_util") ;

					/* Get flags from ID3v2 tag. */
extern  int  id3Flags P_((Id3Tag tag))
    OCD ("id3_util") ;
					/* Get size of ID3v2 tag. */
extern  ssize_t  id3Size P_((Id3Tag tag))
    OCD ("id3_util") ;


/*******************************************************************************
    Public functions (genre lookup).
*******************************************************************************/

extern  const  char  *id3FromGenre P_((ssize_t number))
    OCD ("id3_util") ;

extern  ssize_t  id3ToGenre P_((const char *name))
    OCD ("id3_util") ;


#ifdef __cplusplus		/* If this is a C++ compiler, use C linkage */
}
#endif

#endif				/* If this file was not INCLUDE'd previously. */
