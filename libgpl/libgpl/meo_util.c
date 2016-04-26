/* $Id: meo_util.c,v 1.17 2011/07/18 17:36:19 alex Exp $ */
/*******************************************************************************

File:

    meo_util.c

    Memory Operations Package.


Author:    Alex Measday


Purpose:

    The MEO_UTIL functions perform various operations on memory regions.

    The meoDump() functions generate VMS-style dumps of arbitrary regions
    of memory.  Each line of output includes the address of the memory being
    dumped, the data (formatted in octal, decimal, or hexadecimal), and the
    ASCII equivalent of the data:

        ...
        00000060:  60616263 64656667 68696A6B 6C6D6E6F  "`abcdefghijklmno"
        00000070:  70717273 74757677 78797A7B 7C7D7E7F  "pqrstuvwxyz{|}~."
        00000080:  80818283 84858687 88898A8B 8C8D8E8F  "................"
        ...

    The MEO_UTIL package also provides a simple means of saving the contents
    of an arbitrary memory region to a file:

        #include  <stdio.h>		-- Standard I/O definitions.
        #include  "meo_util.h"		-- Memory operations.
        char  oldBuffer[1234] ;
        ...
        meoSave (oldBuffer, sizeof oldBuffer, "buffer.dat", 0) ;

    The contents of a file can be loaded into an existing region of memory:

        char  *newBuffer = oldBuffer ;
        int  numBytes = sizeof oldBuffer ;
        ...
        meoLoad ("buffer.dat", 0, &newBuffer, &numBytes) ;

    or into memory dynamically allocated by meoLoad():

        char  *newBuffer = NULL ;
        int  numBytes ;
        ...
        meoLoad ("buffer.dat", 0, &newBuffer, &numBytes) ;
        ... use the new buffer ...
        free (newBuffer) ;


Public Procedures (* defined as macros):

    meoDump() - outputs an ASCII dump of a memory region to a file.
  * meoDumpD() - outputs a decimal ASCII dump to a file.
  * meoDumpO() - outputs an octal ASCII dump to a file.
  * meoDumpT() - outputs a text ASCII dump to a file.
  * meoDumpX() - outputs a hexadecimal ASCII dump to a file.
    meoLoad() - loads the contents of a file into memory.
    meoSave() - saves the contents of memory to a file.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Standard character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C library definitions. */
#include  <string.h>			/* Standard C string functions. */
#ifdef _WIN32
#    include  <io.h>			/* Low-level I/O definitions. */
#endif
#if !defined(HAVE_STAT_H) || HAVE_STAT_H
#    include  <sys/stat.h>		/* File status definitions. */
#endif
#ifndef isascii
#    define  isascii(c)  ((unsigned char) (c) <= 0177)
#endif
#include  "fnm_util.h"			/* Filename utilities. */
#include  "meo_util.h"			/* Memory operations. */


int  meo_util_debug = 0 ;		/* Global debug switch (1/0 = yes/no). */
#undef  I_DEFAULT_GUARD
#define  I_DEFAULT_GUARD  meo_util_debug

/*!*****************************************************************************

Procedure:

    meoDump ()

    Generate an ASCII Dump of a Memory Region.


Purpose:

    Function meoDump() formats the binary contents of a memory region in ASCII
    and writes the ASCII dump to a file.  Each output line looks as follows:

        <address>:  <data1> <data2> ... <dataN>  "data/ASCII"

    The data items, DATA1, DATA2, etc., are formatted in decimal, hexadecimal,
    or octal notation.


    Invocation:

        status = meoDump (file, indentation, base, numBytesPerLine,
                          address, buffer, numBytesToDump) ;

    where

        <file>			- I
            is the Unix FILE* handle for the output file.  If FILE is NULL,
            the dump is written to standard output.
        <indentation>		- I
            is a text string used to indent each line of the dump.  The
            string is used as the format string in an FPRINTF(3) statement,
            so you can embed anything you want.  This argument can be NULL.
        <base>			- I
            specifies the output base for the dump: "MeoOctal" for octal,
            "MeoDecimal" for decimal, "MeoHexadecimal" for hexadecimal,
            and "MeoText" for text.  (These are enumerated values defined
            in the "meo_util.h" header file.)
        <numBytesPerLine>	- I
            specifies the number of bytes in the buffer to be formatted and
            dumped on a single line of output.  Good values are 8 values per
            line for octal, 8 for decimal, 16 for hexadecimal, and 40 for text.
        <address>		- I
            is the value to be displayed in the address field.  It can be
            different than the actual buffer address.
        <buffer>		- I
            is the buffer of data to be dumped.
        <numBytesToDump>	- I
            is exactly what it says it is!
        <status>		- O
            returns the status of generating the dump, zero if there were
            no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  meoDump (

#    if PROTOTYPES
        FILE  *file,
        const  char  *indentation,
        MeoBase  base,
        int  numBytesPerLine,
        void  *address,
        const  void  *buffer,
        int  numBytesToDump)
#    else
        file, indentation, base, numBytesPerLine,
        address, buffer, numBytesToDump)

        FILE  *file ;
        char  *indentation ;
        MeoBase  base ;
        int  numBytesPerLine ;
        void  *address ;
        void  *buffer ;
        int  numBytesToDump ;
#    endif

{    /* Local variables. */
    char  c, *out, *outbuf ;
    int  numBytesDumped, numBytesThisLine ;
    unsigned  char  *buf, *inbuf ;
    unsigned  long  number ;




    if (file == NULL)  file = stdout ;


/* Allocate an input buffer to hold the bytes to be dumped.  Doing this
   prevents bus errors should the input buffer be misaligned (i.e., on
   an odd-byte boundary). */

    inbuf = (unsigned char *) malloc (numBytesPerLine) ;
    if (inbuf == NULL) {
        LGE "(meoDump) Error allocating temporary, %d-byte input buffer.\nmalloc: ",
            numBytesPerLine) ;
        return (errno) ;
    }


/* Allocate a string in which to build each output line. */

    outbuf = (char *) malloc (numBytesPerLine * 6) ;
    if (outbuf == NULL) {
        LGE "(meoDump) Error allocating temporary, %d-byte output buffer.\nmalloc: ",
            numBytesPerLine * 6) ;
        return (errno) ;
    }


/*******************************************************************************
    Generate each line of the dump.
*******************************************************************************/

    while (numBytesToDump > 0) {


        numBytesThisLine = (numBytesToDump > numBytesPerLine)
                           ? numBytesPerLine : numBytesToDump ;

        memset (inbuf, '\0', numBytesPerLine) ;		/* Zero trailing bytes. */
        (void) memcpy (inbuf, buffer, numBytesThisLine) ;


/* Output the line indentation and the memory address. */

        if (indentation != NULL)  fprintf (file, indentation) ;
#ifdef YOU_WANT_VARIABLE_LENGTH_ADDRESS
        fprintf (file, "%p:\t", address) ;
#elif defined(__palmos__)
        HostFPrintF (HostLogFile (), "%08lX: ", (long) address) ;
#else
        fprintf (file, "%08lX: ", (long) address) ;	/* Assumes 32-bit address. */
#endif


/* Format the data in the requested base. */

        buf = (unsigned char *) inbuf ;  numBytesDumped = 0 ;
        out = outbuf ;


        switch (base) {

/* Base 8 - display the contents of each byte as an octal number. */

        case MeoOctal:
            while (numBytesDumped < numBytesPerLine) {
                if (numBytesDumped++ < numBytesThisLine)
                    sprintf (out, " %3.3o", *buf++) ;
                else
                    sprintf (out, " %3s", " ") ;
                out = out + strlen (out) ;
            }
            break ;

/* Base 10 - display the contents of each byte as a decimal number. */

        case MeoDecimal:
            while (numBytesDumped < numBytesPerLine) {
                if (numBytesDumped++ < numBytesThisLine)
                    sprintf (out, " %3u", *buf++) ;
                else
                    sprintf (out, " %3s", " ") ;
                out = out + strlen (out) ;
            }
            break ;

/* Base 16 - display the contents of each integer as a hexadecimal number. */

        case MeoHexadecimal:
            while (numBytesDumped < numBytesPerLine) {
                if (numBytesDumped < numBytesThisLine) {
#ifndef USE_SPRINTF
                    number = (unsigned long) *buf++ << 24 ;
                    number |= (unsigned long) *buf++ << 16 ;
                    number |= (unsigned long) *buf++ << 8 ;
                    number |= (unsigned long) *buf++ ;
                    sprintf (out, " %08lX", number) ;
                    out += 9 ;
#else
                    const  char  hexToASCII[] = "0123456789ABCDEF" ;
                    *out++ = ' ' ;
                    *out++ = hexToASCII[*buf >> 4] ;
                    *out++ = hexToASCII[*buf++ && 0x0F] ;
                    *out++ = hexToASCII[*buf >> 4] ;
                    *out++ = hexToASCII[*buf++ && 0x0F] ;
                    *out++ = hexToASCII[*buf >> 4] ;
                    *out++ = hexToASCII[*buf++ && 0x0F] ;
                    *out++ = hexToASCII[*buf >> 4] ;
                    *out++ = hexToASCII[*buf++ && 0x0F] ;
                    *out = '\0' ;
#endif
                } else {
                    strcat (out, "         ") ;
                    out += 9 ;
                }
                numBytesDumped = numBytesDumped + 4 ;
            }
            break ;

/* "Base 26" - treat the data as ASCII text. */

        case MeoText:
        default:
            break ;

        }


/* Append the ASCII version of the buffer. */

        if (base != MeoText) {
            *out++ = ' ' ;  *out++ = ' ' ;
        }
        *out++ = '"' ;
        (void) memcpy (out, buffer, numBytesThisLine) ;
        numBytesDumped = 0 ;
        while (numBytesDumped++ < numBytesThisLine) {
            if (isascii ((unsigned char) (*out)) &&
                isprint ((unsigned char) (*out))) {
                c = *out ;  *out++ = c ;
            } else {
                *out++ = '.' ;
            }
        }
        *out++ = '"' ;  *out++ = '\0' ;


/* Output the dump line to the file. */

#ifdef __palmos__
        HostFPrintF (HostLogFile (), "%s\n", outbuf) ;
#else
        fprintf (file, "%s\n", outbuf) ;
#endif


/* Adjust the pointers and counters for the next line. */

        address = (char *) address + numBytesThisLine ;
        buffer = (char *) buffer + numBytesThisLine ;
        numBytesToDump -= numBytesThisLine ;

    }


/* Deallocate the temporary input/output buffers. */

    free (inbuf) ;
    free (outbuf) ;


    return (0) ;

}

/*!*****************************************************************************

Procedure:

    meoLoad ()

    Load Memory from a File.


Purpose:

    Function meoLoad() loads the binary contents of a memory region from a
    disk file.  The contents must have been previously saved using meoSave().


    Invocation (dynamically allocated buffer):

        void  *startAddress = NULL ;
        ...
        status = meoLoad (fileName, offset, &startAddress, &numBytes) ;

    Invocation (caller-specified buffer):

        void  *startAddress = buffer ;
        long  numBytes = sizeof buffer ;
        ...
        status = meoLoad (fileName, offset, &startAddress, &numBytes) ;

    where:

        <fileName>	- I
            is the name of the file from which the memory contents will be
            loaded.  Environment variables may be embedded in the file name.
        <offset>	- I
            is the byte offset within the file from which the load will begin.
        <startAddress>	- I/O
            is the address of a (VOID *) pointer that specifies or returns the
            address where the contents of the file will be stored.  If the
            (VOID *) pointer is NULL, meoLoad() will MALLOC(3) a buffer for
            the file contents and return its address through this argument;
            the caller is responsible for FREE(3)ing the memory when it is
            no longer needed.  If the (VOID *) pointer is NOT NULL, meoLoad()
            uses it as the address of a caller-allocated buffer in which the
            file contents are to be stored; the size of the buffer is
            specified by the NUMBYTES argument.
        <numBytes>	- I/O
            is the address of a longword that specifies or returns the size of
            the memory buffer.  If the memory buffer is dynamically  allocated
            by meoLoad(), this argument returns the size of the buffer.  If
            meoLoad() uses a caller-allocated buffer, this argument specifies
            the size of the buffer.
        <status>	- O
            returns the status of loading the memory from a file, zero
            if there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  meoLoad (

#    if PROTOTYPES
        const  char  *fileName,
        long  offset,
        void  **startAddress,
        long  *numBytes)
#    else
        fileName, offset, startAddress, numBytes)

        char  *fileName ;
        long  offset ;
        void  **startAddress ;
        long  *numBytes ;
#    endif

{    /* Local variables. */
    FILE  *file ;
    long  fileSize ;
#if !defined(HAVE_STAT_H) || HAVE_STAT_H
    struct  stat  info ;
#endif



/* Open the input file. */

    fileName = fnmBuild (FnmPath, fileName, NULL) ;
    file = fopen (fileName, "rb") ;
    if (file == NULL) {
        LGE "(meoLoad) Error opening %s.\n", fileName) ;
        return (errno) ;
    }

/* Determine the amount of data to be loaded from the file. */

#if defined(HAVE_STAT_H) && !HAVE_STAT_H
#    warning  MeoLoad: No fstat(2); file size defaults to 1024.
    fileSize = 1024 ;			/* Until I figure out another way ... */
#else
    if (fstat (fileno (file), &info)) {
        LGE "(meoLoad) Error determining the size of %s.\n", fileName) ;
        return (errno) ;
    }

    fileSize = info.st_size - offset ;
#endif

/* Allocate a memory buffer, if necessary. */

    if (*startAddress == NULL) {
        *numBytes = fileSize ;
        *startAddress = malloc (fileSize) ;
        if (*startAddress == NULL) {
            LGE "(meoLoad) Error allocating %ld-byte memory buffer for %s.\n",
                fileSize, fileName) ;
            return (errno) ;
        }
    }

/* Read the (possibly truncated) contents of the file into the memory pool. */

#if defined(HAVE_FSEEK) && !HAVE_FSEEK
#    warning  MeoLoad: No fseek(2); offset ignored.
#else
    if (fseek (file, offset, SEEK_SET) != offset) {
        LGE "(meoLoad) Error positioning to offset %ld in %s.\nfseek: ",
            offset, fileName) ;
        return (errno) ;
    }
#endif

    if (fileSize > *numBytes)  fileSize = *numBytes ;

    if (fread (*startAddress, fileSize, 1, file) != 1) {
        LGE "(meoLoad) Error loading %ld bytes from %s to %p.\nfread: ",
            fileSize, fileName, *startAddress) ;
        return (errno) ;
    }

/* Close the file. */

    fclose (file) ;

    LGI "(meoLoad) Loaded %ld bytes from %s to %p.\n",
        fileSize, fileName, *startAddress) ;

    return (0) ;

}

/*!*****************************************************************************

Procedure:

    meoSave ()

    Save Memory to a File.


Purpose:

    Function meoSave() saves the binary contents of a memory region to a
    disk file.  The contents can be reloaded at a later time using meoLoad().


    Invocation:

        status = meoSave (startAddress, numBytes, fileName, offset) ;

    where:

        <startAddress>	- I
            specifies the start of the memory region that is to be saved.
        <numBytes>	- I
            specifies the number of bytes of data to save.
        <fileName>	- I
            is the name of the file to which the memory contents will be
            written.  Environment variables may be embedded in the file name.
        <offset>	- I
            is the byte offset within the file at which the save will begin.
        <status>	- O
            returns the status of saving the memory to a file, zero if
            there were no errors and ERRNO otherwise.

*******************************************************************************/


errno_t  meoSave (

#    if PROTOTYPES
        void  *startAddress,
        long  numBytes,
        const  char  *fileName,
        long  offset)
#    else
        startAddress, numBytes, fileName, offset)

        void  *startAddress ;
        long  numBytes ;
        char  *fileName ;
        long  offset ;
#    endif

{    /* Local variables. */
    FILE  *file ;



/* Open the output file. */

    fileName = fnmBuild (FnmPath, fileName, NULL) ;
    file = fopen (fileName, "wb") ;
    if (file == NULL) {
        LGE "(meoSave) Error opening %s to save memory at %p.\n",
            fileName, startAddress) ;
        return (errno) ;
    }

/* Write the contents of the memory out to the file. */

#if defined(HAVE_FSEEK) && !HAVE_FSEEK
#    warning  MeoSave: No fseek(2); offset ignored.
#else
    if (fseek (file, offset, SEEK_SET) != offset) {
        LGE "(meoSave) Error positioning to offset %ld in %s.\nfseek: ",
            offset, fileName) ;
        return (errno) ;
    }
#endif

    if (fwrite (startAddress, numBytes, 1, file) != 1) {
        LGE "(meoSave) Error saving %ld bytes at %p to %s.\n",
            numBytes, startAddress, fileName) ;
        return (errno) ;
    }

/* Close the file. */

    if (fclose (file)) {
        LGE "(meoSave) Error closing %s for memory at %p.\n",
            fileName, startAddress) ;
        return (errno) ;
    }

    LGI "(meoSave) Saved %ld bytes at %p to %s\n",
        numBytes, startAddress, fileName) ;

    return (0) ;

}

#ifdef  TEST

/*******************************************************************************

    Program to test the MEO_UTIL() functions.

    Under UNIX,
        compile and link as follows:
            % cc -g -DTEST meo_util.c -I<... includes ...> <libraries ...>
        and run with the following command line:
            % a.out <wildcard_file_spec>

    Under VxWorks,
        compile and link as follows:
            % cc -g -c -DTEST -DVXWORKS meo_util.c -I<... includes ...> \
                       -o test_drs.o
            % ld -r test_drs.o <libraries ...> -o test_drs.vx.o
        load as follows:
            -> ld <test_drs.vx.o
        and run with the following command line:
            -> test_drs.vx.o "<wildcard_file_spec>"

*******************************************************************************/

#ifdef VXWORKS

test_meo (fileName)
    char  *fileName ;
{    /* Local variables. */

#else

main (argc, argv)
    int  argc ;
    char  *argv[] ;
{    /* Local variables. */
    char  *fileName = argv[1] ;

#endif

    unsigned  char  buffer[256] ;
    long  i, numBytes ;
    void  *newBuffer ;




    meo_util_debug = 1 ;

    for (i = 0 ;  i < sizeof buffer ;  i++)
        buffer[i] = i ;

    meoSave (buffer, sizeof buffer, fileName, 0) ;

    newBuffer = NULL ;
    meoLoad (fileName, 0, &newBuffer, &numBytes) ;

    printf ("\nOctal Dump:\n\n") ;
    meoDumpO (NULL, "    ", 0, newBuffer, numBytes) ;
    printf ("\nDecimal Dump:\n\n") ;
    meoDumpD (NULL, "    ", 0, newBuffer, numBytes) ;
    printf ("\nHexadecimal Dump:\n\n") ;
    meoDumpX (NULL, "    ", 0, newBuffer, numBytes) ;
    printf ("\nText Dump:\n\n") ;
    meoDumpT (NULL, "    ", 0, newBuffer, numBytes) ;

}
#endif
