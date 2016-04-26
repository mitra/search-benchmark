/* $Id: duop.c,v 1.2 2011/03/31 22:30:55 alex Exp alex $ */
/*******************************************************************************

Process:

    duop

    Dump Opera Files.


Author:    Alex Measday


Purpose:

    Program DUOP dumps binary files used by the Opera web browser.  The file
    format is defined at

        http://www.opera.com/docs/fileformats/

    and is used for the cookies file ("cookies4.dat"), the download rescue
    file ("download.dat"), and the visited links file ("vlink4.dat").  The
    format is supposedly used for the disk cache files, but I don't see any
    likely candidates in my Opera 9 cache directories.

    The output of DUOP is currently Scheme-friendly S-expressions.


    Invocation:

        % duop [-debug]
               [-cache] [-cookies] [-download] [-index] [-visited]
               [<file>]

    where

        "-debug"
            writes debug information to standard output.
        "-cache"
        "-cookies"
        "-download"
        "-index"
        "-visited"
            specify the type of Opera file being dumped.  By default, DUOP
            attempts to determine the file type based on the tag ID in the
            record following the header record.
        "<file>"
            is the file to dump; if no file is specified, input is read from
            standard input.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "meo_util.h"			/* Memory operations. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */


#define  INDENT(depth)		\
	(printf ("%.*s",	\
	(int) (depth) * 4,	\
	"                                                                                "))

#define  MSB_VALUE(numBytes)  (0x00000001 << (((numBytes) * 8) - 1))

#define  TIME_FORMAT  "%Y-%b-%d %H:%M:%S"


/*!*****************************************************************************
    getItem() - gets an unsigned integer of a specified number of bytes
        from a memory buffer.
*******************************************************************************/

static  unsigned  long  getItem (

#    if PROTOTYPES
        unsigned  char  *buffer,
        unsigned  int  itemLength)
#    else
        buffer, itemLength)

        unsigned  char  *buffer ;
        unsigned  int  itemLength ;
#    endif

{
    unsigned  int  i ;
    unsigned  long  item = 0 ;

    for (i = 0 ;  i < itemLength ;  i++)
        item = (item << 8) | buffer[i] ;

    return (item) ;

}


/*!*****************************************************************************
    getLength() - gets a lenght field from a memory buffer.  Global variable
        gLengthLength should be set to the correct value (available in the
        header record of each file) before calling getLength().
*******************************************************************************/

static  unsigned  int  gLengthLength = 2 ;	/* # of bytes for lengths. */

#define  getLength(buffer)  getItem ((buffer), gLengthLength)


/*!*****************************************************************************
    getTag() - gets a tag from a memory buffer.  Global variable gTagLength
        should be set to the correct value (available in the header record
        of each file) before calling getTag().
*******************************************************************************/

static  unsigned  int  gTagLength = 1 ;		/* # of bytes for tags. */

#define  getTag(buffer)  getItem ((buffer), gTagLength)

/*!*****************************************************************************
    DUOP's Main Program.
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

{  /* Local variables. */
    bool  debug, impliedPath, inCookie, inHTTPHeader, tagUsed ;
    char  *argument, *idtagString, *lengthString ;
    FILE  *file ;
    int  errflg, i, length, option ;
    OptContext  scan ;
    size_t  depth, inDomain, inPath ;
    struct  FileHeader {
        unsigned  long  file_version_number ;
        unsigned  long  app_version_number ;
        unsigned  int  idtag_length ;
        unsigned  int  length_length ;
    }  header ;
    struct  timeval  timestamp ;
    unsigned  char  *b, buffer[64*1024] ;
    unsigned  long  recID, recLength, tagID, tagMask, value ;

    enum {
        UnknownFormat,
        CookieFile,
        DiskCache,
        DiskCacheIndex,
        DownloadRescue,
        VisitedLinks
    }  fileType ;

    const  char  *optionList[] = {	/* Command line options. */
        "{cache}", "{cookies}",
        "{debug}", "{download}",
        "{index}", "{visited}", NULL
    } ;




    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    debug = false ;
    file = stdin ;
    fileType = UnknownFormat ;
    depth = 0 ;
    impliedPath = false ;
    inDomain = 0 ;  inPath = 0 ;  inCookie = false ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-cache" */
            fileType = DiskCache ;
            break ;
        case 2:			/* "-cookies" */
            fileType = CookieFile ;
            break ;
        case 3:			/* "-debug" */
            debug = true ;
            break ;
        case 4:			/* "-download" */
            fileType = DownloadRescue ;
            break ;
        case 5:			/* "-index" */
            fileType = DiskCacheIndex ;
            break ;
        case 6:			/* "-visited" */
            fileType = VisitedLinks ;
            break ;
        case NONOPT:		/* "<file>" */
            file = fopen (argument, "rb") ;
            if (file == NULL) {
                LGE "[%s] Error opening input file: %s\nfopen: ",
                    argv[0], argument) ;
                errflg++ ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  duop [-cache] [-cookies] [-debug] [-download] [-index] [-visited] [<file>]\n") ;
        exit (EINVAL) ;
    }

/*******************************************************************************
    Read the File Header.
*******************************************************************************/

    if (fread (buffer, 12, 1, file) != 1) {
        LGE "[%s] Error reading file header.\nfread: ", argv[0]) ;
        exit (errno) ;
    }

    if (debug)  meoDumpX (stdout, NULL, 0, buffer, 12) ;

    header.file_version_number = ((unsigned long) buffer[0] << 24) |
                                 ((unsigned long) buffer[1] << 16) |
                                 ((unsigned long) buffer[2] << 8) |
                                 (unsigned long) buffer[3] ;

    header.app_version_number = ((unsigned long) buffer[4] << 24) |
                                ((unsigned long) buffer[5] << 16) |
                                ((unsigned long) buffer[6] << 8) |
                                (unsigned long) buffer[7] ;

    header.idtag_length = ((unsigned int) buffer[8] << 8) |
                          (unsigned int) buffer[9] ;
    gTagLength = header.idtag_length ;

    header.length_length = ((unsigned int) buffer[10] << 8) |
                           (unsigned int) buffer[11] ;
    gLengthLength = header.length_length ;

    switch (gTagLength) {
    case 1:  idtagString = "uint8" ;  break ;
    case 2:  idtagString = "uint16" ;  break ;
    case 3:  idtagString = "uint24" ;  break ;
    case 4:  idtagString = "uint32" ;  break ;
    default: idtagString = "unknown" ;  break ;
    }

    switch (gLengthLength) {
    case 1:  lengthString = "uint8" ;  break ;
    case 2:  lengthString = "uint16" ;  break ;
    case 3:  lengthString = "uint24" ;  break ;
    case 4:  lengthString = "uint32" ;  break ;
    default: lengthString = "unknown" ;  break ;
    }

    printf ("File: %lu.%lu  App: 0x%08lX  IDTag: %s  Length: %s\n",
            (header.file_version_number >> 12),
            (header.file_version_number & 0x0FFF),
            header.app_version_number, idtagString, lengthString) ;

/*******************************************************************************
    Read the data records.
*******************************************************************************/

    tagMask = MSB_VALUE (gTagLength) ;

    for (i = 0 ;  ;  i++) {

        if (fread (buffer, gTagLength, 1, file) != 1) {
            if (feof (file))  exit (0) ;
            LGE "[%s] Error reading data tag for record %d.\nfread: ",
                argv[0], i) ;
            exit (errno) ;
        }

        recID = getTag (buffer) ;

        if (fileType == UnknownFormat) {
            switch (recID & ~tagMask) {
            case 0x0001:			/* Also the disk cache file! */
                fileType = CookieFile ;
                break ;
            case 0x0002:
                fileType = VisitedLinks ;
                break ;
            case 0x0040:
                fileType = DiskCacheIndex ;
                break ;
            case 0x0041:
                fileType = DownloadRescue ;
                break ;
            default:
                break ;
            }
        }

        if ((recID & tagMask) == 0) {
            if (fread (&buffer[gTagLength], gLengthLength, 1, file) != 1) {
                LGE "[%s] Error reading data length for record %d.\nfread: ",
                    argv[0], i) ;
                exit (errno) ;
            }
            recLength = getLength (&buffer[gTagLength]) ;
        } else {
            recLength = 0 ;
        }

        if (recLength > 0) {
            if (fread (&buffer[gTagLength], recLength, 1, file) != 1) {
                LGE "[%s] Error reading %lu bytes of data for record %d.\nfread: ",
                    argv[0], recLength, i) ;
                exit (errno) ;
            }
        }

        recLength += gTagLength ;
        if (debug) {
            printf ("----- recID = 0x%04lX  recLength = %lu\n",
                    recID, recLength) ;
            meoDumpX (stdout, NULL, 0, buffer, recLength) ;
        }

/*******************************************************************************
    Cookie File
*******************************************************************************/

        if (fileType == CookieFile) {

            b = buffer ;

            while ((unsigned long) (b - buffer) < recLength) {

                tagID = getTag (b) ;
                b += gTagLength ;

                if (debug) {
                    printf ("----- tagID = 0x%04lX  depth = %lu  C = %s  P = %lu  D = %lu\n",
                            tagID, (unsigned long) depth,
                            (inCookie ? "yes" : "no"),
                            (unsigned long) inPath,
                            (unsigned long) inDomain) ;
                }

                tagUsed = true ;

                switch (tagID & ~tagMask) {
                case 0x0003:			/* Begin cookie description. */
                    if (inCookie) {
                        INDENT (--depth) ;
                        printf ("))\n") ;
                    } else if (impliedPath) {
                        INDENT (depth++) ;	/* Begin implied path component. */
                        printf ("(duop-path (\n") ;
                        inPath++ ;
                        impliedPath = false ;
                    }
                    INDENT (depth++) ;
                    printf ("(duop-cookie (\n") ;
                    inCookie = true ;
                    break ;
                case 0x0010:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(name . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0011:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(value . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0012:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    INDENT (depth) ;
                    printf ("(expiry . \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x0013:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    INDENT (depth) ;
                    printf ("(last-use . \"%s\")\n", tvShow (timestamp, true, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x0014:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(comment . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0015:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(comment-url . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0016:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(domain . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0017:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(path . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0018:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(limitations . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0019:
                    INDENT (depth) ;
                    if (tagID & tagMask)
                        printf ("(https-only . #t)\n") ;
                    else
                        printf ("(https-only . #f)\n") ;
                    break ;
                case 0x001A:
                    INDENT (depth) ;
                    printf ("(version . %d)\n", (int) *b++) ;
                    break ;
                case 0x001B:
                    INDENT (depth) ;
                    if (tagID & tagMask)
                        printf ("(origin-only . #t)\n") ;
                    else
                        printf ("(origin-only . #f)\n") ;
                    break ;
                case 0x001C:
                    INDENT (depth) ;
                    if (tagID & tagMask)
                        printf ("(reserved . #t)\n") ;
                    else
                        printf ("(reserved . #f)\n") ;
                    break ;
                case 0x0020:
                    INDENT (depth) ;
                    if (tagID & tagMask)
                        printf ("(only-prefix . #t)\n") ;
                    else
                        printf ("(only-prefix . #f)\n") ;
                    break ;
                case 0x0022:
                    INDENT (depth) ;
                    if (tagID & tagMask)
                        printf ("(logged-in . #t)\n") ;
                    else
                        printf ("(logged-in . #f)\n") ;
                    break ;
                case 0x0023:
                    INDENT (depth) ;
                    if (tagID & tagMask)
                        printf ("(authenticated . #t)\n") ;
                    else
                        printf ("(authenticated . #f)\n") ;
                    break ;
                case 0x0024:
                    INDENT (depth) ;
                    if (tagID & tagMask)
                        printf ("(third-party . #t)\n") ;
                    else
                        printf ("(third-party . #f)\n") ;
                    break ;
                default:
                    tagUsed = false ;
                    break ;
                }

                if (tagUsed)  continue ;

                if (inCookie) {			/* End cookie description. */
                    INDENT (--depth) ;
                    printf ("))\n") ;
                    inCookie = false ;
                }

                tagUsed = true ;

                switch (tagID & ~tagMask) {
                case 0x0002:			/* Begin path component. */
                    INDENT (depth++) ;
                    printf ("(duop-path (\n") ;
                    inPath++ ;
                    break ;
                case 0x001D:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(name . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0005:			/* End path component. */
                    if (impliedPath) {
                        impliedPath = false ;	/* Empty implied path. */
                        break ;
                    }
                    INDENT (--depth) ;
                    printf ("))\n") ;
                    inPath-- ;
                    break ;
                default:
                    tagUsed = false ;
                    break ;
                }

                if (tagUsed)  continue ;

                tagUsed = true ;

                switch (tagID & ~tagMask) {
                case 0x0001:			/* Begin domain component. */
                    INDENT (depth++) ;
                    printf ("(duop-domain (\n") ;
                    inDomain++ ;
                    impliedPath = true ;
                    break ;
                case 0x001E:
                    length = getLength (b) ;
                    b += gLengthLength ;
                    INDENT (depth) ;
                    printf ("(name . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x001F:
                    INDENT (depth) ;
                    printf ("(filter . %d)\n", (int) *b++) ;
                    break ;
                case 0x0021:
                    INDENT (depth) ;
                    printf ("(handle . %d)\n", (int) *b++) ;
                    break ;
                case 0x0025:
                    INDENT (depth) ;
                    printf ("(warn . %d))\n", (int) *b++) ;
                    break ;
                case 0x0004:			/* End domain component. */
                    if (!inDomain)  break ;	/* Ignore EOD at EOF. */
                    INDENT (--depth) ;
                    printf ("))\n") ;
                    inDomain-- ;
                    break ;
                default:
                    tagUsed = false ;
                    break ;
                }

            }

        }

/*******************************************************************************
    Visited Links File
*******************************************************************************/

        else if (fileType == VisitedLinks) {

            b = buffer ;

            while ((unsigned long) (b - buffer) < recLength) {

                tagID = getTag (b) ;
                b += gTagLength ;

                if (debug)  printf ("----- tagID = 0x%04lX\n", tagID) ;

                switch (tagID & ~tagMask) {
                case 0x0002:			/* Visited links record. */
                    printf ("(duop-visited (\n") ;
                    break ;
                case 0x0003:			/* Fully-qualified URL. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("    (url . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0004:			/* Time of last visit. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf ("    (last-visit . \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x000B:
                    if (tagID & tagMask)
                        printf ("    (form-query . #t)\n") ;
                    else
                        printf ("    (form-query . #f)\n") ;
                    break ;
                case 0x0022:			/* Relative link record. */
                    length = getLength (b) ;	/* Skip record length. */
                    b += gLengthLength ;
                    printf ("    (relative-link") ;
                    break ;
                case 0x0023:			/* Relative link name. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf (" \"%.*s\"", length, b) ;
                    b += length ;
                    break ;
                case 0x0024:			/* Time of last visit. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf (" \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                default:
                    fprintf (stderr, "***** Unrecognized tagID = 0x%04lX\n", tagID) ;
                    break ;
                }

            }

            printf ("))\n") ;			/* End of record. */

        }

/*******************************************************************************
    Download Rescue File
*******************************************************************************/

        else if (fileType == DownloadRescue) {

            inHTTPHeader = false ;

            b = buffer ;

            while ((unsigned long) (b - buffer) < recLength) {

                tagID = getTag (b) ;
                b += gTagLength ;

                if (debug)  printf ("----- tagID = 0x%04lX\n", tagID) ;

                tagUsed = true ;

                switch (tagID & ~tagMask) {
                case 0x0010:			/* HTTP header record. */
                    length = getLength (b) ;	/* Skip record length. */
                    b += gLengthLength ;
                    printf ("    (http-header (\n") ;
                    inHTTPHeader = true ;
                    break ;
                case 0x0015:			/* HTTP date header. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (http-date . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0016:			/* Expiry date. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf ("        (expiry . \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x0017:			/* Last-modified date. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (last-modified . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0018:			/* MIME type. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (mime-type . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0019:			/* Entity tag. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (entity-tag . %.*s)\n", length, b) ;
                    b += length ;
                    break ;
                case 0x001A:			/* Moved to URL. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (moved-to . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x001B:			/* Response text. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (response-text . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x001C:			/* Response code. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    value = getItem (b, length) ;
                    printf ("        (response-code . %lu)\n", value) ;
                    b += length ;
                    break ;
                case 0x001D:			/* Refresh URL. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (refresh-url . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x001E:			/* Refresh delta time. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    value = getItem (b, length) ;
                    printf ("        (refresh-delta . %lu)\n", value) ;
                    b += length ;
                    break ;
                case 0x001F:			/* Suggested file name. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (suggested-file . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0020:			/* Content encodings. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (content-encoding . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0021:			/* Content location. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("        (content-location . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0025:			/* User Agent string. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    value = getItem (b, length) ;
                    printf ("        (user-agent . %lu)\n", value) ;
                    b += length ;
                    break ;
                case 0x0026:			/* User Agent sub version. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    value = getItem (b, length) ;
                    printf ("        (user-agent-sub . %lu)\n", value) ;
                    b += length ;
                    break ;
                default:
                    tagUsed = false ;
                    break ;
                }

                if (tagUsed)  continue ;

                if (inHTTPHeader) {
                    printf ("    ))\n") ;	/* End HTTP Header record. */
                    inHTTPHeader = false ;
                }

                tagUsed = true ;

                switch (tagID & ~tagMask) {
                case 0x0041:			/* Download rescue record. */
                    printf ("(duop-download (\n") ;
                    break ;
                case 0x0003:			/* Fully-qualified URL. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("    (url . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x0004:			/* Time of last visit. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf ("    (last-visit . \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x0005:			/* Time of last load. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf ("    (last-load . \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x0007:			/* Load status. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    value = getItem (b, length) ;
                    printf ("    (load-status . %lu)\n", value) ;
                    b += length ;
                    break ;
                case 0x0008:			/* Content size. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    value = getItem (b, length) ;
                    printf ("    (content-size . %lu)\n", value) ;
                    b += length ;
                    break ;
                case 0x0009:			/* MIME type. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("    (mime-type . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x000A:			/* Character set. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("    (char-set . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x000B:
                    if (tagID & tagMask)
                        printf ("    (form-query . #t)\n") ;
                    else
                        printf ("    (form-query . #f)\n") ;
                    break ;
                case 0x000C:
                    if (tagID & tagMask)
                        printf ("    (stored-locally . #t)\n") ;
                    else
                        printf ("    (stored-locally . #f)\n") ;
                    break ;
                case 0x000D:			/* File name. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf ("    (file-name . \"%.*s\")\n", length, b) ;
                    b += length ;
                    break ;
                case 0x000F:
                    if (tagID & tagMask)
                        printf ("    (check-if-modified . #t)\n") ;
                    else
                        printf ("    (check-if-modified . #f)\n") ;
                    break ;
                case 0x0022:			/* Relative link record. */
                    length = getLength (b) ;	/* Skip record length. */
                    b += gLengthLength ;
                    printf ("    (relative-link") ;
                    break ;
                case 0x0023:			/* Relative link name. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    printf (" \"%.*s\"", length, b) ;
                    b += length ;
                    break ;
                case 0x0024:			/* Time of last visit. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf (" \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x0028:			/* Time of last start. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf ("    (last-start . \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x0029:			/* Time of last stop. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    timestamp = tvCreate ((long) getItem (b, length), 0) ;
                    printf ("    (last-stop . \"%s\")\n", tvShow (timestamp, false, TIME_FORMAT)) ;
                    b += length ;
                    break ;
                case 0x002A:			/* Size of last segment. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    value = getItem (b, length) ;
                    printf ("    (last-size . %lu)\n", value) ;
                    b += length ;
                    break ;
                case 0x002C:			/* Unknown tag. */
                case 0x0032:			/* Unknown tag. */
                case 0x0033:			/* Unknown tag. */
                case 0x0034:			/* Unknown tag. */
                case 0x0035:			/* Unknown tag. */
                    length = getLength (b) ;
                    b += gLengthLength ;
                    if (length > 0) {
                        value = getItem (b, length) ;
                        printf ("    (TAG-%04lX . %lu)\n", tagID, value) ;
                    } else {
                        printf ("    (TAG-%04lX . '())\n", tagID) ;
                    }
                    b += length ;
                    break ;
                default:
                    fprintf (stderr, "***** Unrecognized tagID = 0x%04lX\n", tagID) ;
                    break ;
                }

            }

            printf ("))\n") ;			/* End of record. */

        }

    }


    exit (errno) ;

}
