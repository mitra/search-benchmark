/* $Id: ffc.c,v 1.2 2005/02/12 00:44:18 alex Exp alex $ */
/*******************************************************************************

  ffc.c

  Format File in Columns.


    Invocation:

        % ffc [-bold] [-columns <number>] [-column_width <num_characters>]
              [-expand] [-form_feeds <yes|no|force>]
              [-number] [-output <file>]
              [-page_length <num_lines>] [-page_width <num_characters>]
              [-prolog <file>] [-top_margin <num_lines>] [-unexpand]
              [<input_file(s)>]

    where:

        "-bold"
            specifies bold page numbers.  This is essentially just the
            "-number" option, with the page header surrounded by the
            Wordstar control sequence (^B) for enabling/disabling bold
            print.
        "-columns <number>"
            specifies the number of columns (default = 2).
        "-column_width <num_characters>"
            specifies the width of a column; if not specfied, column
            width is determined based on the page width and the number
            of columns.
        "-expand"
            inhibits the expansion of input tabs to spaces; if this option
            is not present, FFC will pass the input through the UNIX
            "expand" filter to replace tabs by spaces (if you don't,
            columns 2-N in the output will probably have alignment
            problems).
        "-form_feeds <yes|no|force>"
            controls the use of form feeds in the output.  If this option
            is not specified or if "yes" is specified, form feeds are output
            as needed (i.e., for pages shorter than the maximum page length).
            If the "no" mode is specified, the appropriate number of blank
            lines are output to reach the bottom of the page.  If the "force"
            option is specified, form feeds are output between every page;
            to guarantee the presence of form feeds between pages, the
            number of lines output per page will be at most one less than
            the maximum number of lines per page (see the "-page_length"
            option below).
        "-number"
            invokes page numbering on output.  This uses up two lines
            at the top of the page: one line for the page number followed
            by one blank line.  Any header lines (see "-top_margin" option)
            precede these two lines.
        "-output <file>"
            specifies the name of the output file; if not specified, the
            output is directed to the standard output.
        "-page_length <num_lines>"
            specifies the number of lines per page (default = 66).  The
            number of input lines in one column of the output page is
            equal to the page length minus the number of header lines
            (see the "-number" and "-top_margin" options).
        "-page_width <num_characters>"
            specifies the width of the output page (default = 158).
        "-prolog <file>"
            specifies the name of a file whose contents (e.g., printer setup
            codes) will be prepended to the output.
        "-top_margin <num_lines>"
            specifies the number of blank lines at the top of each page
            (default = 0).  (Also see the "-number" option below.)
        "-unexpand"
            causes the "unexpansion" of output spaces to tabs; if this
            option is present, FFC will pass its output through the UNIX
            UNEXPAND(1) filter to replace spaces by tabs, resulting in a
            significant reduction in output volume.  If you go this route,
            be sure to also specify the "-form_feeds no" option.  Form feeds
            are treated UNEXPAND(1) as if they occupy one column of output;
            this wrong assumption throws off the tabbing of the line in
            which the form feed occurs.
        "<input_file>"
            is the file to be formatted in columns.  If this argument is a
            wildcard file specification, only the first file in the expanded
            list is formatted.  If this argument is not specified, input is
            taken from standard input.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <ctype.h>			/* Character functions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#ifdef vms
#    include  "vmsparam.h"		/* System parameters (VMS). */
#else
#    include  <sys/param.h>		/* System parameters. */
#endif
#include  "drs_util.h"			/* Directory scanning utilities. */
#include  "fnm_util.h"			/* Filename utilities. */
#include  "get_util.h"			/* "Get Next" functions. */
#include  "list_util.h"			/* List manipulation functions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "tv_util.h"			/* "timeval" manipulation functions. */
#include  "aperror.h"			/* APERROR() definitions. */


#define  MAX_COLUMNS  32
#define  MAX_FILES  1024
#define  MAX_STRING  256
#define  FORM_FEED  12

					/* Page/column data. */
static  char  *column[MAX_COLUMNS] ;
static  char  *header[MAX_COLUMNS] ;

					/* Page dimensions, etc. */
static  int  bold_page_numbers = 0 ;	/* 0 = no, 1 = yes. */
static  int  column_width = 0 ;		/* Computed from page width and #columns. */
static  int  expand_input_tabs = 1 ;	/* 0 = no, 1 = yes (normally required). */
static  int  expand_output_tabs = 0 ;	/* 0 = no, 1 = yes (reduces output volume). */
static  int  force_form_feeds = 0 ;	/* 0 = no, 1 = yes. */
static  int  num_columns = 2 ;
static  int  num_header_lines = 0 ;
static  int  page_length = 66 ;		/* Our laser printer in landscape mode. */
static  int  page_numbering = 0 ;	/* 0 = no, 1 = yes. */
static  int  page_width = 158 ;		/* Our laser printer in landscape mode. */
static  int  use_form_feeds = 1 ;	/* On output: 0 = no, 1 = yes. */

					/* Miscellaneous non-locals. */
static  char  input_file_spec[MAXPATHLEN] ;
static  int  current_page, debug ;


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  clear_columns (
#    if PROTOTYPES
        int  column_width,
        int  page_length,
        int  num_columns,
        char  *column[],
        char  *header[]
#    endif
    ) ;

static  void  read_column (
#    if PROTOTYPES
        FILE  *file,
        int  column_width,
        int  page_length,
        char  *column
#    endif
    ) ;

static  void  output_page (
#    if PROTOTYPES
        FILE  *file,
        int  column_width,
        int  page_length,
        int  num_columns,
        char  *column[],
        char  *header[]
#    endif
    ) ;

static  int  text_remaining (
#    if PROTOTYPES
        int  line,
        int  column_width,
        int  num_columns,
        char  *column[]
#    endif
    ) ;


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
    char  *arg, *argument, buffer[MAX_STRING] ;
    char  *name, *outputFile, *prologFile, *s ;
    DirectoryScan  directory ;
    int  col, errflg, option ;
    FILE  *infile, *outfile ;
    FileName  inputFile ;
    List  fileList ;
    OptContext  scan ;
    ssize_t  length ;

    const  char  *option_list[] = {	/* Command line options. */
        "{bold}", "{columns:}", "{column_width:}",
        "{debug}", "{expand}", "{form_feeds:}",
        "{number}", "{output:}", "{page_length:}",
        "{page_width:}", "{prolog:}",
        "{top_margin:}", "{unexpand}",
        NULL
    } ;





/*******************************************************************************
  Scan the command line options.
*******************************************************************************/

    debug = 0 ;  fileList = NULL ;  outputFile = NULL ;  prologFile = NULL ;

    opt_init (argc, argv, NULL, option_list, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-bold" */
            bold_page_numbers = 1 ;
            page_numbering = 1 ;
            break ;
        case 2:			/* "-columns <number>" */
            num_columns = atoi (argument) ;
            break ;
        case 3:			/* "-column_width <num_characters>" */
            column_width = atoi (argument) ;
            break ;
        case 4:			/* "-debug" */
            debug = 1 ;
            break ;
        case 5:			/* "-expand" */
            expand_input_tabs = 0 ;
            break ;
        case 6:			/* "-form_feeds <yes|no|force>" */
            if (strcmp (argument, "yes") == 0)
                use_form_feeds = 1 ;
            else if (strcmp (argument, "no") == 0)
                use_form_feeds = 0 ;
            else if (strcmp (argument, "force") == 0)
                force_form_feeds = 1 ;
            else
                errflg++ ;
            break ;
        case 7:			/* "-number" */
            page_numbering = 1 ;
            break ;
        case 8:			/* "-output <file>" */
            outputFile = argument ;
            break ;
        case 9:			/* "-page_length <num_lines>" */
            page_length = atoi (argument) ;
            break ;
        case 10:		/* "-page_width <num_characters>" */
            page_width = atoi (argument) ;
            break ;
        case 11:		/* "-prolog <file>" */
            prologFile = argument ;
            break ;
        case 12:		/* "-top_margin <num_lines>" */
            num_header_lines = atoi (argument) ;
            break ;
        case 13:		/* "-unexpand" */
            expand_output_tabs = 1 ;
            break ;
        case NONOPT:		/* Possibly a comma-separated list of files. */
            length = -1 ;  arg = argument ;
            while ((arg = getarg (arg, &length)) != NULL) {
                name = strndup (arg, length) ;
                drsCreate (name, &directory) ;
                while ((s = (char *) drsNext (directory)) != NULL) {
                    listAdd (&fileList, -1, (void *) fnmCreate (s, NULL)) ;
                }
                drsDestroy (directory) ;
                free (name) ;
            }
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  ffc  [<options>] [<input_file(s)>]\n") ;
        fprintf (stderr, "where <options> are:\n") ;
        fprintf (stderr, "        -bold\n") ;
        fprintf (stderr, "        -columns <number>\n") ;
        fprintf (stderr, "        -column_width <num_characters>\n") ;
        fprintf (stderr, "        -expand\n") ;
        fprintf (stderr, "        -form_feeds <yes|no|force>\n") ;
        fprintf (stderr, "        -number\n") ;
        fprintf (stderr, "        -output <file>\n") ;
        fprintf (stderr, "        -page_length <num_lines>\n") ;
        fprintf (stderr, "        -page_width <num_characters>\n") ;
        fprintf (stderr, "        -prolog <file>\n") ;
        fprintf (stderr, "        -top_margin <num_lines>\n") ;
        fprintf (stderr, "        -unexpand\n") ;
        exit (EINVAL) ;
    }


/* Set the various width and length parameters. */

    if (column_width <= 0)  column_width = (page_width / num_columns) - 1 ;

    page_length = page_length - num_header_lines ;
    if (page_numbering)  page_length = page_length - 2 ;

    if (fileList == NULL)	/* If no input files, use standard input. */
        listAdd (&fileList, -1, (void *) strdup ("")) ;

#ifdef vms
    expand_output_tabs = 0 ;			/* Don't POPEN(3)! */
#endif

/*******************************************************************************
  Allocate memory to store the columns on a page.
*******************************************************************************/

    for (col = 0 ;  col < num_columns ;  col++) {
        column[col] = malloc (column_width * page_length + 1) ;
        if (column[col] == NULL) {
            fprintf (stderr, "Error allocating memory for columns.\n") ;
            perror ("malloc") ;  exit (errno) ;
        }
        header[col] = malloc (column_width + 1) ;
        if (header[col] == NULL) {
            fprintf (stderr, "Error allocating memory for headers.\n") ;
            perror ("malloc") ;  exit (errno) ;
        }
    }


/*******************************************************************************
    Open the output file.  If no output file name is specified, the output is
    written to standard output.
*******************************************************************************/

    if (outputFile == NULL) {
        outfile = stdout ;
    } else {
        outfile = fopen (outputFile, "w") ;
        if (outfile == NULL) {
            fprintf (stderr, "Error opening output file: %s\n", outputFile) ;
            perror ("fopen") ;  exit (errno) ;
        }
    }


/*******************************************************************************
    If a prolog file was specified, copy its contents to the output file.
*******************************************************************************/

    if (prologFile != NULL) {

        FILE  *profile ;

        profile = fopen (prologFile, "r") ;
        if (profile == NULL) {
            fprintf (stderr, "Error opening prolog file: %s\n", prologFile) ;
            perror ("fopen") ;  exit (errno) ;
        }

        for ( ; ; ) {
            length = fread (buffer, 1, sizeof buffer, profile) ;
            if (length <= 0)  break ;
            fwrite (buffer, 1, length, outfile) ;
        }

        fclose (profile) ;

    }

/*******************************************************************************
  For each input file, read and output the file formatted in columns.
*******************************************************************************/

    clear_columns (column_width, page_length, num_columns, column, header) ;
    col = 0 ;

    while (fileList != NULL) {

        inputFile = listDelete (&fileList, 1) ;
        if (debug)  fprintf (stderr, "%s\n", (inputFile == NULL) ?
                             "<stdin>" : fnmPath (inputFile)) ;

        current_page = 1 ;


/*******************************************************************************
    Open the input file.
*******************************************************************************/

        if (inputFile == NULL) {
            infile = stdin ;	/* Get input from the standard input. */
        } else {
            infile = fopen (fnmPath (inputFile), "r") ;
            if (infile == NULL) {
                fprintf (stderr, "Error opening input file \"%s\": ",
                         fnmPath (inputFile)) ;
                perror ("") ;  exit (errno) ;
            }
        }


/*******************************************************************************
    Read pages from the input file, place them in columns on the output page,
    and output the output pages.
*******************************************************************************/

        strcpy (input_file_spec, fnmFile (inputFile)) ;

        while (!feof (infile)) {

            if (col < num_columns) {
                read_column (infile, column_width,
                             force_form_feeds ? (page_length - 1) : page_length,
                             column[col]) ;
                if (current_page == 1)
                    sprintf (buffer, "%s-- Page %d --  (%s)  %s%s",
                             (bold_page_numbers ? "\002" : ""),
                             current_page++, input_file_spec, tvShow (tvTOD (), 1, "%C"),
                             (bold_page_numbers ? "\002" : "")) ;
                else
                    sprintf (buffer, "%s-- Page %d --  (%s)%s",
                             (bold_page_numbers ? "\002" : ""),
                             current_page++, input_file_spec,
                             (bold_page_numbers ? "\002" : "")) ;
                strncpy (header[col], buffer, column_width) ;
                *(header[col] + column_width) = '\0' ;
                col++ ;
            } else {
                output_page (outfile, column_width, page_length, num_columns,
                             column, header) ;
                clear_columns (column_width, page_length, num_columns,
                               column, header) ;
                col = 0 ;
            }

        }


/* Close the input file. */

        fclose (infile) ;		/* File. */


    }     /* For each input file. */


/*******************************************************************************
    Flush any remaining text.
*******************************************************************************/

    if (col > 0)
        output_page (outfile, column_width, page_length, num_columns,
                     column, header) ;

    exit (0) ;

}

/*******************************************************************************

  CLEAR_COLUMNS - clears (blank-fills) the columns in the output page.  A null-
    terminator marks the end of each column.

*******************************************************************************/


static  void  clear_columns (

#    if PROTOTYPES
        int  column_width,
        int  page_length,
        int  num_columns,
        char  *column[],
        char  *header[])
#    else
        column_width, page_length, num_columns, column, header)

        int  column_width, num_columns, page_length ;
        char  *column[], *header[] ;
#    endif

{
    int  col ;

    for (col = 0 ;  col < num_columns ;  col++) {
        memset (column[col], ' ', column_width*page_length) ;
        *(column[col]+(column_width*page_length)) = '\0' ;
        strcpy (header[col], "") ;
    }
}

/*******************************************************************************

  READ_COLUMN - reads the next page from a file and stores it in a column
    matrix.  The new line character is stripped from each input line and the
    line is stored in the matrix WITHOUT a null terminator.  The next page stops
    after PAGE_LENGTH lines or when a form feed is encountered.

*******************************************************************************/


static  void  read_column (

#    if PROTOTYPES
        FILE  *file,
        int  column_width,
        int  page_length,
        char  *column)
#    else
        file, column_width, page_length, column)

        FILE  *file ;
        int  column_width, page_length ;
        char  *column ;
#    endif

{    /* Local variables. */
    char  inbuf[512], *nl ;
    int  length, line ;


    memset (inbuf, 0, sizeof inbuf) ;

    line = 0 ;
    while (line++ < page_length) {
        if (fgets (inbuf, (sizeof inbuf), file) == NULL)
            line = page_length ;
        else if (strchr (inbuf, FORM_FEED) != NULL)
            line = page_length ;
        else {
            if ((nl = strchr (inbuf, '\n')) != NULL)  *nl = '\0' ;
            length = strlen (inbuf) ;
            if (expand_input_tabs)
                length = strDetab (inbuf, length, 0, NULL, sizeof inbuf) ;
            if (length > column_width)  length = column_width ;
            memcpy (column, inbuf, length) ;
            column = column + column_width ;
        }
    }

}

/*******************************************************************************

  OUTPUT_PAGE - formats columns on an output page and outputs the page to a
    file.

*******************************************************************************/


static  void  output_page (

#    if PROTOTYPES
        FILE  *file,
        int  column_width,
        int  page_length,
        int  num_columns,
        char  *column[],
        char  *header[])
#    else
        file, column_width, page_length, num_columns, column, header)

        FILE  *file ;
        int  column_width, num_columns, page_length ;
        char  *column[], *header[] ;
#    endif

{    /* Local variables. */
    char  *outbuf, *p ;
    int  col, line ;
    static  int  trailing_lines = 0 ;




    if (trailing_lines > 0) {	/* Trailing blank lines from previous page? */
        if (use_form_feeds) {
            fprintf (file, "%c", FORM_FEED) ;
            trailing_lines = 0 ;
        } else {
            while (trailing_lines-- > 0)
                fprintf (file, "\n") ;
        }
    }

    outbuf = malloc (num_columns * (column_width + 1) + 1) ;

    for (line = 0 ;  line < num_header_lines ;  line++)
        fprintf (file, "\n") ;

    if (page_numbering) {
        for (p = outbuf, col = 0 ;  col < num_columns ;  col++) {
            memset (p, ' ', column_width) ;
            strcpy (p, header[col]) ;
            *(p + strlen (p)) = ' ' ;		/* Erase '\0' from copy. */
            p = p + column_width ;
            *p++ = ' ' ;
        }
        *p = '\0' ;
        strTrim (outbuf, -1) ;			/* Trim trailing blanks. */
        fprintf (file, "%s\n", outbuf) ;
        fprintf (file, "\n") ;
    }

    for (line = 0 ;  line < page_length ;  line++) {
        if (text_remaining (line, column_width, num_columns, column)) {
            p = outbuf ;
            for (col = 0 ;  col < num_columns ;  col++) {
                memcpy (p, column[col] + (line*column_width), column_width) ;
                p = p + column_width ;
                *p++ = ' ' ;
            }
            *p = '\0' ;
            strTrim (outbuf, -1) ;		/* Trim trailing blanks. */
            fprintf (file, "%s\n", outbuf) ;
        } else {
            break ;
        }
    }

					/* Print form-feed next time called? */
    trailing_lines = page_length - line ;

    free (outbuf) ;

}

/*******************************************************************************

  TEXT_REMAINING - returns TRUE if lines of text remain in any of the columns.
    Note that the end of each column is assumed to be null-terminated (see
    CLEAR_COLUMN).

*******************************************************************************/


static  int  text_remaining (

#    if PROTOTYPES
        int  line,
        int  column_width,
        int  num_columns,
        char  *column[])
#    else
        line, column_width, num_columns, column)

        int  column_width, line, num_columns ;
        char  *column[] ;
#endif

{    /* Local variables. */
    char  *p ;
    int  col ;


    for (col = 0 ;  col < num_columns ;  col++) {
        p = column[col] + (line*column_width) ;
        if (strspn (p, " ") != strlen (p))  return (-1) ;	/* Text! */
    }
    return (0) ;						/* No text. */
}
