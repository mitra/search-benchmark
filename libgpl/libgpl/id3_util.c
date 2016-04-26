/* $Id: id3_util.c,v 1.5 2012/05/20 10:36:15 alex Exp alex $ */
/*******************************************************************************

File:

    id3_util.c

    ID3 Tag Utilities.


Author:    Alex Measday


Purpose:

    The ID3_UTIL package ...


Public Procedures:

    id3Assign() - sets the fields in an ID3 tag.
    id3Create() - creates an empty ID3 tag.
    id3Destroy() - deletes an ID3 tag.
    id3Encode() - encodes an ID3 tag into its file format.
    id3Get() - gets the ID3 tag from a named file.
    id3GetF() - gets the ID3 tag from an open file.
    id3IsEmpty() - checks if an ID3 tag is empty.
    id3Set() - sets the ID3 tag in a named file.
    id3SetF() - sets the ID3 tag in an open file.
    id3Strip() - strips the ID3 tag from a named file.
    id3StripF() - strips the ID3 tag from an open file.
    id3Version() - gets an ID3 tag's version.

    id3Album() - gets/sets a field in an ID3 tag.
    id3Artist() - gets/sets a field in an ID3 tag.
    id3Comment() - gets/sets a field in an ID3 tag.
    id3Genre() - gets/sets a field in an ID3 tag.
    id3Song() - gets/sets a field in an ID3 tag.
    id3Track() - gets/sets a field in an ID3 tag.
    id3Year() - gets/sets a field in an ID3 tag.

    id3Flags() - gets an ID3v2 tag's header flags.
    id3Size() - gets an ID3v2 tag's size.

    id3FromGenre() - translates a genre number to its name.
    id3ToGenre() - translates a genre name to its number.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#if !defined(HAVE_FTRUNCATE) || HAVE_FTRUNCATE
#    if defined(vaxc)
	/* DEC-C has ftruncate() in "unistd.h"; need to figure out VAX-C. */
#        undef  HAVE_FTRUNCATE
#        define  HAVE_FTRUNCATE  0
#    elif defined(_WIN32)
#        include  <io.h>		/* Low-level I/O definitions. */
#        define  ftruncate  _chsize
#    else
#        include  <unistd.h>		/* UNIX I/O definitions. */
#    endif
#endif
#include  "fnm_util.h"			/* Filename utilities. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "id3_util.h"			/* ID3 tag utilities. */


int  id3_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  id3_util_debug

/*******************************************************************************
    ID3 Tag - contains the fields in an ID3 v1.1 tag.
*******************************************************************************/

#define  MAX_SONG_LENGTH  30
#define  MAX_ARTIST_LENGTH  30
#define  MAX_ALBUM_LENGTH  30
#define  MAX_YEAR_LENGTH  4
#define  MAX_COMMENT_LENGTH  28

#define  ID3_V2_UNSYNCHRONIZATION	0x80		/* ID3v2 flags */
#define  ID3_V2_EXTENDED_HEADER		0x40
#define  ID3_V2_EXPERIMENTAL		0x20

#define  ID3_DEFINED_MASK	0x000000FFL
#define  ID3_DEFINED_SONG	0x00000001L
#define  ID3_DEFINED_ARTIST	0x00000002L
#define  ID3_DEFINED_ALBUM	0x00000004L
#define  ID3_DEFINED_YEAR	0x00000008L
#define  ID3_DEFINED_COMMENT	0x00000010L
#define  ID3_DEFINED_TRACK	0x00000020L
#define  ID3_DEFINED_GENRE	0x00000040L

typedef  struct  _Id3Tag {
    Id3Version  version ;
    long  flags ;			/* Which fields are defined, etc. */
    char  song[MAX_SONG_LENGTH+1] ;
    char  artist[MAX_ARTIST_LENGTH+1] ;
    char  album[MAX_ALBUM_LENGTH+1] ;
    char  comment[MAX_COMMENT_LENGTH+1] ;
    long  year ;			/* 1970, for example. */
    ssize_t  track ;			/* 1..N */
    ssize_t  genre ;			/* 0..125 */
    int  v2Flags ;			/* ID3v2 flags */
    size_t  v2Size ;			/* ID3v2 tag size, excluding header */
}  _Id3Tag ;


/*******************************************************************************
    Genre Lookup Table.
*******************************************************************************/

typedef  struct  GenreMap {		/* For number/name mappings. */
    const  ssize_t  number ;
    const  char  *name ;
}  GenreMap ;

static  const  GenreMap  genreLUT[]  OCD ("id3_util")  = {
    { 0, "Blues" },
    { 1, "Classic Rock" },
    { 2, "Country" },
    { 3, "Dance" },
    { 4, "Disco" },
    { 5, "Funk" },
    { 6, "Grunge" },
    { 7, "Hip-Hop" },
    { 8, "Jazz" },
    { 9, "Metal" },
    { 10, "New Age" },
    { 11, "Oldies" },
    { 12, "Other" },
    { 13, "Pop" },
    { 14, "R&B" },
    { 15, "Rap" },
    { 16, "Reggae" },
    { 17, "Rock" },
    { 18, "Techno" },
    { 19, "Industrial" },
    { 20, "Alternative" },
    { 21, "Ska" },
    { 22, "Death Metal" },
    { 23, "Pranks" },
    { 24, "Soundtrack" },
    { 25, "Euro-Techno" },
    { 26, "Ambient" },
    { 27, "Trip-Hop" },
    { 28, "Vocal" },
    { 29, "Jazz+Funk" },
    { 30, "Fusion" },
    { 31, "Trance" },
    { 32, "Classical" },
    { 33, "Instrumental" },
    { 34, "Acid" },
    { 35, "House" },
    { 36, "Game" },
    { 37, "Sound Clip" },
    { 38, "Gospel" },
    { 39, "Noise" },
    { 40, "AlternRock" },
    { 40, "Alt. Rock" },	/* Alternative name. */
    { 41, "Bass" },
    { 42, "Soul" },
    { 43, "Punk" },
    { 44, "Space" },
    { 45, "Meditative" },
    { 46, "Instrumental Pop" },
    { 47, "Instrumental Rock" },
    { 48, "Ethnic" },
    { 49, "Gothic" },
    { 50, "Darkwave" },
    { 51, "Techno-Industrial" },
    { 52, "Electronic" },
    { 53, "Pop-Folk" },
    { 54, "Eurodance" },
    { 55, "Dream" },
    { 56, "Southern Rock" },
    { 57, "Comedy" },
    { 58, "Cult" },
    { 59, "Gangsta" },
    { 59, "Gangsta Rap" },	/* Alternative  name. */
    { 60, "Top 40" },
    { 61, "Christian Rap" },
    { 62, "Pop/Funk" },
    { 63, "Jungle" },
    { 64, "Native American" },
    { 65, "Cabaret" },
    { 66, "New Wave" },
    { 67, "Psychedelic" },	/* Correct spelling. */
    { 67, "Psychadelic" },	/* Incorrect spelling in standard. */
    { 68, "Rave" },
    { 69, "Showtunes" },
    { 70, "Trailer" },
    { 71, "Lo-Fi" },
    { 72, "Tribal" },
    { 73, "Acid Punk" },
    { 74, "Acid Jazz" },
    { 75, "Polka" },
    { 76, "Retro" },
    { 77, "Musical" },
    { 78, "Rock & Roll" },
    { 79, "Hard Rock" },
    { 80, "Folk" },
    { 81, "Folk-Rock" },
    { 82, "National Folk" },
    { 83, "Swing" },
    { 84, "Fast Fusion" },
    { 84, "Fast-Fusion" },	/* Alternative name. */
    { 85, "Bebop" },		/* Correct spelling. */
    { 85, "Bebob" },		/* Incorrect spelling in standard. */
    { 86, "Latin" },
    { 87, "Revival" },
    { 88, "Celtic" },
    { 89, "Bluegrass" },
    { 90, "Avantgarde" },
    { 91, "Gothic Rock" },
    { 92, "Progressive Rock" },
    { 93, "Psychedelic Rock" },
    { 94, "Symphonic Rock" },
    { 95, "Slow Rock" },
    { 96, "Big Band" },
    { 97, "Chorus" },
    { 98, "Easy Listening" },
    { 99, "Acoustic" },
    { 100, "Humour" },
    { 101, "Speech" },
    { 102, "Chanson" },
    { 103, "Opera" },
    { 104, "Chamber Music" },
    { 105, "Sonata" },
    { 106, "Symphony" },
    { 107, "Booty Bass" },
    { 108, "Primus" },
    { 109, "Porn Groove" },
    { 110, "Satire" },
    { 111, "Slow Jam" },
    { 112, "Club" },
    { 113, "Tango" },
    { 114, "Samba" },
    { 115, "Folklore" },
    { 116, "Ballad" },
    { 117, "Power Ballad" },
    { 118, "Rhythmic Soul" },
    { 119, "Freestyle" },
    { 120, "Duet" },
    { 121, "Punk Rock" },
    { 122, "Drum Solo" },
    { 123, "A Cappella" },	/* Correct spelling. */
    { 123, "A capella" },	/* Incorrect spelling in standard. */
    { 124, "Euro-House" },
    { 125, "Dance Hall" },
    { 126, "Goa" },
    { 127, "Drum & Bass" },
    { 128, "Club-House" },
    { 129, "Hardcore" },
    { 130, "Terror" },
    { 131, "Indie" },
    { 132, "BritPop" },
    { 133, "Afro Punk" },	/* Nice name for a bigoted joke entry.  Seriously! */
    { 133, "Afro-Punk" },	/* Alternative name for a bigoted joke entry. */
    { 134, "Polsk Punk" },
    { 135, "Beat" },
    { 136, "Christian Gangsta Rap" },
    { 137, "Heavy Metal" },
    { 138, "Black Metal" },
    { 139, "Crossover" },
    { 140, "Contemporary Christian" },
    { 141, "Christian Rock" },
    { 142, "Merengue" },
    { 143, "Salsa" },
    { 144, "Thrash Metal" },
    { 145, "Anime" },
    { 146, "JPop" },
    { 147, "Synthpop" },
    { 148, "Abstract" },
    { 149, "Art Rock" },
    { 150, "Baroque" },
    { 151, "Bhangra" },
    { 152, "Big Beat" },
    { 153, "Breakbeat" },
    { 154, "Chillout" },
    { 155, "Downtempo" },
    { 156, "Dub" },
    { 157, "EBM" },
    { 158, "Eclectic" },
    { 159, "Electro" },
    { 160, "Electroclash" },
    { 161, "Emo" },
    { 162, "Experimental" },
    { 163, "Garage" },
    { 164, "Global" },
    { 165, "IDM" },
    { 166, "Illbient" },
    { 167, "Industro-Goth" },
    { 168, "Jam Band" },
    { 169, "Krautrock" },
    { 170, "Leftfield" },
    { 171, "Lounge" },
    { 172, "Math Rock" },
    { 173, "New Romantic" },
    { 174, "Nu-Breakz" },
    { 175, "Post-Punk" },
    { 176, "Post-Rock" },
    { 177, "Psytrance" },
    { 178, "Shoegaze" },
    { 179, "Space Rock" },
    { 180, "Trop Rock" },
    { 181, "World Music" },
    { 182, "Neoclassical" },
    { 183, "Audiobook" },
    { 184, "Audio Theatre" },
    { 185, "Neue Deutsche Welle" },
    { 186, "Podcast" },
    { 187, "Indie Rock" },
    { 188, "G-Funk" },
    { 189, "Dubstep" },
    { 190, "Garage Rock" },
    { 191, "Psybient" },
    { 255, "None" },
    { -1, NULL }
} ;

/*!*****************************************************************************

Procedure:

    id3Assign ()

    Set the Fields in an ID3 Tag.


Purpose:

    Function id3Assign() sets the fields en masse in an ID3 tag.


    Invocation:

        status = id3Assign (tag, song, artist, album, comment,
                            year, track, genre) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <song>		- I
            specifies the song to be set in the ID3 tag.  The name stored in
            the tag is truncated to 30 characters.  To clear the field, specify
            a zero-length string ("").  To skip the field, specify NULL.
        <artist>	- I
            specifies the artist to be set in the ID3 tag.  The name stored in
            the tag is truncated to 30 characters.  To clear the field, specify
            a zero-length string ("").  To skip the field, specify NULL.
        <album>		- I
            specifies the album to be set in the ID3 tag.  The name stored in
            the tag is truncated to 30 characters.  To clear the field, specify
            a zero-length string ("").  To skip the field, specify NULL.
        <comment>	- I
            specifies the comment to be set in the ID3 tag.  The name stored in
            the tag is truncated to 28 characters.  To clear the field, specify
            a zero-length string ("").  To skip the field, specify NULL.
        <year>		- I
            specifies the year to be set in the ID3 tag.  To clear the field,
            specify 0; to skip the field, specify -1.
        <track>		- I
            specifies the track to be set in the ID3 tag.  To clear the field,
            specify 0; to skip the field, specify -1.
        <genre>		- I
            specifies the genre to be set in the ID3 tag.  To clear the field,
            specify 0; to skip the field, specify -1.
        <status>	- O
            returns the status of assigning the fields, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3Assign (

#    if PROTOTYPES
        Id3Tag  tag,
        const  char  *song,
        const  char  *artist,
        const  char  *album,
        const  char  *comment,
        long  year,
        ssize_t  track,
        ssize_t  genre)
#    else
        tag, song, artist, album, comment, year, track, genre)

        Id3Tag  tag ;
        char  *song ;
        char  *artist ;
        char  *album ;
        char  *comment ;
        long  year ;
        ssize_t  track ;
        ssize_t  genre ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Assign) NULL tag handle: ") ;
        return (errno) ;
    }

    id3Song (tag, song) ;
    id3Artist (tag, artist) ;
    id3Album (tag, album) ;
    id3Comment (tag, comment) ;
    id3Year (tag, year) ;
    id3Track (tag, track) ;
    id3Genre (tag, genre) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3Create ()

    Create an Empty ID3 Tag.


Purpose:

    The id3Create() function creates an empty ID3 tag (all fields cleared).


    Invocation:

        status = id3Create (version, &tag) ;

    where

        <version>	- I
            specifies the type (v1 or v2) of ID3 tag to create.
        <tag>		- O
            returns a handle for the new tag.  This handle is used in calls
            to the other ID3_UTIL functions.
        <status>	- O
            returns the status of creating the tag, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3Create (

#    if PROTOTYPES
        Id3Version  version,
        Id3Tag  *tag)
#    else
        version, tag)

        Id3Version  version ;
        Id3Tag  *tag ;
#    endif

{

/* Allocate a new tag structure. */

    *tag = (_Id3Tag *) calloc (1, sizeof (_Id3Tag)) ;
    if (*tag == NULL) {
        LGE "(id3Create) Error allocating tag structure.\ncalloc: ") ;
        return (errno) ;
    }

    (*tag)->version = version ;

/* Flag all the fields as undefined. */

    (*tag)->flags &= ~ID3_DEFINED_MASK ;

    LGI "(id3Create) Created version 0x%06lX tag %p.\n",
        version, (void *) *tag) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3Destroy ()

    Delete an ID3 Tag.


Purpose:

    Function id3Destroy() destroys a previously created ID3 tag.


    Invocation:

        status = id3Destroy (tag) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <status>	- O
            returns the status of deleting the tag, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3Destroy (

#    if PROTOTYPES
        Id3Tag  tag)
#    else
        tag)

        Id3Tag  tag ;
#    endif

{

    if (tag == NULL)  return (0) ;

    LGI "(id3Destroy) Deleting ID3 tag %p ...\n", (void *) tag) ;

    free (tag) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3Encode ()

    Encodes an ID3 Tag in its File Format.


Purpose:

    Function id3Encode() encodes an ID3 tag into a byte buffer.


    Invocation:

        id3Encode (tag, initialize, &buffer) ;

    where

        <tag>		- I
            is the handle for the ID3 tag to be encode.
        <initialize>	- I
            indicates whether or not id3Encode() should initialize the buffer.
            Multiple ID3 tags can be merged by specifying true on the call to
            id3Encode() for the first tag and false on the calls for the
            remaining tags.
        <buffer>	- O
            is the address of a buffer into which the ID3 tag is encoded.

*******************************************************************************/


void  id3Encode (

#    if PROTOTYPES
        const  Id3Tag  tag,
        bool  initialize,
        char  *buffer)
#    else
        tag, initialize, buffer)

        Id3Tag  tag ;
        bool  initialize ;
        char  *buffer ;
#    endif

{    /* Local variables. */
    size_t  i ;



/* If the buffer needs to be initialized, then do so. */

    if (buffer == NULL)  return ;

    if (initialize) {
        memset (buffer, 0, ID3_V1_TRAILER_SIZE) ;
        buffer[ID3_V1_TRAILER_SIZE-1] = ~0 ;	/* Unknown genre. */
    }

    i = 0 ;
    buffer[i++] = 'T' ;
    buffer[i++] = 'A' ;
    buffer[i++] = 'G' ;

    if (tag == NULL)  return ;

/* Encode the fields defined in the tag into the buffer; skip undefined fields. */

    if (tag->flags & ID3_DEFINED_SONG)
        strncpy (&buffer[i], tag->song, MAX_SONG_LENGTH) ;
    i += MAX_SONG_LENGTH ;

    if (tag->flags & ID3_DEFINED_ARTIST)
        strncpy (&buffer[i], tag->artist, MAX_ARTIST_LENGTH) ;
    i += MAX_ARTIST_LENGTH ;

    if (tag->flags & ID3_DEFINED_ALBUM)
        strncpy (&buffer[i], tag->album, MAX_ALBUM_LENGTH) ;
    i += MAX_ALBUM_LENGTH ;

    if (tag->flags & ID3_DEFINED_YEAR) {
        char  yearString[16] ;			/* Don't overflow into comment! */
        sprintf (yearString, "%4ld", tag->year) ;
        strncpy (&buffer[i], yearString, MAX_YEAR_LENGTH) ;
    }
    i += MAX_YEAR_LENGTH ;

    if (tag->flags & ID3_DEFINED_COMMENT)
        strncpy (&buffer[i], tag->comment, MAX_COMMENT_LENGTH) ;
    i += MAX_COMMENT_LENGTH ;

    buffer[i++] = 0 ;				/* Indicates version 1.1 tag. */

    if (tag->flags & ID3_DEFINED_TRACK)
        buffer[i] = (char) tag->track ;
    i++ ;

    if (tag->flags & ID3_DEFINED_GENRE)
        buffer[i] = (char) tag->genre ;
    i++ ;

    return ;

}

/*!*****************************************************************************

Procedure:

    id3Get ()

    Get the ID3 Tag from a Named File.


Purpose:

    The id3Get() function retrieves the ID3 tag, if any, from a specified file.


    Invocation:

        status = id3Get (pathname, version, &tag) ;

    where

        <pathname>	- I
            is the name of the file from which the ID3 tag will be read.
            Environment variables may be embedded in the file name.
       <version>	- I
            specifies which ID3 tag (v1 or v2) to get from the file.
        <tag>		- O
            returns a handle for the ID3 tag in the file.  This handle is
            used in calls to the other ID3_UTIL functions; it should be
            id3Destroy()ed when no longer needed.  If the file has no ID3 tag,
            a NULL handle and an error status of success (0) is returned.
        <status>	- O
            returns the status of getting the tag from the file,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3Get (

#    if PROTOTYPES
        const  char  *pathname,
        Id3Version  version,
        Id3Tag  *tag)
#    else
        pathname, version, tag)

        char  *pathname ;
        Id3Version  version ;
        Id3Tag  *tag ;
#    endif

{    /* Local variables. */
    FILE  *file ;



/* Open the file. */

    pathname = fnmBuild (FnmPath, pathname, NULL) ;
    file = fopen (pathname, "rb") ;
    if (file == NULL) {
        LGE "(id3Get) Error opening %s.\n", pathname) ;
        return (errno) ;
    }

/* Get the ID3 tag from the file. */

    if (id3GetF (file, version, tag)) {
        LGE "(id3Get) Error getting ID3 tag from %s.\nid3GetF: ", pathname) ;
        PUSH_ERRNO ;  fclose (file) ;  POP_ERRNO ;
        return (errno) ;
    }

    fclose (file) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3GetF ()

    Get the ID3 Tag from an Open File.


Purpose:

    The id3GetF() function retrieves the ID3 tag, if any, from an open file.


    Invocation:

        status = id3GetF (file, version, &tag) ;

    where

        <file>		- I
            is the Unix FILE* handle for the previously opened file.
       <version>	- I
            specifies which ID3 tag (v1 or v2) to get from the file.
        <tag>		- O
            returns a handle for the ID3 tag in the file.  This handle is
            used in calls to the other ID3_UTIL functions; it should be
            id3Destroy()ed when no longer needed.  If the file has no ID3 tag,
            a NULL handle and an error status of success (0) is returned.
        <status>	- O
            returns the status of getting the tag from the file,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3GetF (

#    if PROTOTYPES
        FILE  *file,
        Id3Version  version,
        Id3Tag  *tag)
#    else
        file, version, tag)

        FILE  *file ;
        Id3Version  version,
        Id3Tag  *tag ;
#    endif

{    /* Local variables. */
    char  buffer[ID3_V1_TRAILER_SIZE], header[ID3_V2_HEADER_SIZE] ;
    size_t  i ;




/*******************************************************************************
    Read the tag from the file.
*******************************************************************************/

    if (ID3V_VERSION (version) == 2) {		/* Get ID3v2 tag? */

/* Position to and read the first 10 bytes in the file. */

#if defined(HAVE_FSEEK) && !HAVE_FSEEK
#        warning  id3GetF: No fseek(3).
#else
        if (fseek (file, 0, SEEK_SET)) {
            LGE "(id3GetF) Error positioning to ID3v2 tag.\nfseek: ") ;
            return (errno) ;
        }
#endif

        if (fread (header, ID3_V2_HEADER_SIZE, 1, file) != 1) {
            LGE "(id3GetF) Error reading ID3v2 header.\nfread: ") ;
            return (errno) ;
        }

/* Check that the header read from the file is for a valid ID3 tag.  If not,
   simply return a NULL tag. */

        if ((header[0] != 'I') || (header[1] != 'D') || (header[2] != '3')) {
            LGI "(id3GetF) No ID3v2 tag in file.\n") ;
            *tag = NULL ;
            return (0) ;		/* Successful with NULL handle. */
        }

        version = ID3V (2, header[3], header[4]) ;

    } else {					/* Get ID3v1 tag. */

/* Position to and read the last 128 bytes in the file. */

#if defined(HAVE_FSEEK) && !HAVE_FSEEK
#        warning  id3GetF: No fseek(3).
#else
        if (fseek (file, -ID3_V1_TRAILER_SIZE, SEEK_END)) {
            LGE "(id3GetF) Error positioning to ID3v1 tag.\nfseek: ") ;
            return (errno) ;
        }
#endif

        if (fread (buffer, ID3_V1_TRAILER_SIZE, 1, file) != 1) {
            LGE "(id3GetF) Error reading ID3v1 tag.\nfread: ") ;
            return (errno) ;
        }

/* Check that the trailer read from the file is a valid ID3 tag.  If not,
   simply return a NULL tag. */

        if ((buffer[0] != 'T') || (buffer[1] != 'A') || (buffer[2] != 'G')) {
            LGI "(id3GetF) No ID3v1 tag in file.\n") ;
            *tag = NULL ;
            return (0) ;		/* Successful with NULL handle. */
        }

    }

/*******************************************************************************
    Create the in-memory tag.
*******************************************************************************/

/* Create an empty ID3 tag. */

    if (id3Create (version, tag)) {
        LGE "(id3GetF) Error creating empty ID3 tag.\nid3Create: ") ;
        return (errno) ;
    }

    if (ID3V_VERSION (version) == 2) {
        (*tag)->v2Flags = header[5] & 0x0FF ;
        (*tag)->v2Size = ((header[6] & 0x07FL) << 21) |
                         ((header[7] & 0x07FL) << 14) |
                         ((header[8] & 0x07FL) << 7) |
                         (header[9] & 0x07FL) ;
        LGI "(id3GetF) ID3v2 tag (Version 0x%06lX, Flags 0x%02X, Size %lu)\n",
            (*tag)->version, (*tag)->v2Flags, (unsigned long) (*tag)->v2Size) ;
        return (0) ;
    }

/* Populate the tag with the fields in the trailer. */

    i = 3 ;
    strncpym ((*tag)->song, &buffer[i],
              MAX_SONG_LENGTH, MAX_SONG_LENGTH+1) ;
    (*tag)->flags |= ID3_DEFINED_SONG ;
    i += MAX_SONG_LENGTH ;

    strncpym ((*tag)->artist, &buffer[i],
              MAX_ARTIST_LENGTH, MAX_ARTIST_LENGTH+1) ;
    (*tag)->flags |= ID3_DEFINED_ARTIST ;
    i += MAX_ARTIST_LENGTH ;

    strncpym ((*tag)->album, &buffer[i],
              MAX_ALBUM_LENGTH, MAX_ALBUM_LENGTH+1) ;
    (*tag)->flags |= ID3_DEFINED_ALBUM ;
    i += MAX_ALBUM_LENGTH ;

    strncpym ((*tag)->comment, &buffer[i+4],
              MAX_COMMENT_LENGTH, MAX_COMMENT_LENGTH+1) ;
    (*tag)->flags |= ID3_DEFINED_COMMENT ;

    buffer[i+MAX_YEAR_LENGTH] = '\0' ;
    (*tag)->year = atol (&buffer[i]) ;
    (*tag)->flags |= ID3_DEFINED_YEAR ;
    i += MAX_YEAR_LENGTH + MAX_COMMENT_LENGTH ;

    if (buffer[i++] == 0) {			/* Version 1.1 tag? */
        (*tag)->track = buffer[i++] & 0x00FF ;
        (*tag)->flags |= ID3_DEFINED_TRACK ;
    } else {					/* Version 1.0 tag: no track. */
        i++ ;
    }

    (*tag)->genre = buffer[i++] & 0x00FF ;
    (*tag)->flags |= ID3_DEFINED_GENRE ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3IsEmpty ()

    Check If an ID3 Tag is Empty.


Purpose:

    Function id3IsEmpty() returns true if all the fields in an ID3 tag are
    cleared and false otherwise.


    Invocation:

        isEmpty = id3IsEmpty (tag) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <isEmpty>	- O
            returns true if all the field in the ID3 tag are cleared and
            false otherwise.

*******************************************************************************/


bool  id3IsEmpty (

#    if PROTOTYPES
        const  Id3Tag  tag)
#    else
        tag)

        Id3Tag  tag ;
#    endif

{

    if (tag == NULL)  return (true) ;

    return ((tag->song[0] == '\0') &&
            (tag->artist[0] == '\0') &&
            (tag->album[0] == '\0') &&
            (tag->comment[0] == '\0') &&
            (tag->year <= 0) &&
            (tag->track <= 0) &&
            (tag->genre <= 0)) ;

}

/*!*****************************************************************************

Procedure:

    id3Set ()

    Set the ID3 Tag in a Named File.


Purpose:

    The id3Set() function stores an ID3 tag in a specified file,
    overwriting the previous tag, if any.


    Invocation:

        status = id3Set (pathname, tag) ;

    where

        <pathname>	- I
            is the name of the file to which the ID3 tag will be written.
            Environment variables may be embedded in the file name.
        <tag>		- I
            is the handle for the ID3 tag to be stored in the file.
        <status>	- O
            returns the status of setting the tag in the file,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3Set (

#    if PROTOTYPES
        const  char  *pathname,
        const  Id3Tag  tag)
#    else
        pathname, tag)

        char  *pathname ;
        Id3Tag  tag ;
#    endif

{    /* Local variables. */
    FILE  *file ;



/* Open the file for reading and writing. */

    pathname = fnmBuild (FnmPath, pathname, NULL) ;
    file = fopen (pathname, "rb+") ;
    if (file == NULL) {
        LGE "(id3Set) Error opening %s.\n", pathname) ;
        return (errno) ;
    }

/* Set the ID3 tag in the file. */

    if (id3SetF (file, tag)) {
        LGE "(id3Set) Error setting ID3 tag in %s.\nid3SetF: ", pathname) ;
        PUSH_ERRNO ;  fclose (file) ;  POP_ERRNO ;
        return (errno) ;
    }

    fclose (file) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3SetF ()

    Set the ID3 Tag in an Open File.


Purpose:

    The id3SetF() function stores an ID3 tag in an open file.  If the file
    already has an ID3 tag, the old tag is overwritten with the new tag.
    Otherwise, the new tag is appended to the file.


    Invocation:

        status = id3SetF (file, tag) ;

    where

        <file>		- I
            is the Unix FILE* handle for the previously opened file.
            The file must be opened for reading and writing ("rb+").
        <tag>		- I
            is the handle for the ID3 tag to be stored in the file.
        <status>	- O
            returns the status of setting the tag in the file,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3SetF (

#    if PROTOTYPES
        FILE  *file,
        const  Id3Tag  tag)
#    else
        file, tag)

        FILE  *file ;
        Id3Tag  tag ;
#    endif

{    /* Local variables. */
    char  buffer[ID3_V1_TRAILER_SIZE] ;
    Id3Tag  oldTag ;



/* Determine if there is an existing ID3 tag in the file that needs to be
   overwritten. */

    if (id3GetF (file, tag->version, &oldTag)) {
        LGE "(id3SetF) Error checking for existing ID3 tag.\nid3GetF: ") ;
        return (errno) ;
    }

/* Construct the actual 128-byte ID3 tag. */

    id3Encode (oldTag, true, buffer) ;

    id3Encode (tag, false, buffer) ;

/* Position to the location in the file at which the tag is to be written.
   If the file already has a tag, the new tag will overwrite the old tag.
   Otherwise, the new tag is simply appended to the file. */

#if defined(HAVE_FSEEK) && !HAVE_FSEEK
#    warning  id3SetF: No fseek(3).
#else
    if (fseek (file, (oldTag == NULL) ? 0 : -ID3_V1_TRAILER_SIZE, SEEK_END)) {
        LGE "(id3SetF) Error positioning for ID3 tag.\nfseek: ") ;
        return (errno) ;
    }
#endif

/* Write the tag to the file. */

    if (fwrite (buffer, ID3_V1_TRAILER_SIZE, 1, file) != 1) {
        LGE "(id3SetF) Error writing ID3 tag.\nfwrite: ") ;
        return (errno) ;
    }

    LGI "(id3SetF) Wrote tag %p.\n", (void *) tag) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3Strip ()

    Strip the ID3 Tag from a Named File.


Purpose:

    The id3Strip() function strips the ID3 tag from a specified file.


    Invocation:

        status = id3Strip (pathname, version) ;

    where

        <pathname>	- I
            is the name of the file to which the ID3 tag will be written.
            Environment variables may be embedded in the file name.
        <version>	- I
            specifies which ID3 tag (v1 or v2) to strip from the file.
        <status>	- O
            returns the status of stripping the tag from the file,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3Strip (

#    if PROTOTYPES
        const  char  *pathname,
        Id3Version  version)
#    else
        pathname, version)

        char  *pathname ;
        Id3Version  version ;
#    endif

{    /* Local variables. */
    FILE  *file ;



/* Open the file for reading and writing. */

    pathname = fnmBuild (FnmPath, pathname, NULL) ;
    file = fopen (pathname, "rb+") ;
    if (file == NULL) {
        LGE "(id3Strip) Error opening %s.\n", pathname) ;
        return (errno) ;
    }

/* Strip the ID3 tag from the file. */

    if (id3StripF (file, version)) {
        LGE "(id3Strip) Error stripping ID3 tag from %s.\nid3StripF: ", pathname) ;
        PUSH_ERRNO ;  fclose (file) ;  POP_ERRNO ;
        return (errno) ;
    }

    fclose (file) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3StripF ()

    Strip the ID3 Tag from an Open File.


Purpose:

    The id3StripF() function stores strips the ID3 tag from an open file.


    Invocation:

        status = id3StripF (file, version) ;

    where

        <file>		- I
            is the Unix FILE* handle for the previously opened file.
            The file must be opened for reading and writing ("rb+").
        <version>	- I
            specifies which ID3 tag (v1 or v2) to strip from the file.
        <status>	- O
            returns the status of stripping the tag from the file,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  id3StripF (

#    if PROTOTYPES
        FILE  *file,
        Id3Version  version)
#    else
        file, version)

        FILE  *file ;
        Id3Version  version ;
#    endif

{    /* Local variables. */
#if defined(HAVE_FTRUNCATE) && !HAVE_FTRUNCATE
#    warning  id3StripF: No ftruncate(3).
#else
#if defined(HAVE_FSEEK) && !HAVE_FSEEK
#    warning  id3StripF: No fseek(3).
#else
    Id3Tag  tag ;
    long  truncatedLength ;
    size_t  tagLength ;



/* Determine if the file has an ID3 tag. */

    if (id3GetF (file, version, &tag)) {
        LGE "(id3StripF) Error checking for ID3 tag.\nid3GetF: ") ;
        return (errno) ;
    } else if (tag == NULL) {
        LGI "(id3StripF) No ID3v%ld tag.\n", ID3V_VERSION (version)) ;
        return (0) ;
    }

    tagLength = (ID3V_VERSION (version) == 2)
                ? (ID3_V2_HEADER_SIZE + tag->v2Size)
                : ID3_V1_TRAILER_SIZE ;


/*******************************************************************************
    ID3v2 tags are located at the beginning of the file.  To remove the tag,
    the subsequent data in the file must be moved down to the beginning of
    the file.
*******************************************************************************/

    if (ID3V_VERSION (version) == 2) {

#define  ID3_BLOCK_SIZE  (16*1024)
        char  buffer[ID3_BLOCK_SIZE] ;
        size_t  numBytesRead ;

/* Position to the beginning of the ID3v2 tag in the file. */

        if (fseek (file, 0, SEEK_SET)) {
            LGE "(id3StripF) Error positioning to ID3v2 tag.\nfseek: ") ;
            return (errno) ;
        }

/* Move the data in the file down to the beginning of the file. */

        for ( ; ; ) {

            if (fseek (file, tagLength, SEEK_CUR))  break ;

            numBytesRead = fread (buffer, 1, sizeof buffer, file) ;
            if (numBytesRead < 1)  break ;

            LGI "(id3StripF) Moving %lu bytes from %ld to %ld.\n",
                (unsigned long) numBytesRead,
                ftell (file) - numBytesRead,
                ftell (file) - numBytesRead - tagLength) ;

            if (fseek (file, -((long) numBytesRead) - tagLength,
                SEEK_CUR))  break ;

            if (fwrite (buffer, numBytesRead, 1, file) != 1) {
                LGE "(id3StripF) Error writing %lu bytes.\nfwrite: ",
                    (unsigned long) numBytesRead) ;
                return (errno) ;
            }

        }

    }


/*******************************************************************************
    Truncate the empty space at the end of the file (if the data was moved down
    overwriting the ID3v2 tag) or the 128-byte ID3v1 tag (if that tag is being
    stripped).
*******************************************************************************/

    if (fseek (file, 0, SEEK_END)) {
        LGE "(id3StripF) Error positioning to end of file.\nfseek: ") ;
        return (errno) ;
    }

    truncatedLength = ftell (file) - tagLength ;

    if (ftruncate (fileno (file), truncatedLength)) {
        LGE "(id3StripF) Error truncating file to %ld bytes.\nftruncate: ",
            (long) truncatedLength) ;
        return (errno) ;
    }

    LGI "(id3StripF) Stripped ID3v%ld tag.\n", ID3V_VERSION (version)) ;

#endif
#endif


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    id3Version ()

    Get an ID3 Tag's Version.


Purpose:

    Function id3Version() returns the version (including the major and revision
    numbers) of an ID3 tag.


    Invocation:

        version = id3Version (tag) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <version>	- O
            returns ID3 tag's version; see the Id3Version type definition
            in "id3_util.h".

*******************************************************************************/


Id3Version  id3Version (

#    if PROTOTYPES
        Id3Tag  tag)
#    else
        tag)

        Id3Tag  tag ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Version) NULL tag handle: ") ;
        return (0L) ;
    }

    return (tag->version) ;

}

/*!*****************************************************************************

Procedure:

    id3Album ()

    Get/Set an ID3 Tag's Album Field.


Purpose:

    Function id3Album() gets/sets an ID3 tag's album field.


    Invocation (get field):

        album = id3Album (tag, NULL) ;

    Invocation (set field):

        id3Album (tag, album) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <album>		- O
            returns the album field from the ID3 tag.
        <album>		- I
            specifies the album to be set in the ID3 tag.  The name stored
            in the tag is truncated to 30 characters.  To clear the field,
            specify a zero-length string (""); NULL indicates the get-field
            operation.

*******************************************************************************/


const  char  *id3Album (

#    if PROTOTYPES
        Id3Tag  tag,
        const  char  *album)
#    else
        tag, album)

        Id3Tag  tag ;
        char  *album ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Album) NULL tag handle: ") ;
        return (NULL) ;
    }

    if (album != NULL) {
        strlcpy (tag->album, album, sizeof tag->album) ;
        tag->flags |= ID3_DEFINED_ALBUM ;
    }

    return (tag->album) ;

}

/*!*****************************************************************************

Procedure:

    id3Artist ()

    Get/Set an ID3 Tag's Artist Field.


Purpose:

    Function id3Artist() gets/sets an ID3 tag's artist field.


    Invocation (get field):

        artist = id3Artist (tag, NULL) ;

    Invocation (set field):

        id3Artist (tag, artist) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <artist>		- O
            returns the artist field from the ID3 tag.
        <artist>		- I
            specifies the artist to be set in the ID3 tag.  The name stored
            in the tag is truncated to 30 characters.  To clear the field,
            specify a zero-length string (""); NULL indicates the get-field
            operation.

*******************************************************************************/


const  char  *id3Artist (

#    if PROTOTYPES
        Id3Tag  tag,
        const  char  *artist)
#    else
        tag, artist)

        Id3Tag  tag ;
        char  *artist ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Artist) NULL tag handle: ") ;
        return (NULL) ;
    }

    if (artist != NULL) {
        strlcpy (tag->artist, artist, sizeof tag->artist) ;
        tag->flags |= ID3_DEFINED_ARTIST ;
    }

    return (tag->artist) ;

}

/*!*****************************************************************************

Procedure:

    id3Comment ()

    Get/Set an ID3 Tag's Comment Field.


Purpose:

    Function id3Comment() gets/sets an ID3 tag's comment field.


    Invocation (get field):

        comment = id3Comment (tag, NULL) ;

    Invocation (set field):

        id3Comment (tag, comment) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <comment>	- O
            returns the comment field from the ID3 tag.
        <comment>	- I
            specifies the comment to be set in the ID3 tag.  The name stored
            in the tag is truncated to 28 characters.  To clear the field,
            specify a zero-length string (""); NULL indicates the get-field
            operation.

*******************************************************************************/


const  char  *id3Comment (

#    if PROTOTYPES
        Id3Tag  tag,
        const  char  *comment)
#    else
        tag, comment)

        Id3Tag  tag ;
        char  *comment ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Comment) NULL tag handle: ") ;
        return (NULL) ;
    }

    if (comment != NULL) {
        strlcpy (tag->comment, comment, sizeof tag->comment) ;
        tag->flags |= ID3_DEFINED_COMMENT ;
    }

    return (tag->comment) ;

}

/*!*****************************************************************************

Procedure:

    id3Genre ()

    Get/Set an ID3 Tag's Genre Field.


Purpose:

    Function id3Genre() gets/sets an ID3 tag's genre field.


    Invocation (get field):

        genre = id3Genre (tag, -1) ;

    Invocation (set field):

        id3Genre (tag, genre) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <genre>		- O
            returns the genre field from the ID3 tag.
        <genre>		- I
            specifies the genre to be set in the ID3 tag.  To clear the field,
            specify a genre number of zero (yer blues!); -1 indicates the
            get-field operation.

*******************************************************************************/


ssize_t  id3Genre (

#    if PROTOTYPES
        Id3Tag  tag,
        ssize_t  genre)
#    else
        tag, genre)

        Id3Tag  tag ;
        ssize_t  genre ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Genre) NULL tag handle: ") ;
        return (-1) ;
    }

    if (genre >= 0) {
        tag->genre = genre ;
        tag->flags |= ID3_DEFINED_GENRE ;
    }

    return (tag->genre) ;

}

/*!*****************************************************************************

Procedure:

    id3Song ()

    Get/Set an ID3 Tag's Song Field.


Purpose:

    Function id3Song() gets/sets an ID3 tag's song field.


    Invocation (get field):

        song = id3Song (tag, NULL) ;

    Invocation (set field):

        id3Song (tag, song) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <song>		- O
            returns the song field from the ID3 tag.
        <song>		- I
            specifies the song to be set in the ID3 tag.  The name stored
            in the tag is truncated to 30 characters.  To clear the field,
            specify a zero-length string (""); NULL indicates the get-field
            operation.

*******************************************************************************/


const  char  *id3Song (

#    if PROTOTYPES
        Id3Tag  tag,
        const  char  *song)
#    else
        tag, song)

        Id3Tag  tag ;
        char  *song ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Song) NULL tag handle: ") ;
        return (NULL) ;
    }

    if (song != NULL) {
        strlcpy (tag->song, song, sizeof tag->song) ;
        tag->flags |= ID3_DEFINED_SONG ;
    }

    return (tag->song) ;

}

/*!*****************************************************************************

Procedure:

    id3Track ()

    Get/Set an ID3 Tag's Track Field.


Purpose:

    Function id3Track() gets/sets an ID3 tag's track field.


    Invocation (get field):

        track = id3Track (tag, -1) ;

    Invocation (set field):

        id3Track (tag, track) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <track>		- O
            returns the track field from the ID3 tag.
        <track>		- I
            specifies the track to be set in the ID3 tag.  To clear the field,
            specify a track number of zero (0); -1 indicates the get-field
            operation.

*******************************************************************************/


ssize_t  id3Track (

#    if PROTOTYPES
        Id3Tag  tag,
        ssize_t  track)
#    else
        tag, track)

        Id3Tag  tag ;
        ssize_t  track ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Track) NULL tag handle: ") ;
        return (-1) ;
    }

    if (track >= 0) {
        tag->track = track ;
        tag->flags |= ID3_DEFINED_TRACK ;
    }

    return (tag->track) ;

}

/*!*****************************************************************************

Procedure:

    id3Year ()

    Get/Set an ID3 Tag's Year Field.


Purpose:

    Function id3Year() gets/sets an ID3 tag's year field.


    Invocation (get field):

        year = id3Year (tag, -1) ;

    Invocation (set field):

        id3Year (tag, year) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <year>		- O
            returns the year field from the ID3 tag.
        <year>		- I
            specifies the year to be set in the ID3 tag.  To clear the field,
            specify a year number of zero (0); -1 indicates the get-field
            operation.

*******************************************************************************/


long  id3Year (

#    if PROTOTYPES
        Id3Tag  tag,
        long  year)
#    else
        tag, year)

        Id3Tag  tag ;
        long  year ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Year) NULL tag handle: ") ;
        return (-1) ;
    }

    if (year >= 0) {
        tag->year = year ;
        tag->flags |= ID3_DEFINED_YEAR ;
    }

    return (tag->year) ;

}

/*!*****************************************************************************

Procedure:

    id3Flags ()

    Get an ID3v2 Tag's Header Flags.


Purpose:

    Function id3Flags() returns the ID3v2 flags found in an ID3v2 tag's header.


    Invocation:

        flags = id3Flags (tag) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <flags>		- O
            returns the ID3v2 header flags from the ID3 tag; zero is returned
            if the ID3 tag is not version 2.

*******************************************************************************/


int  id3Flags (

#    if PROTOTYPES
        Id3Tag  tag)
#    else
        tag)

        Id3Tag  tag ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Flags) NULL tag handle: ") ;
        return (0) ;
    }

    return (tag->v2Flags) ;

}

/*!*****************************************************************************

Procedure:

    id3Size ()

    Get an ID3 Tag's Size.


Purpose:

    Function id3Size() returns the size, in bytes, of an ID3 tag.
    For ID3v1 tags, the size is always 128 bytes.  For ID3v2 tags,
    the size excludes the 10-byte ID3v2 header.


    Invocation:

        size = id3Size (tag) ;

    where

        <tag>		- I
            is the tag handle returned by id3Create().
        <size>		- O
            returns the size in bytes of the ID3 tag.  NOTE that for ID3v2 tags,
            the size *excludes* the 10-byte header.

*******************************************************************************/


ssize_t  id3Size (

#    if PROTOTYPES
        Id3Tag  tag)
#    else
        tag)

        Id3Tag  tag ;
#    endif

{

    if (tag == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(id3Size) NULL tag handle: ") ;
        return (0) ;
    }

    return ((ID3V_VERSION (tag->version) == 2) ? tag->v2Size
                                               : ID3_V1_TRAILER_SIZE) ;

}

/*!*****************************************************************************

Procedure:

    id3FromGenre ()

    Translate a Genre Number to its Name.


Purpose:

    Function id3FromGenre() looks up a genre number and returns its
    corresponding name.


    Invocation:

        name = id3FromGenre (number) ;

    where

        <number>	- I
            is the number (0..125) for the genre.
        <name>		- O
            returns the genre name; NULL is returned for an invalid number.

*******************************************************************************/


const  char  *id3FromGenre (

#    if PROTOTYPES
        ssize_t  number)
#    else
        number)

        ssize_t  number ;
#    endif

{    /* Local variables. */
    size_t  i ;



    for (i = 0 ;  genreLUT[i].name != NULL ;  i++) {
        if (genreLUT[i].number == number)  break ;
    }

    return (genreLUT[i].name) ;

}

/*!*****************************************************************************

Procedure:

    id3ToGenre ()

    Translate a Genre Name to its Number.


Purpose:

    Function id3ToGenre() looks up a genre name and returns its corresponding
    number.


    Invocation:

        number = id3ToGenre (name) ;

    where

        <name>		- I
            is the name of the genre.
        <number>	- O
            returns the genre number (0..125) or -1 for an invalid name.

*******************************************************************************/


ssize_t  id3ToGenre (

#    if PROTOTYPES
        const  char  *name)
#    else
        name)

        char  *name ;
#    endif

{    /* Local variables. */
    size_t  i, length ;



    if (name == NULL)  return (-1) ;

    length = strlen (name) ;

    for (i = 0 ;  genreLUT[i].name != NULL ;  i++) {
        if (length > strlen (genreLUT[i].name))  continue ;
        if (strncasecmp (genreLUT[i].name, name, length) == 0)  break ;
    }

    return (genreLUT[i].number) ;

}
