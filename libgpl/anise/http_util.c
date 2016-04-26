/* $Id: http_util.c,v 1.9 2012/05/06 22:32:25 alex Exp alex $ */
/*******************************************************************************

File:

    http_util.c

    HTTP Utilities.


Author:    Alex Measday


Purpose:

    The HTTP_UTIL functions perform various HTTP functions.


Public Procedures:

    httpConvert() - converts escape sequences in an HTTP message line.
    httpEvaluate() - evaluates an HTTP command.
    httpLog() - logs an HTTP transaction.
    httpResolve() - resolves the resource name from an HTTP message.
    httpTypeOf() - returns the MIME type of a file.

Private Procedures:

    httpGET() - executes an HTTP GET command.
    httpHEAD() - executes an HTTP HEAD command.
    httpPOST() - executes an HTTP POST command.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#if defined(HAVE_MEMCPY) && !HAVE_MEMCPY
#    define  memmove(dest,src,length)  bcopy(src,dest,length)
#endif
#if defined(VMS)
#    include  <file.h>			/* File definitions. */
#    include  <unixio.h>		/* VMS-emulation of UNIX I/O. */
#    define  S_ISDIR(mode)  ((((mode) & S_IFMT) & S_IFDIR) != 0)
#    define  S_ISLNK(mode)  false
#elif defined(VXWORKS)
#    include  <ioLib.h>			/* I/O library definitions. */
#    define  HAVE_LSTAT  0
#else
#    include  <fcntl.h>			/* File control definitions. */
#    ifdef _WIN32
#        include  <io.h>		/* UNIX I/O definitions. */
#        define  S_ISDIR(mode)  ((((mode) & _S_IFMT) & _S_IFDIR) != 0)
#        define  S_ISLNK(mode)  false
#    else
#        include  <unistd.h>		/* UNIX I/O definitions. */
#    endif
#endif
#include  <sys/stat.h>			/* File status definitions. */
#if defined(HAVE_LSTAT) && !HAVE_LSTAT
#    define  lstat  stat		/* OS doesn't support links. */
#endif
#include  "drs_util.h"			/* Directory scanning utilities. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "hash_util.h"			/* Hash table definitions. */
#include  "lfn_util.h"			/* LF-terminated network I/O. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "anise.h"			/* ANISE definitions. */
#include  "http_util.h"			/* HTTP utilities. */


char  *tildeTranslation = NULL ;	/* Format string for tilde translation. */


/*******************************************************************************
    Private Functions.
*******************************************************************************/

static  errno_t  httpGET (
#    if PROTOTYPES
        LfnStream  client,
        const  char  *resource,
        const  char  *version,
        ResponseInfo  *response
#    endif
    ) ;

static  errno_t  httpHEAD (
#    if PROTOTYPES
        LfnStream  client,
        const  char  *resource,
        const  char  *version,
        ResponseInfo  *response
#    endif
    ) ;

static  errno_t  httpPOST (
#    if PROTOTYPES
        LfnStream  client,
        const  char  *resource,
        const  char  *version,
        const  char  *body,
        ResponseInfo  *response
#    endif
    ) ;

/*!*****************************************************************************

Procedure:

    httpConvert ()

    Convert Escape Sequences in an HTTP Line.


Purpose:

    Function httpConvert() converts escape sequences in an HTTP line of text
    to the corresponding characters.


    Invocation:

        length = httpConvert (text) ;

    where:

        <text>		- I
            is the line of text to be converted; the conversion is done
            in place.
        <length>	- O
            returns the length of the converted line.

*******************************************************************************/


size_t  httpConvert (

#    if PROTOTYPES
        char  *text)
#    else
        text)

        char  *text ;
#    endif

{    /* Local variables. */
    char  buffer[3], *s ;



/* Convert each escape sequence in the text to the corresponding ASCII
   character. */

    s = text ;
    for ( ; ; ) {
        s = strchr (s, '%') ;
        if ((s == NULL) || (strlen (s) < 3))  break ;
        if (isxdigit ((unsigned char) s[1]) &&
            isxdigit ((unsigned char) s[2])) {
            buffer[0] = s[1] ;  buffer[1] = s[2] ;  buffer[2] = '\0' ;
            *s++ = (char) strtol (buffer, NULL, 16) ;
            memmove (s, s+2, strlen (s+2) + 1) ;
        }
        s++ ;
    }

    return (strlen (text)) ;

}

/*!*****************************************************************************

Procedure:

    httpEvaluate ()

    Evaluate an HTTP Command.


Purpose:

    Function httpEvaluate() evaluates an HTTP command.


    Invocation:

        status = httpEvaluate (client, numLines, header, body,
                               &keepAlive, &response) ;

    where:

        <client>	- I
            is the stream handle for the network connection with the client.
        <numLines>	- I
            is the number of lines of text in the HTTP message header.
        <header>	- I
            is an array of the lines of text from the HTTP header.
        <body>		- I
            is a pointer to the message body, if any.
        <keepAlive>	- O
            returns true if the client requested that the connection be kept
            "alive" and false otherwise.
        <response>	- O
            returns information about the server's response to the command.
        <status>	- O
            returns the status of processing the HTTP command, zero if
            there were no errors and ERRNO otherwise.  Note that this
            status is independent of the status returned to the client.

*******************************************************************************/


errno_t  httpEvaluate (

#    if PROTOTYPES
        LfnStream  client,
        int  numLines,
        char  *header[],
        char  *body,
        bool  *keepAlive,
        ResponseInfo  *response)
#    else
        client, numLines, header, body, keepAlive, response)

        LfnStream  client ;
        int  numLines ;
        char  *header[] ;
        char  *body ;
        bool  *keepAlive ;
        ResponseInfo  *response ;
#    endif

{    /* Local variables. */
    char  *command, *resource, *s, *version ;
    int  i, status ;




/* Initialize the response information. */

    if (response != NULL) {
        response->status = -1 ;
        response->numBytes = -1 ;
        s = (char *) sktPeer (lfnFd (client)) ;
        strcpy (response->peer, (s == NULL) ? "-" : s) ;
    }

/* Check to see if the client has requested that the connection remain alive. */

    *keepAlive = false ;
    for (i = 1 ;  i < numLines ;  i++) {
        s = strchr (header[i], ':') ;
        if (s != NULL) {
            s++ ;
            s = s + strspn (s, " \t") ;
            if ((strlen (s) >= strlen ("Keep-Alive")) &&
                (strncmp (s, "Keep-Alive", strlen ("Keep-Alive")) == 0)) {
                *keepAlive = true ;
                break ;
            }
        }
    }

/* Parse the HTTP command line. */

    s = strdup (header[0]) ;
    command = strtok (s, " \t") ;
    resource = strtok (NULL, " \t") ;
    version = strtok (NULL, " \t") ;

/* Execute the command. */

    if (strcmp (command, "GET") == 0) {
        status = httpGET (client, resource, version, response) ;
        free (s) ;  SET_ERRNO (status) ;
        return (errno) ;
    } else if (strcmp (command, "HEAD") == 0) {
        status = httpHEAD (client, resource, version, response) ;
        free (s) ;  SET_ERRNO (status) ;
        return (errno) ;
    } else if (strcmp (command, "POST") == 0) {
        status = httpPOST (client, resource, version, version, response) ;
        free (s) ;  SET_ERRNO (status) ;
        return (errno) ;
    } else {
        free (s) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    httpLog ()

    Log an HTTP Transaction.


Purpose:

    Function httpLog() records an HTTP request/response transaction in
    an HTTP log file.  The log file follows the combined, Common Log File
    format, so each entry in the file is a single ASCII line formatted as
    follows:

        <client> - - <date> "<request>" <status> <bytes> <referer> <agent>

    where:

        <client> is the name of the client's host.

        "- -" are two N/A fields that are supposed to contain information
            about the client supplied by an authentication server on the
            client's host.  The relevant protocol is RFC 931 (Authentication
            Server), issued in 1985 and now obsolete!

        <request> is the HTTP request (e.g., "GET <url> HTTP/1.0"), enclosed
            in double quotes.

        <status> is the HTTP status code returned to the client.

        <bytes> is the number of bytes of data (excluding the HTTP header)
            sent to the client.

        <referer> is the "Referer:" field from the HTTP request header;
            e.g., the URL of the page containing a link for the requested
            item.

        <agent> is the "User-Agent:" field from the HTTP request header;
            e.g., the name of the client's browser.

    If a field cannot be determined or is not applicable, "-" is substituted.


    Invocation:

        status = httpLog (logFile, client, numLines, header, response) ;

    where:

        <logFile>	- I
            is the LogFile handle for the log file.  If this argument is NULL,
            the log is written to standard output.
        <client>	- I
            is the stream handle for the network connection with the client.
        <numLines>	- I
            is the number of lines of text in the HTTP message header.
        <header>	- I
            is an array of the lines of text from the HTTP header.
        <response>	- I
            is the address of information about the server's response
            to the command.
        <status>	- O
            returns the status of logging the transaction, zero if there
            were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  httpLog (

#    if PROTOTYPES
        LogFile  logFile,
        LfnStream  client,
        int  numLines,
        char  *header[],
        ResponseInfo  *response)
#    else
        logFile, client, numLines, header, response)

        LogFile  logFile ;
        LfnStream  client ;
        int  numLines ;
        char  *header[] ;
        ResponseInfo  *response ;
#    endif

{    /* Local variables. */
    char  *s ;
    int  i, length ;




/* <host> <rfc931> <authUser> <date> "<request>" ... */

    logWrite (logFile, "%s - - %s \"%s\"",
              response->peer,
              tvShow (tvTOD (), 1, "%d/%b/%Y:%H:%M:%S"),
              header[0]) ;

/* ... <status> <numBytes> ... */

    logWrite (logFile, (response->status >= 0) ? " %d" : " -",
              response->status) ;
    logWrite (logFile, (response->numBytes >= 0) ? " %ld" : " -",
              response->numBytes) ;

/* ... <referer> ... */

    s = "-" ;  length = 1 ;
    for (i = 1 ;  i < numLines ;  i++) {
        if (strncmp (header[i], "Referer:", 8) == 0) {
            s = &header[i][8] + strspn (&header[i][8], " \t") ;
            length = strcspn (s, " \t") ;
            break ;
        }
    }
    logWrite (logFile, " %.*s", length, s) ;

/* ... <agent> */

    s = "-" ;  length = 1 ;
    for (i = 1 ;  i < numLines ;  i++) {
        if (strncmp (header[i], "User-Agent:", 11) == 0) {
            s = &header[i][11] + strspn (&header[i][11], " \t") ;
            length = strcspn (s, " \t") ;
            break ;
        }
    }
    logWrite (logFile, " %.*s\n", length, s) ;

    logFlush (logFile) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    httpResolve ()

    Resolve a Resource Name from an HTTP Command.


Purpose:

    Function httpResolve() converts the resource name (URI) received in an
    HTTP GET or POST command into a fully-qualified file name.


    Invocation:

        pathname = httpResolve (resource) ;

    where:

        <resource>	- I
            is the name of the resource as received in the HTTP command.
        <pathname>	- O
            returns the fully-qualified pathname for the resource.
            The pathname should be used or duplicated before calling
            httpResolve() again.

*******************************************************************************/


const  char  *httpResolve (

#    if PROTOTYPES
        const  char  *resource)
#    else
        resource)

        char  *resource ;
#    endif

{    /* Local variables. */
    char  *firstSlash, temp[PATH_MAX] ;
    static  char  pathname[PATH_MAX] ;




/* If necessary, get the tilde translation format string. */

    if (tildeTranslation == NULL) {
        tildeTranslation = getenv ("TILDE_TRANSLATION") ;
        if (tildeTranslation == NULL) {
            strcpy (pathname, fnmBuild (FnmDirectory, "~", NULL)) ;
            strcat (pathname, "%s/html/%s") ;
            tildeTranslation = strdup (pathname) ;
        }
    }

/* Ignore a leading "/".  The home directory "~/" defaults to the login
   directory of the user who started up ANISE. */

    if (*resource == '/')  resource++ ;

    if ((strlen (resource) >= 2) && (strncmp (resource, "~/", 2) == 0))
        resource += 2 ;

/* Perform tilde translation.  The tilde translation format string is of
   the form, "...%s...%s...", where the "%s"s are replaced by the user's
   name and the trailing portion of the resource name, respectively. */

    if (*resource == '~') {
        firstSlash = strchr (resource, '/') ;
        if (firstSlash == NULL) {
            sprintf (pathname, tildeTranslation, ++resource, "/") ;
        } else {
            strcpy (temp, ++resource) ;
            temp[firstSlash-resource] = '\0' ;
            sprintf (pathname, tildeTranslation, temp, ++firstSlash) ;
        }
    } else {
#if !defined(HAVE_GETLOGIN) || HAVE_GETLOGIN
        char  *s = getlogin () ;
#elif !defined(HAVE_CUSERID) || HAVE_CUSERID
        char  *s = cuserid (NULL) ;
#elif defined(_WIN32)
        char  *s = temp ;
        int  length = sizeof temp ;
        GetUserNameA (s, &length) ;
#else
        char  *s = NULL ;
#endif
        sprintf (pathname, tildeTranslation, (s == NULL) ? "" : s, resource) ;
    }

/* If no file was specified, assume "index.html". */

    if (strlen (pathname) == 0)  strcpy (pathname, "index.html") ;

    strcpy (pathname, fnmBuild (FnmPath, pathname, NULL)) ;

    return (pathname) ;

}

/*!*****************************************************************************

Procedure:

    httpTypeOf ()

    Determine the MIME Type of a File.


Purpose:

    Function httpTypeOf() returns the MIME type of a file based on its
    file extension.


    Invocation:

        mimeType = httpTypeOf (pathname) ;

    where:

        <pathname>	- I
            is the name of the file.
        <mimeType>	- O
            returns the MIME type of the file, as determined from the
            file's extension.

*******************************************************************************/


const  char  *httpTypeOf (

#    if PROTOTYPES
        const  char  *pathname)
#    else
        pathname)

        char  *pathname ;
#    endif

{    /* Local variables. */
    char  buffer[256], *extension, *typeFileName, *typeName ;
    FILE  *file ;
    static  HashTable  mimeTypes = NULL ;




/*******************************************************************************
    If the extension-MIME mapping hasn't been loaded yet, then do so.
*******************************************************************************/

    if (mimeTypes == NULL) {

/* Create a hash table for the MIME types. */

        if (hashCreate (64, &mimeTypes)) {
            LGE "(httpTypeOf) Error creating hash table for MIME types.\nhashCreate: ") ;
            return ("unknown") ;
        }

/* Open the MIME type definition file. */

        typeFileName = getenv ("MIME_TYPES") ;
        if (typeFileName == NULL)  typeFileName = "/etc/mime.types" ;

        file = fopen (typeFileName, "r") ;
        if (file == NULL) {
            LGE "(httpTypeOf) Error opening MIME type definition file: %s\nfopen: ",
                typeFileName) ;
            return ("unknown") ;
        }

/* Load the file-extension/MIME-type mappings from the file. */

        while (fgets (buffer, sizeof buffer, file) != NULL) {
            typeName = strtok (buffer, " \t\n") ;
            if ((typeName == NULL) || (*typeName == '#'))  continue ;
            for ( ; ; ) {
                extension = strtok (NULL, " \t\n") ;
                if (extension == NULL)  break ;
                if (hashAdd (mimeTypes, extension,
                             (void *) strdup (typeName))) {
                    LGE "(httpTypeOf) Error adding %s-%s mapping to MIME types table.\nhashAdd: ",
                        extension, typeName) ;
                    return ("unknown") ;
                }
            }
        }

/* Close the definition file. */

        fclose (file) ;

    }


/*******************************************************************************
    Look up the target file's extension in the MIME types table.
*******************************************************************************/

    extension = (char *) fnmBuild (FnmExtension, pathname, NULL) ;
    extension++ ;				/* Skip leading "." */

    if (hashSearch (mimeTypes, extension, (void **) &typeName))
        return (typeName) ;
    else
        return ("unknown") ;


}

/*!*****************************************************************************

Procedure:

    httpGET ()

    Execute an HTTP GET Command.


Purpose:

    Function httpGET() executes an HTTP GET command.


    Invocation:

        status = httpGET (client, resource, version, &response) ;

    where:

        <client>	- I
            is the stream handle for the network connection with the client.
        <resource>	- I
            is the name of the resource being retrieved.
        <version>	- I
            is the HTTP version provided by the client.
        <response>	- O
            returns information about the server's response to the command.
        <status>	- O
            returns the status of executing the GET command, zero if
            there were no errors and ERRNO otherwise.  Note that this
            status is independent of the status returned to the client.

*******************************************************************************/


static  errno_t  httpGET (

#    if PROTOTYPES
        LfnStream  client,
        const  char  *resource,
        const  char  *version,
        ResponseInfo  *response)
#    else
        client, resource, version, response)

        LfnStream  client ;
        char  *resource ;
        char  *version ;
        ResponseInfo  *response ;
#    endif

{    /* Local variables. */
    char  *buffer, *pathname, *s ;
    DirectoryScan  scan ;
    int  file, length ;
    struct  stat  info ;





/* Convert the URI to a fully-qualified pathname. */

    pathname = (char *) httpResolve (resource) ;

    if (stat (pathname, &info)) {
        LGE "(httpGET) File: %s\nstat: ", pathname) ;
        response->status = 404 ;
        return (lfnPutLine (client, -1.0, "%s 404 %s: %s\r\n\r\n",
                            version, pathname, strerror (errno))) ;
    }

/* If the URI specifies a directory and the directory has an index file,
   then use the index file as the target of the GET. */

    if (S_ISDIR (info.st_mode)) {
        strcat (pathname, "index.html") ;
        if (stat (pathname, &info)) {		/* Index file not found? */
            s = strrchr (pathname, '/') ;
            *++s = '\0' ;			/* Restore the directory. */
            stat (pathname, &info) ;
        }
    }


/*******************************************************************************
    If the target file is a directory, then list its contents.
*******************************************************************************/

    if (S_ISDIR (info.st_mode)) {

        strcat (pathname, "*") ;
        if (drsCreate (pathname, &scan)) {
            response->status = 401 ;
            return (lfnPutLine (client, -1.0, "%s 401 %s\r\n\r\n",
                                version, strerror (errno))) ;
        }
        pathname[strlen (pathname) - 1] = '\0' ;

        response->status = 200 ;
        if (lfnPutLine (client, -1.0, "%s 200\r\n", version) ||
            lfnPutLine (client, -1.0, "Content-type: text/html\r\n") ||
            lfnPutLine (client, -1.0, "\r\n") ||
            lfnPutLine (client, -1.0, "<HTML><HEAD><TITLE>%s</TITLE></HEAD>\r\n",
                        pathname) ||
            lfnPutLine (client, -1.0, "<H2>%s</H2>\r\n", pathname) ||
            lfnPutLine (client, -1.0, "<UL>\r\n")) {
            PUSH_ERRNO ;  drsDestroy (scan) ;  POP_ERRNO ;
            return (errno) ;
        }

        for (s = (char *) drsFirst (scan) ;
             s != NULL ;
             s = (char *) drsNext (scan)) {
            errno_t  status ;
            lstat (s, &info) ;  s = (char *) fnmBuild (FnmFile, s, NULL) ;
            if (S_ISDIR (info.st_mode)) {
                status = lfnPutLine (client, -1.0,
                                     "<LI> <A HREF=\"%s/\"><B>%s/</B></A>\r\n",
                                     s, s) ;
            } else if (S_ISLNK (info.st_mode)) {
                status = lfnPutLine (client, -1.0,
                                     "<LI> <A HREF=\"%s\"><CODE>%s</CODE>@</A>\r\n",
                                     s, s) ;
            } else {
                status = lfnPutLine (client, -1.0,
                                     "<LI> <A HREF=\"%s\"><CODE>%s</CODE></A><CODE>    </CODE><I>(%dK)</I>\r\n",
                                     s, s, (info.st_size + 1023) / 1024) ;
            }
            if (status) {
                PUSH_ERRNO ;  drsDestroy (scan) ;  POP_ERRNO ;
                return (errno) ;
            }
        }

        drsDestroy (scan) ;

        return (lfnPutLine (client, -1.0, "</UL></BODY></HTML>\r\n")) ;

    }


/*******************************************************************************
    The target file is a file; send its contents to the client.
*******************************************************************************/

/* Open the file. */

    file = open (pathname, O_RDONLY, 0) ;
    if (file < 0) {
        LGE "(httpGET) File: %s\nopen: ", pathname) ;
        response->status = 404 ;
        return (lfnPutLine (client, -1.0, "%s 404 %s: %s\r\n\r\n",
                            version, pathname, strerror (errno))) ;
    }

/* Send the response header to the client. */

    response->status = 200 ;
    if (lfnPutLine (client, -1.0, "%s 200\r\n", version) ||
        lfnPutLine (client, -1.0, "Content-type: %s\r\n",
                    httpTypeOf (pathname)) ||
        lfnPutLine (client, -1.0, "Content-length: %lu\r\n", info.st_size) ||
        lfnPutLine (client, -1.0, "\r\n")) {
        PUSH_ERRNO ;  close (file) ;  POP_ERRNO ;
        return (errno) ;
    }

/* Send the contents of the file. */

#define  BUFSIZE  8*1024
    buffer = malloc (BUFSIZE) ;
    if (buffer == NULL) {
        LGE "(httpGET) Error allocating %d-byte buffer.\nmalloc: ", BUFSIZE) ;
        PUSH_ERRNO ;  close (file) ;  POP_ERRNO ;
        return (errno) ;
    }

    response->numBytes = 0 ;
    while (0 < (length = read (file, buffer, BUFSIZE))) {
        if (lfnWrite (client, -1.0, length, buffer, NULL)) {
            PUSH_ERRNO ;  close (file) ;  free (buffer) ;  POP_ERRNO ;
            return (errno) ;
        }
        response->numBytes += length ;
    }

    free (buffer) ;

/* Close the file. */

    close (file) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    httpHEAD ()

    Execute an HTTP HEAD Command.


Purpose:

    Function httpHEAD() executes an HTTP HEAD command.


    Invocation:

        status = httpHEAD (client, resource, version, &response) ;

    where:

        <client>	- I
            is the stream handle for the network connection with the client.
        <resource>	- I
            is the name of the file being retrieved.
        <version>	- I
            is the HTTP version provided by the client.
        <response>	- O
            returns information about the server's response to the command.
        <status>	- O
            returns the status of executing the HEAD command, zero if
            there were no errors and ERRNO otherwise.  Note that this
            status is independent of the status returned to the client.

*******************************************************************************/


static  errno_t  httpHEAD (

#    if PROTOTYPES
        LfnStream  client,
        const  char  *resource,
        const  char  *version,
        ResponseInfo  *response)
#    else
        client, resource, version, response)

        LfnStream  client ;
        char  *resource ;
        char  *version ;
        ResponseInfo  *response ;
#    endif

{    /* Local variables. */
    char  *pathname ;
    struct  stat  info ;




/* Convert the URL-specified file name to a fully-qualified pathname. */

    pathname = (char *) httpResolve (resource) ;

    if (stat (pathname, &info)) {
        LGE "(httpHEAD) File: %s\nstat: ", pathname) ;
        response->status = 404 ;
        return (lfnPutLine (client, -1.0, "%s 404 %s: %s\r\n\r\n",
                            version, pathname, strerror (errno))) ;
    }

/* Send the response header to the client. */

    response->status = 200 ;
    response->numBytes = info.st_size ;

    if (lfnPutLine (client, -1.0, "%s 200\r\n", version) ||
        lfnPutLine (client, -1.0, "Content-type: %s\r\n",
                    httpTypeOf (pathname)) ||
        lfnPutLine (client, -1.0, "Content-length: %lu\r\n", info.st_size) ||
        lfnPutLine (client, -1.0, "\r\n")) {
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    httpPOST ()

    Execute an HTTP POST Command.


Purpose:

    Function httpPOST() executes an HTTP POST command.


    Invocation:

        status = httpPOST (client, resource, version, body, response) ;

    where:

        <client>	- I
            is the stream handle for the network connection with the client.
        <resource>	- I
            is the name of the file being retrieved.
        <version>	- I
            is the HTTP version provided by the client.
        <body>		- I
            is a pointer to the information being posted.
        <response>	- O
            returns information about the server's response to the command.
        <status>	- O
            returns the status of executing the POST command, zero if
            there were no errors and ERRNO otherwise.  Note that this
            status is independent of the status returned to the client.

*******************************************************************************/


static  errno_t  httpPOST (

#    if PROTOTYPES
        LfnStream  client,
        const  char  *resource,
        const  char  *version,
        const  char  *body,
        ResponseInfo  *response)
#    else
        client, resource, version, body, response)

        LfnStream  client ;
        char  *resource ;
        char  *version ;
        char  *body ;
        ResponseInfo  *response ;
#    endif

{

    response->status = 501 ;

    return (lfnPutLine (client, -1.0, "%s 501\r\n\r\n", version)) ;

}
