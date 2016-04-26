/* $Id: tag311.c,v 1.5 2015/08/15 03:35:10 alex Exp alex $ */
/*******************************************************************************

Program:

    tag311.c

    Get/Set ID3 Version 1.1 Tags.


Author:    Alex Measday


Purpose:

    TAG311 lists, creates, and modifies the ID3v1 tags commonly found in MP3
    files.  For many years, I used ID3EDIT (http://id3edit.sourceforge.net/),
    ID3CONVERT (an example program found in ID3LIB), and some shell scripts
    to maintain MP3 tags under Linux.  However, I also wanted a simple
    command-line program that combined these capabilities and that would run
    under Windows and Linux.  Hence, TAG311.  The single-letter abbreviations
    for the field-setting options and the long listing format were borrowed
    from ID3EDIT.  The "-strip 2" option replaced the need for ID3CONVERT
    and the "-set <parts>" option eliminated the need for the shell scripts.

    Without any options, TAG311 lists the ID3 tags in one or more MP3 files.
    The default listing format for ID3v1 tags is "-brief"; the "-long" option
    produces an ID3EDIT-compatible listing.  Normally, TAG311 only notes the
    presence of an ID3v2 tag; the "-dump" option will generate an ASCII dump
    of the tag.

    To add new or modify existing ID3v1 tags. specify one or more of the field
    options ("-song", "-artist", "-album", "-comment", "-year", or "-genre")
    before the list of MP3 files.  In the case of an existing tag, only the
    specified fields are updated in the tag.

    The genre field can be specified by numerical value or name.  The "-plus"
    option will generate a full list of the defined genre numbers and names.
    The "-match <text>" option only lists the genres whose names contain
    <text>.

    TAG311 can generate certain ID3v1 fields from MP3 file names that consist
    of the track number, the artist, and the song title, separated by hyphens.
    The "-set <parts>" option specifies which fields are in the file name(s)
    and in which order they appear.  For example, "-set tsa" will extract and
    set the track number, song title, and artist from "07 - Help - Beatles.mp3".
    The "-copy <file>" option loads the ID3v1 tag from a file.  The
    field-setting options can then be applied to the tag and the tag stored
    in one or more other files.

    To strip the ID3v2 tag from an MP3 file, specify "-strip 2" before the
    list of files.  The original files are modified, so interrupting TAG311
    in the middle of stripping a tag may corrupt the MP3 file.

    The "-trim" option trims trailing blanks from text fields in ID3v1 tags.
    Yes, I'm obsessive!


    Invocation:

        % tag311 [-help]
                 [-brief] [-dump] [-long]
                 [-match <text>] [-plus]
                 [-copy <file>]
                 [-field <separator>] [-set <parts>]
                 [-clean] [-strip <version>] [-trim]
                 [-song <title>]
                 [-artist <artist>]
                 [-b <album>] [-album <album>]
                 [-comment <text>]
                 [-year <year>]
                 [-track <number>]
                 [-genre <genre>]
                 <file(s)>

    where

        "-help"
            displays help information.
        "-brief"
        "-long"
            specifies the output format (brief, long) when listing tags;
            the default is brief.
        "-dump"
            generates a hexadecimal/ASCII dump of a file's ID3v2 tag.
        "-match <text>"
            writes to standard output a list of the genres that contain
            the specified text.  (Basically the equivalent of piping the
            output of "-plus" through "grep <text>".)
        "-plus"
            writes to standard output a list of all of the defined genres.
        "-copy <file>"
            loads the fields from a file's ID3v1 tag for the purpose of storing
            them in files that follow on the command line.  Individual fields
            can be overridden using the various field options prior to
            specifying the destination files.
        "-field <separator>"
            specifies the field separator for the "-set <parts>" option below.
            The default separator is " - ".
        "-set <parts>"
            causes TAG311 to set certain fields in a file's ID3v1 tag using
            information embedded in the file's name.  The argument is a string
            containing the ordered single-letter abbreviations for the fields
            in the file's name: "t" for the track number, "s" for the song
            title, "a" for the artist, "c" for a comment, and "y" for the
            year.  For example, specify "-set ts" if the file names are of
            the form: "<track> - <song>.mp3".  The default field separator
            is " - ", although a different divider can be specified with the
            "-field <separator>" option.
        "-clean"
            is shorthand for "-strip 2 -trim".
        "-strip <version>"
            strips the specified version (1 or 2) of ID3 tag from the files.
        "-trim"
            trims trailing blanks from the text fields in ID3v1 tags.
        "-song <title>"
        "-artist <artist>"
        "-b <album>"
        "-album <album>"
        "-comment <text>"
        "-year <year>"
        "-track <number>"
        "-genre <genre>"
            are used to set fields in the ID3v1 tags of the files that follow
            on the command line.
        "<file(s)>"
            specifies the file(s) to be accessed.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  <sys/types.h>			/* System type definitions. */
#include  <sys/stat.h>			/* File status definitions. */
#include  "drs_util.h"			/* Directory scanning utilities. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "get_util.h"			/* "Get Next" functions. */
#include  "id3_util.h"			/* ID3 utilities. */
#include  "meo_util.h"			/* Memory operations. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */


/*******************************************************************************
    Private Functions
*******************************************************************************/

static  errno_t  listTags (
#    if PROTOTYPES
        const  char  *fileSpec,
        bool  brief,
        bool  dump
#    endif
    ) ;

static  errno_t  storeTag (
#    if PROTOTYPES
        Id3Tag  tag,
        const  char  *setParts,
        const  char  *separator,
        const  char  *fileSpec
#    endif
    ) ;

static  errno_t  stripTags (
#    if PROTOTYPES
        Id3Version  version,
        const  char  *fileSpec
#    endif
    ) ;

static  errno_t  trimFields (
#    if PROTOTYPES
        const  char  *fileSpec
#    endif
    ) ;

/*******************************************************************************
    Main Program.
*******************************************************************************/

int  main (

#    if PROTOTYPES
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{    /* Local variables. */
    bool  brief, dump, help, trim ;
    char  *argument, *s, *separator, *setParts ;
    Id3Tag  tag ;
    Id3Version  stripVersion, version ;
    int  errflg, option ;
    OptContext  context ;
    ssize_t  genre ;

    const  char  *optionList[] = {	/* Command line options. */
        "{help}",
        "{artist:}",
        "b:", "{album:}",
        "{comment:}",
        "{genre:}",
        "{song:}",
        "{track:}",
        "{year:}",
        "{brief}",
        "{clean}",
        "{copy:}",
        "{dump}",
        "{field:}",
        "{long}",
        "{match:}",
        "{plus}",
        "{set:}",
        "{strip:}",
        "{trim}",
        NULL
    } ;




    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    help = false ;
    brief = true ;  dump = false ;
    stripVersion = 0 ;  version = ID3V_V1_1 ;
    separator = NULL ;  setParts = NULL ;
    tag = NULL ;  trim = false ;

    opt_init (argc, argv, NULL, optionList, &context) ;
    errflg = 0 ;

    while ((option = opt_get (context, &argument))) {

        switch (option) {
        case 1:			/* "-help" */
            help = true ;
            break ;
        case 2:			/* "-artist <artist>" */
            if ((tag == NULL) && id3Create (version, &tag))
                errflg++ ;
            else
                id3Artist (tag, argument) ;
            break ;
        case 3:			/* "-b <album>" */
        case 4:			/* "-album <album>" */
            if ((tag == NULL) && id3Create (version, &tag))
                errflg++ ;
            else
                id3Album (tag, argument) ;
            break ;
        case 5:			/* "-comment <text>" */
            if ((tag == NULL) && id3Create (version, &tag))
                errflg++ ;
            else
                id3Comment (tag, argument) ;
            break ;
        case 6:			/* "-genre <genre>" */
            if ((tag == NULL) && id3Create (version, &tag)) {
                errflg++ ;
            } else {
                genre = (ssize_t) strtol (argument, &s, 0) ;
                if (*s != '\0') {
                    genre = id3ToGenre (argument) ;
                    if (genre < 0)  errflg++ ;
                }
                id3Genre (tag, genre) ;
            }
            break ;
        case 7:			/* "-song <title>" */
            if ((tag == NULL) && id3Create (version, &tag))
                errflg++ ;
            else
                id3Song (tag, argument) ;
            break ;
        case 8:			/* "-track <number>" */
            if ((tag == NULL) && id3Create (version, &tag))
                errflg++ ;
            else
                id3Track (tag, atoi (argument)) ;
            break ;
        case 9:			/* "-year <year>" */
            if ((tag == NULL) && id3Create (version, &tag))
                errflg++ ;
            else
                id3Year (tag, atol (argument)) ;
            break ;
        case 10:		/* "-brief" */
            brief = true ;
            break ;
        case 11:		/* "-clean" */
            stripVersion = ID3V (2, 255, 255) ;
            trim = true ;
            break ;
        case 12:		/* "-copy <file>" */
            if (tag != NULL) {
                id3Destroy (tag) ;
                tag = NULL ;
            }
            if (id3Get (argument, ID3V_V1, &tag))  errflg++ ;
            break ;
        case 13:		/* "-dump" */
            dump = true ;
            break ;
        case 14:		/* "-field <separator>" */
            separator = argument ;
            break ;
        case 15:		/* "-long" */
            brief = false ;
            break ;
        case 16:		/* "-match <text>" */
            for (genre = 0 ;  ;  genre++) {
                s = (char *) id3FromGenre (genre) ;
                if (s == NULL)  exit (0) ;
                if (strstr (s, argument) != NULL) {
                    printf ("%3d: %s\n", (int) genre, s) ;
                }
            }
            break ;
        case 17:		/* "-plus" */
            for (genre = 0 ;  ;  genre++) {
                s = (char *) id3FromGenre (genre) ;
                if (s == NULL) {
                    printf ("%3d: %s\n", 255, id3FromGenre (255)) ;
                    exit (0) ;
                }
                printf ("%3d: %s\n", (int) genre, s) ;
            }
            break ;
        case 18:		/* "-set <parts>" */
            if ((tag == NULL) && id3Create (version, &tag))
                errflg++ ;
            else
                setParts = argument ;
            break ;
        case 19:		/* "-strip <version>" */
            stripVersion = ID3V (atoi (argument), 255, 255) ;
            break ;
        case 20:		/* "-trim" */
            trim = true ;
            break ;
        case NONOPT:		/* "<file(s)>" */
            if (stripVersion != 0) {
                stripTags (stripVersion, argument) ;
            }
            if (tag != NULL) {
                storeTag (tag, setParts, separator, argument) ;
            }
            if (trim) {
                trimFields (argument) ;
            }
            if (listTags (argument, brief, dump))  errflg++ ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (context) ;

    if (errflg || help) {
        fprintf (stderr, "Usage:  tag311 [-help]\n") ;
        fprintf (stderr, "               [-brief] [-dump] [-long]\n") ;
        fprintf (stderr, "               [-match <text>] [-plus]\n") ;
        fprintf (stderr, "               [-copy <file>]\n") ;
        fprintf (stderr, "               [-field <separator>] [-set <parts>]\n") ;
        fprintf (stderr, "               [-clean] [-strip <version>] [-trim]\n") ;
        fprintf (stderr, "               [-song <title>]\n") ;
        fprintf (stderr, "               [-artist <artist>]\n") ;
        fprintf (stderr, "               [-b <album>] [-album <album>]\n") ;
        fprintf (stderr, "               [-comment <text>]\n") ;
        fprintf (stderr, "               [-year <year>]\n") ;
        fprintf (stderr, "               [-track <number>]\n") ;
        fprintf (stderr, "               [-genre <genre>]\n") ;
        fprintf (stderr, "               <file(s)>\n") ;
        if (!help) {
            fprintf (stderr, "Type \"tag311 -help\" for more detailed information.\n") ;
            exit (EINVAL) ;
        }
        fprintf (stderr, "\n") ;
        fprintf (stderr, "OVERVIEW:\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    TAG311 lists, creates, and modifies the ID3v1 tags commonly found in MP3\n") ;
        fprintf (stderr, "    files.  For many years, I used ID3EDIT (http://id3edit.sourceforge.net/),\n") ;
        fprintf (stderr, "    ID3CONVERT (an example program found in ID3LIB), and some shell scripts\n") ;
        fprintf (stderr, "    to maintain MP3 tags under Linux.  However, I also wanted a simple\n") ;
        fprintf (stderr, "    command-line program that combined these capabilities and that would run\n") ;
        fprintf (stderr, "    under Windows and Linux.  Hence, TAG311.  The single-letter abbreviations\n") ;
        fprintf (stderr, "    for the field-setting options and the long listing format were borrowed\n") ;
        fprintf (stderr, "    from ID3EDIT.  The \"-strip 2\" option replaced the need for ID3CONVERT\n") ;
        fprintf (stderr, "    and the \"-set <parts>\" option eliminated the need for the shell scripts.\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "LISTING TAGS:\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    Without any options, TAG311 lists the ID3 tags in one or more MP3 files.\n") ;
        fprintf (stderr, "    The default listing format for ID3v1 tags is \"-brief\"; the \"-long\" option\n") ;
        fprintf (stderr, "    produces an ID3EDIT-compatible listing.  Normally, TAG311 only notes the\n") ;
        fprintf (stderr, "    presence of an ID3v2 tag; the \"-dump\" option will generate an ASCII dump\n") ;
        fprintf (stderr, "    of the tag.\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "EDITING TAGS:\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    To add new or modify existing ID3v1 tags. specify one or more of the field\n") ;
        fprintf (stderr, "    options (\"-song\", \"-artist\", \"-album\", \"-comment\", \"-year\", or \"-genre\")\n") ;
        fprintf (stderr, "    before the list of MP3 files.  In the case of an existing tag, only the\n") ;
        fprintf (stderr, "    specified fields are updated in the tag.\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    The genre field can be specified by numerical value or name.  The \"-plus\"\n") ;
        fprintf (stderr, "    option will generate a full list of the defined genre numbers and names.\n") ;
        fprintf (stderr, "    The \"-match <text>\" option only lists the genres whose names contain\n") ;
        fprintf (stderr, "    <text>.\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    TAG311 can generate certain ID3v1 fields from MP3 file names that consist\n") ;
        fprintf (stderr, "    of the track number, the artist, and the song title, separated by hyphens.\n") ;
        fprintf (stderr, "    The \"-set <parts>\" option specifies which fields are in the file name(s)\n") ;
        fprintf (stderr, "    and in which order they appear.  For example, \"-set tsa\" will extract and\n") ;
        fprintf (stderr, "    set the track number, song title, and artist from \"07 - Help - Beatles.mp3\".\n") ;
        fprintf (stderr, "    The default field delimiter is \" - \", although a different one can be\n") ;
        fprintf (stderr, "    specified with the \"-field <separator>\" option.\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    The \"-copy <file>\" option loads the ID3v1 tag from a file.  The\n") ;
        fprintf (stderr, "    field-setting options can then be applied to the tag and the tag stored\n") ;
        fprintf (stderr, "    in one or more other files.\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "MISCELLANEOUS:\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    To strip the ID3v2 tag from an MP3 file, specify \"-strip 2\" before the\n") ;
        fprintf (stderr, "    list of files.  The original files are modified, so interrupting TAG311\n") ;
        fprintf (stderr, "    in the middle of stripping a tag may corrupt the MP3 file.\n") ;
        fprintf (stderr, "\n") ;
        fprintf (stderr, "    The \"-trim\" option trims trailing blanks from text fields in ID3v1 tags.\n") ;
        fprintf (stderr, "    Yes, I'm obsessive!\n") ;
        exit (0) ;
    }


    exit (0) ;

}

/*!*****************************************************************************

Procedure:

    listTags ()


Purpose:

    Function listTags() lists the ID3 tags in the specified set of files.


    Invocation:

        status = listTags (fileSpec, brief, dump) ;

    where

        <fileSpec>	- I
            is the (possibly wildcard) specification of the file(s).
        <brief>		- I
            controls the output format, brief or long.
        <status>	- O
            returns the status of listing the tags, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  listTags (

#    if PROTOTYPES
        const  char  *fileSpec,
        bool  brief,
        bool  dump)
#    else
        fileSpec, brief, dump)

        char  *fileSpec ;
        bool  brief ;
        bool  dump ;
#    endif

{    /* Local variables. */
    char  *fileName, *genre ;
    DirectoryScan  scan ;
    Id3Tag  tag ;



/*******************************************************************************
    Scan the specified files and dump any ID3 tags that are found.
*******************************************************************************/

    if (drsCreate (fileSpec, &scan))  return (errno) ;

    fileName = (char *) drsFirst (scan) ;

    while (fileName != NULL) {

        if (brief)
            printf ("\n") ;
        else
            printf ("\nFile:\t\t%s\n", fnmBuild (FnmFile, fileName, NULL)) ;

        if (id3Get (fileName, ID3V_V1, &tag)) {
            fileName = (char *) drsNext (scan) ;
            continue ;
        }

        if (tag == NULL) {
            printf ("ID3v1:\tno tag\t%s\n",
                    fnmBuild (FnmFile, fileName, NULL)) ;
        } else if (brief) {
            printf ("%2ld\t%-30s\t(%s)\n",
                    (long) id3Track (tag, -1),
                    id3Song (tag, NULL),
                    id3Comment (tag, NULL)) ;
            genre = (char *) id3FromGenre (id3Genre (tag, -1)) ;
            printf ("%-30s\t%s\t%ld\t(%s)\n",
                    id3Artist (tag, NULL),
                    id3Album (tag, NULL),
                    id3Year (tag, -1),
                    (genre == NULL) ? "Unknown" : genre) ;
            id3Destroy (tag) ;
        } else {
            printf ("Song:\t\t%s\n", id3Song (tag, NULL)) ;
            printf ("Artist:\t\t%s\n", id3Artist (tag, NULL)) ;
            printf ("Album:\t\t%s\n", id3Album (tag, NULL)) ;
            printf ("Year:\t\t%ld\n", id3Year (tag, -1)) ;
            printf ("Comment:\t%s\n", id3Comment (tag, NULL)) ;
            printf ("Track:\t\t%ld\n", (long) id3Track (tag, -1)) ;
            genre = (char *) id3FromGenre (id3Genre (tag, -1)) ;
            printf ("Genre:\t\t%s\n", (genre == NULL) ? "Unknown" : genre) ;
            id3Destroy (tag) ;
        }

        if (!id3Get (fileName, ID3V_V2, &tag) && (tag != NULL)) {
            printf ("ID3v2:\t(v%ld.%ld, flags 0x%02X, size %ld)\n",
                    ID3V_VERSION (id3Version (tag)),
                    ID3V_MAJOR (id3Version (tag)),
                    id3Flags (tag),
                    (long) id3Size (tag)) ;
            if (dump) {
                long  numBytes ;
                void  *startAddress ;
                numBytes = ID3_V2_HEADER_SIZE + id3Size (tag) ;
                startAddress = (void *) malloc (numBytes) ;
                if (!meoLoad (fileName, 0, &startAddress, &numBytes)) {
                    meoDump (NULL, "    ", MeoHexadecimal, 16, 0,
                             startAddress, numBytes) ;
                }
                free (startAddress) ;
            }
            id3Destroy (tag) ;
        }

        fileName = (char *) drsNext (scan) ;

    }

    drsDestroy (scan) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    storeTag ()


Purpose:

    Function storeTag() sets the ID3 tag in the specified set of files.
    If a file already has an ID3 tag, the defined fields in the new tag
    overwrite the corresponding fields in the old tag.  For example, if
    the new tag only has the album field defined, then storeTag() will
    only update the album field in each file's ID3 tag.


    Invocation:

        status = storeTag (tag, setParts, separator, fileSpec) ;

    where

        <tag>		- I
            is the ID3 tag to be stored in the files.
        <setParts>	- I
            specifies which ID3 tag elements are to be set based on parts of
            the file name: "t" for the track, "s" for the song title, and/or
            "a" for the artist.
        <separator>	- I
            specifies a string that separates the fields in a file name.
            If this argument is NULL, " - " is used.
        <fileSpec>	- I
            is the (possibly wildcard) specification of the file(s).
        <status>	- O
            returns the status of listing the tags, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  storeTag (

#    if PROTOTYPES
        Id3Tag  tag,
        const  char  *setParts,
        const  char  *separator,
        const  char  *fileSpec)
#    else
        tag, setParts, fileSpec)

        Id3Tag  tag ;
        char  *setParts ;
        char  *separator ;
        char  *fileSpec ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;




/*******************************************************************************
    Scan the specified files and store/update their ID3 tags.
*******************************************************************************/

    if (drsCreate (fileSpec, &scan))  return (errno) ;

    fileName = (char *) drsFirst (scan) ;

    while (fileName != NULL) {

        if (setParts != NULL) {		/* Get tag elements from file name? */

            const  char  *field = fnmBuild (FnmName, fileName, NULL) ;
            char  *s ;
            size_t  i, length = 0 ;

#define  SEPARATOR  " - "
            if (separator == NULL)  separator = SEPARATOR ;

            for (i = 0 ;  i < strlen (setParts) ;  i++) {

                field = getfield_s (field, separator, &length) ;
                if (length == 0)  continue ;

                switch (setParts[i]) {
                case 'a':
                case 'A':
                    s = strndup (field, length) ;
                    id3Artist (tag, s) ;
                    free (s) ;
                    break ;
                case 'b':
                case 'B':
                    s = strndup (field, length) ;
                    id3Album (tag, s) ;
                    free (s) ;
                    break ;
                case 'c':
                case 'C':
                    s = strndup (field, length) ;
                    id3Comment (tag, s) ;
                    free (s) ;
                    break ;
                case 's':
                case 'S':
                    s = strndup (field, length) ;
                    id3Song (tag, s) ;
                    free (s) ;
                    break ;
                case 't':
                case 'T':
                    id3Track (tag, atoi (field)) ;
                    break ;
                case 'y':
                case 'Y':
                    id3Year (tag, atol (field)) ;
                    break ;
                default:
                    break;
                }

            }

        }

        id3Set (fileName, tag) ;

        fileName = (char *) drsNext (scan) ;

    }

    drsDestroy (scan) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    stripTags ()


Purpose:

    Function stripTags() strips the ID3 tags (of a specified ID3 version)
    from one or more files.


    Invocation:

        status = stripTags (version, fileSpec) ;

    where

        <version>	- I
            specifies whether to strip the ID3v1 or ID3v2 tags from the files.
        <fileSpec>	- I
            is the (possibly wildcard) specification of the file(s).
        <status>	- O
            returns the status of stripping the tags, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  stripTags (

#    if PROTOTYPES
        Id3Version  version,
        const  char  *fileSpec)
#    else
        version, fileSpec)

        Id3Version  version,
        char  *fileSpec ;
#    endif

{    /* Local variables. */
    char  *fileName ;
    DirectoryScan  scan ;



/*******************************************************************************
    Scan the specified files and strip their ID3 tags.
*******************************************************************************/

    if (drsCreate (fileSpec, &scan))  return (errno) ;

    fileName = (char *) drsFirst (scan) ;

    while (fileName != NULL) {

        id3Strip (fileName, version) ;

        fileName = (char *) drsNext (scan) ;

    }

    drsDestroy (scan) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    trimFields ()


Purpose:

    Function trimFields() trims trailing blanks from the text fields
    in the ID3v1 tags of the specified files.


    Invocation:

        status = trimFields (fileSpec) ;

    where

        <fileSpec>	- I
            is the (possibly wildcard) specification of the file(s).
        <status>	- O
            returns the status of trimming the fields, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  trimFields (

#    if PROTOTYPES
        const  char  *fileSpec)
#    else
        fileSpec)

        char  *fileSpec ;
#    endif

{    /* Local variables. */
    char  buffer[256], *fileName, *s ;
    DirectoryScan  scan ;
    Id3Tag  tag ;



/*******************************************************************************
    Scan the specified files and load any ID3 tags that are found.
*******************************************************************************/

    if (drsCreate (fileSpec, &scan))  return (errno) ;

    fileName = (char *) drsFirst (scan) ;

    while (fileName != NULL) {

        if (!id3Get (fileName, ID3V_V1, &tag) && (tag != NULL)) {

            s = (char *) id3Album (tag, NULL) ;
            if (s != NULL) {
                strcpy (buffer, s) ;  strTrim (buffer, -1) ;
                id3Album (tag, buffer) ;
            }

            s = (char *) id3Artist (tag, NULL) ;
            if (s != NULL) {
                strcpy (buffer, s) ;  strTrim (buffer, -1) ;
                id3Artist (tag, buffer) ;
            }

            s = (char *) id3Comment (tag, NULL) ;
            if (s != NULL) {
                strcpy (buffer, s) ;  strTrim (buffer, -1) ;
                id3Comment (tag, buffer) ;
            }

            s = (char *) id3Song (tag, NULL) ;
            if (s != NULL) {
                strcpy (buffer, s) ;  strTrim (buffer, -1) ;
                id3Song (tag, buffer) ;
            }

            id3Set (fileName, tag) ;

            id3Destroy (tag) ;

        }

        fileName = (char *) drsNext (scan) ;

    }

    drsDestroy (scan) ;

    return (0) ;

}
