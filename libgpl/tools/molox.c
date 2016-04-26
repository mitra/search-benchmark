/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/**************************************************************************

Process:

    molox

    Module Lines-of-Code Counter.


Author:    Alex Measday, ISI


Purpose:

    Program MOLOX counts lines of C code and lines of PDL in a file,
    on a per-module basis.  For each of the input C source files,
    MOLOX first runs the source file through the UNIX CTAGS(1) utility,
    which outputs the starting line number of each module in the source
    file.  MOLOX then scans the input file, counts the number of lines
    of code and PDL in each module, and outputs the line counts for
    each module.  For example, the output for the MOLOX program looks
    as follows:
                                  C    CDS     PDL   PDS
                                ----- -----   ----- -----
        molox.c:                  28                            (248)
            main                 199    78      31     8
            remove_comments       21     8       1     0

        Summary - # of files: 1, # of modules: 2, # of lines: 248

    The first line following the column headings contains the source file
    name.  The number "28" under the "C" column is the number of lines of
    C code that precede the first module in the file; e.g., global data
    declarations, etc.  The figure in parentheses, "(248)", is the total
    lines of C code in the entire file.  Below the file name is an indented
    list of the modules found in the source file; in this case, there are
    only two, "main" and "remove_comments".  There are 4 counts given for
    each module: the number of lines of C code in the module, the number
    of C decision statements (IF statements, etc.), the number of lines
    of PDL for the module, and the number of PDL decision statements.

    After all the input source files have been processed, MOLOX outputs a
    summary line totaling the number of files processed, the modules, and
    the lines of code.

    MOLOX considers any line containing a semi-colon (";") or left brace
    ("{") as a line of C code; in addition, C Pre-Processor directives are
    counted.  Special handling is used to produce accurate counts for LEX
    and YACC files.  MOLOX is not perfect:

        (1) Multiple statements on one line are treated as a single
            statement.

        (2) Comments are recognized and ignored, but MOLOX may be fooled
            by comment delimiters appearing inside a C string.

    It's not perfect, but what's your alternative?! :)  In any case, the
    program seems pretty accurate - the undercounts and overcounts seem to
    balance out.


    Invocation:

        % molox [-d] [-h] [-l] [-L] [-s] [-S] [-v] [source_file(s)]

    where

        "-d"
            enables debug output (written to STDERR).
        "-h"
            inhibits the output of column headings for the lines-of-code counts.
        "-l"
            specifies that long module names will be encountered.  MOLOX will
            shift the columns of numbers to the right by one tab stop so that
            the columns line up (more or less).
        "-L"
            is like the "-l" option, except that the columns of number are
            shifted TWO tab stops to the right.
        "-s"
            causes MOLOX to generate CFLOW(1)-readable output (written to
            STDOUT).  The output can be run through CFLOW to construct the
            calling hierarchy for the program.  The hierarchy is based on
            the CALL statements in the PDL.
        "-S"
            causes MOLOX to generate the calling hierarchy in STRUCHART
            intermediate form.  The hierarchy is based on the CALL statements
            in the PDL; the output is written to STDOUT.
        "-v"
            enables verbose mode.  In this mode, MOLOX displays (to STDERR)
            the name of each file as the file is processed.  This is useful
            when you have redirected the module/count output (STDOUT) to a
            file.
        "<source_file(s)>"
            is the list of C, LEX, and/or YACC source files to read.  VMS-style
            "sticky defaults" are implemented, so a fully-expanded source file
            argument supplies defaults for missing pathname components in the
            source file argument that follows.  If a directory is specified
            (indicated by a trailing "/" in the file name), MOLOX automatically
            scans all ".c", ".h", ".l", ".x", and ".y" files in the directory.


File/Record References:
Name                      Use    Description
----                      ---    -----------
C Source File(s)         Input

System Variables:
Mnemonic name      Process name        Use   Description
-------------      ------------        ---   -----------

Notes:

**************************************************************************/

/*PDL-------------------------PDL------------------------PDL**

DOFOR each input source file
    Run CTAGS(1) on the file.
    Read CTAGS(1) output and extract starting line numbers for each module.
    Set all module line-of-code counts to zero.
    DOWHILE not EOF on the input file
        Read next line from file.
        CALL REMOVE_COMMENTS to remove C Language comments.
        Determine which module contains this line.
        IF line is a line of C code THEN
            Increment module's lines-of-code count.
        ENDIF
        IF line is a C decision statement THEN
            Increment module's C-decisions count.
        ENDIF
    ENDDO
    Rewind the input file.
    DOWHILE not EOF on the input file
        Read next line from file.
        Determine which module contains this line.
        IF line is a line of PDL THEN
            Increment module's lines-of-PDL count.
        ENDIF
        IF line is a PDL decision statement THEN
            Increment module's PDL-decisions count.
        ENDIF
    ENDDO
    DOFOR each module in the input file
        Output module's statistics.
    ENDDO
ENDDO

Output summary statistics.

**PDL-------------------------PDL------------------------PDL*/

#ifdef sccs
    static char sccsid[] = "File: molox.c  Release: 1.10  Date: 11/13/91, 10:10:39" ;
#endif

#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "libutilgen.h"		/* LIBUTILGEN definitions. */
#include  "fparse.h"			/* Filename parsing definitions. */
#include  "list_util.h"			/* List manipulation functions. */
#include  "opt_util.h"			/* Option scanning definitions. */


#define  MAX_INPUT  (4*1024)


/*******************************************************************************
    Module list - is a sorted list of the modules in a single source file.
        The very first node in the list is for the "zeroth" module in the
        file, i.e., those lines of code (global declarations, etc.) preceding
        the first module in the file.  The remaining modules in the list are
        sorted by their location in the source file.
*******************************************************************************/

    typedef  struct  module {
        char  *name ;			/* Module name. */
        int  first_line ;		/* Line numbers in file. */
        int  last_line ;
        int  loc_count ;		/* Number of lines of code. */
        int  loc_ds ;			/* Number of decision statements in code. */
        int  pdl_count ;		/* Number of lines of PDL. */
        int  pdl_ds ;			/* Number of decision statements in PDL. */
    }  module ;


/*******************************************************************************
    File Extensions Table - specifies the types of files MOLOX looks at when
        the user only specifies a directory name; i.e., MOLOX will scan all
        the C files in the directory, all the header files, etc.
*******************************************************************************/

    static  char  *extension[] = {	/* Table of file extensions. */
        "*.c", "*.h", "*.l", "*.x", "*.y", NULL
    } ;


/*******************************************************************************
    LOC Keywords Table - is a list of the C keywords that introduce decision
        statements.
*******************************************************************************/

    static  char  *loc_keyword[] = {	/* Table of C decision statement keywords. */
        "case", "default", "do", "else",
        "for", "if", "while", NULL
    } ;


/*******************************************************************************
    PDL Keywords Table - is a list of the PDL keywords that introduce decision
        statements.
*******************************************************************************/

    static  char  *pdl_keyword[] = {	/* Table of PDL decision statement keywords. */
        "CASE", "DO", "DOFOR", "DOUNTIL", "DOWHILE",
        "ELSE", "ELSEIF", "IF", NULL
    } ;

/*******************************************************************************
    MOLOX Main Program.
*******************************************************************************/

main (argc, argv)

    int  argc ;
    char  *argv[] ;

{  /* Local variables. */
    char  buffer[MAX_INPUT], *file_name, *s, *sticky_defaults ;
    char  *tabs = "\t\t\t\t\t" ;
    FILE  *infile ;
    int  cflow, count, debug, errflg, first_call ;
    int  heading, i, in_comment, in_pdl ;
    int  length, line_number, line_of_pdl, long_names ;
    int  num_files, num_modules, option ;
    int  struchart ;
    int  total, total_all_files, verbose ;
    List  file_list, module_list ;
    module  *m ;





/*******************************************************************************
  Scan the command line options.
*******************************************************************************/

    debug = 0 ;  file_list = NULL ;
    sticky_defaults = str_dupl ("", -1) ;
    heading = 1 ;  long_names = 0 ;
    struchart = 0 ;  cflow = 0 ;
    verbose = 0 ;  vperror_print = 1 ;
    errflg = 0 ;

    while (((option = getopt (argc, argv, "dhlLsSv")) != NONOPT) ||
           (optarg != NULL)) {
        switch (option) {
        case 'd':  debug = 1 ;  break ;
        case 'h':  heading = 0 ;  break ;
        case 'l':  long_names = 1 ;  break ;
        case 'L':  long_names = 2 ;  break ;
        case 's':  struchart = 1 ;  cflow = 1 ;  break ;
        case 'S':  struchart = 1 ;  break ;
        case 'v':  verbose = 1 ;  break ;
        case '?':  errflg++ ;  break ;
        case NONOPT:
            if (strcmp (optarg, ".") == 0)
                s = fparse ("./", sticky_defaults, NULL, ALL) ;
            else
                s = fparse (optarg, sticky_defaults, NULL, ALL) ;
            str_free (&sticky_defaults, -1) ;
            sticky_defaults = str_dupl (s, -1) ;
            i = strlen (sticky_defaults) - 1 ;
            if (sticky_defaults[i] == '/') {
                for (i = 0 ;  extension[i] != NULL ;  i++) {
                    while ((s = fsearch (extension[i], sticky_defaults,
                                         NULL, NULL)) != NULL) {
                        list_add (&file_list, -1, (void *) str_dupl (s, -1)) ;
                    }
                }
            } else {
                while ((s = fsearch (sticky_defaults, NULL, NULL)) != NULL) {
                    list_add (&file_list, -1, (void *) str_dupl (s, -1)) ;
                }
            }
            break ;
        default :  break ;
        }
    }

    str_free (&sticky_defaults, -1) ;

    if (errflg) {
        fprintf (stderr,
                 "Usage:  molox [-d] [-h] [-l] [-L] [-s] [-S] [-v] source_file(s)\n") ;
        exit (-1) ;
    }

/*******************************************************************************
    For each source file, determine the module boundaries and count the lines
    of code in each module.
*******************************************************************************/

    num_files = list_length (file_list) ;
    num_modules = 0 ;  total_all_files = 0 ;
    module_list = NULL ;

    if (heading && (num_files > 0) && !struchart) {
        length = (25 + (long_names * 8) - 1 + 7) / 8 ;
        printf ("%.*s  C    CDS     PDL   PDS\n", length, tabs) ;
        printf ("%.*s----- -----   ----- -----\n", length, tabs) ;
    }

    for ( ; ; ) {

        file_name = (char *) list_delete (&file_list, 1) ;
        if (file_name == NULL)  break ;

        if (verbose)  fprintf (stderr, "%s\n", file_name) ;

/* Let CTAGS(1) parse the source file and determine the starting line number
   of each module in the file.  I originally piped the output of CTAGS(1)
   through SORT(1).  Unfortunately, CTAGS(1) runs extra-long module names
   flush up against their line numbers, so MOLOX is forced to sort the
   modules itself. */

        sprintf (buffer, "ctags -wx %s", file_name) ;
        infile = popen (buffer, "r") ;
        if (infile == NULL) {
            vperror ("(molox) Error piping %s through CTAGS.\npopen: ", file_name) ;
            exit (errno) ;
        }

/* Construct a list of the modules in the file. */

        while (list_length (module_list) > 0) {		/* Delete old list. */
            m = list_delete (&module_list, 1) ;
            str_free (&m->name, -1) ;  free ((char *) m) ;
        }

        while (fgets (buffer, sizeof buffer, infile) != NULL) {
            length = strlen (buffer) ;
            if ((length > 0) && (buffer[length-1] == '\n')) {
                buffer[length-1] = '\0' ;
                str_trim (buffer, -1) ;
            }
            length = 0 ;
            s = getword (buffer, " \t", &length) ;	/* Module name. */
            if (length <= 0)  continue ;
            m = (module *) malloc (sizeof (module)) ;
            if (m == NULL) {
                vperror ("(molox) Error allocating module node for \"%s\".\nmalloc: ", s) ;
                exit (errno) ;
            }
            m->name = str_dupl (s, length) ;
            s = getword (s, " \t", &length) ;		/* Line number. */
            m->first_line = atoi (s) ;
            if (m->first_line <= 0) {
			/* If an extra-long module name was run flush up
			   against the line number, then backtrack and
			   correctly extract the two items of information. */
                length = 0 ;  s = getword (buffer, " \t", &length) ;
                while (isdigit (s[length-1]))
                    length-- ;
                m->first_line = atoi (&s[length]) ;
                if (m->first_line > 0) {
                    str_free (&m->name, -1) ;
                    m->name = str_dupl (s, length) ;
                    s = getword (s, " \t", &length) ;
                } else {
                    vperror ("(molox) Unrecognizable line: \"%s\"\n", buffer) ;
                    str_free (&m->name, -1) ;  free ((char *) m) ;
                    continue ;
                }
            }
            if (debug)  fprintf (stderr, "(molox) Line: %d\tModule: %s\n",
                                 m->first_line, m->name) ;
            s = getword (s, " \t", &length) ;		/* File name. */
            s = getword (s, " \t", &length) ;		/* Declaration. */
            if (*s == '#') {				/* CPP directive? */
                if (debug)  fprintf (stderr, "(molox) Ignoring CPP directive: %s\n", s) ;
                str_free (&m->name, -1) ;  free ((char *) m) ;
                continue ;
            } else if (strchr (s, ':') != NULL) {	/* YACC grammar rule? */
                if (debug)  fprintf (stderr, "(molox) Ignoring YACC grammar rule: %s\n", s) ;
                str_free (&m->name, -1) ;  free ((char *) m) ;
                continue ;
            }
            m->loc_count = 0 ;  m->loc_ds = 0 ;
            m->pdl_count = 0 ;  m->pdl_ds = 0 ;
		/* Insert module in list, sorted by starting line numbers. */
            for (i = list_length (module_list) ;  i > 0 ;  i--) {
                if (((module *) list_get (module_list, i))->first_line
                    < m->first_line)  break ;
            }
            list_add (&module_list, i, (void *) m) ;
        }

        pclose (infile) ;

/* Insert a dummy module at the beginning of the module list.  This node
   accumulates counts that precede first module in the file. */

        m = (module *) malloc (sizeof (module)) ;
        if (m == NULL) {
            vperror ("(molox) Error allocating module node for source preceding first module.\nmalloc: ") ;
            exit (errno) ;
        }
        m->name = str_dupl ("-- declarations --", -1) ;
        m->first_line = 0 ;
        m->loc_count = 0 ;  m->pdl_count = 0 ;
        list_add (&module_list, 0, (void *) m) ;

/* Compute the last line of each module.  The last line of a module is simply
   one less than the first line of the following module. */

        line_number = 999999 ;
        for (i = list_length (module_list) ;  i > 0 ;  i--) {
            m = (module *) list_get (module_list, i) ;
            m->last_line = line_number ;
            line_number = m->first_line - 1 ;
        }

        if (debug) {
            for (i = 0 ;  i < list_length (module_list) ;  i++) {
                m = (module *) list_get (module_list, i+1) ;
                printf ("(molox) Module %s:\tLines %d - %d\n",
                        m->name, m->first_line, m->last_line) ;
            }
        }

/*******************************************************************************
    On the first pass over the input file, process the C code.  Comments are
    deleted from the source text and the number of lines of C code and the
    number of C decision statements (IFs, ELSEs, etc.) are counted.
*******************************************************************************/

        if (open_input_file (file_name, &infile, &s)) {
            vperror ("(molox) Error opening source file: %s\nfopen: ", file_name) ;
            continue ;
        }

        count = 0 ;  total = 0 ;  in_comment = 0 ;
        line_number = 0 ;

        while (fgets (buffer, sizeof buffer, infile) != NULL) {

            line_number++ ;
            length = strlen (buffer) ;
            if ((length > 0) && (buffer[length-1] == '\n')) {
                buffer[length-1] = '\0' ;
                length = str_trim (buffer, -1) ;
            }
            remove_comments (buffer, &in_comment) ;
            if (strlen (buffer) == 0)  continue ;

            i = 0 ;  m = (module *) list_get (module_list, ++i) ;
            while ((m != NULL) && (line_number > m->last_line))
                m = (module *) list_get (module_list, ++i) ;
            if (m == NULL)  continue ;

/* Is the current line a line of C code? */

            if ((buffer[0] == '#') ||		/* CPP directive? */
                (buffer[0] == '%') ||		/* LEX/YACC directive? */
                strchr (buffer, ';') ||		/* C statement terminator? */
                strchr (buffer, '{')) {		/* C block initiator? */
                m->loc_count++ ;  total++ ;
            }

/* Is the current line a C decision statement (IF, ELSE, FOR, etc.)? */

            length = 0 ;  s = getword (buffer, " \t({:}", &length) ;
            if (length > 0) {
                for (i = 0 ;  loc_keyword[i] != NULL ;  i++) {
                    if ((length == strlen (loc_keyword[i])) &&
                        (strncmp (s, loc_keyword[i], length) == 0))
                        break ;
                }
                if (loc_keyword[i] != NULL)  m->loc_ds++ ;
            }

        }     /* While not end-of-file */

/*******************************************************************************
    On the second pass over the input file, count the number of lines of PDL
    and the number of PDL decision statements (IFs, ELSEs, etc.).  (Merging
    the first and second passes into a single pass over the input file did
    NOT improve the speed of MOLOX; the limiting factor is apparently the
    spawning of the CTAGS command for each input file.)
*******************************************************************************/

        rewind (infile) ;	/* Reset input file to the beginning. */

        in_comment = 0 ;  in_pdl = 0 ;  line_number = 0 ;

        while (fgets (buffer, sizeof buffer, infile) != NULL) {

            line_number++ ;
            length = strlen (buffer) ;
            if ((length > 0) && (buffer[length-1] == '\n'))
                buffer[length-1] = '\0' ;
            length = str_trim (buffer, -1) ;

/* Is the current line a line of PDL? */

            if (in_pdl) {
                if (str_index (buffer, -1, "PDL*/") != NULL) {
                    in_pdl = 0 ;
                    if (cflow && !first_call)  printf ("}\n") ;
                }
                line_of_pdl = in_pdl && (length > 0) ;
            } else {
                in_pdl = (length > 5) && (strncmp (buffer, "/*PDL", 5) == 0) ;
                if (in_pdl)  first_call = 1 ;
                line_of_pdl = 0 ;
            }
            if (line_of_pdl) {
                i = 0 ;  m = (module *) list_get (module_list, ++i) ;
                while ((m != NULL) && (line_number >= m->first_line))
                    m = (module *) list_get (module_list, ++i) ;
                if (m == NULL)  continue ;
                m->pdl_count++ ;
            }

/* Is the current line a PDL decision statement (IF, ELSE, FOR, etc.)? */

            if (line_of_pdl) {
                length = 0 ;  s = getword (buffer, " \t", &length) ;
                if (length > 0) {
                    for (i = 0 ;  pdl_keyword[i] != NULL ;  i++) {
                        if ((length == strlen (pdl_keyword[i])) &&
                            (strncmp (s, pdl_keyword[i], length) == 0))
                            break ;
                    }
                    if (pdl_keyword[i] != NULL)  m->pdl_ds++ ;
                    if (struchart && (strncmp (s, "CALL", 4) == 0)){
                        if (first_call) {
                            if (cflow)
                                printf ("%s () {\n", m->name) ;
                            else
                                printf ("ROUTINE %s CALLS\n", m->name) ;
                            first_call = 0 ;
                        }
                        s = getword (s, " \t(", &length) ;
                        if (cflow)
                            printf ("    %.*s () ;\n", length, s) ;
                        else
                            printf ("    %.*s\n", length, s) ;
                    }
                }
            }

        }     /* While not end-of-file */


        fclose (infile) ;		/* Close the input file. */

/*******************************************************************************
    Print out the list of modules and their lines-of-code counts.
*******************************************************************************/

        for (i = 0 ;  i < list_length (module_list) ;  i++) {
            m = (module *) list_get (module_list, i+1) ;
            if (struchart)  continue ;
            if (i == 0) {
                s = fparse (file_name, NULL, FILEXTVER) ;
                length = strlen (s) + 1 ;
                length = (25 + (long_names * 8) - length - 1 + 7) / 8 ;
                if (length < 1)  length = 1 ;
                printf ("\n%s:%.*s%4d\t\t\t\t(%d)\n",
                        s, length, tabs, m->loc_count, total) ;
                continue ;
            }
            length = strlen (m->name) + 4 ;
            length = (25 + (long_names * 8) - length - 1 + 7) / 8 ;
            if (length < 1)  length = 1 ;
            printf ("    %s%.*s%4d  %4d    %4d  %4d\n",
                    m->name, length, tabs,
                    m->loc_count, m->loc_ds,
                    m->pdl_count, m->pdl_ds) ;
        }

        num_modules = num_modules + list_length (module_list) - 1 ;
        total_all_files = total_all_files + total ;

        str_free (&file_name, -1) ;

    }


/*******************************************************************************
    Output summary statistics for all the files processed.
*******************************************************************************/

    if (!struchart && !cflow) {
        printf ("\nSummary - # of files: %d, # of modules: %d, # of lines: %d\n",
                num_files, num_modules, total_all_files) ;
    }

}

/*******************************************************************************

Procedure:

    remove_comments ()


Author:    Alex Measday, ISI


Purpose:

    Function REMOVE_COMMENTS removes C Language comments from input text.
    For example, the following fragment of code reads a C source file
    (through STDIN), strips the comments from the file, and outputs the
    bare-bones code to STDOUT:

        char  buffer[1024] ;
        int  in_comment ;
        ...
        in_comment = 0 ;
        while (gets (buffer) != NULL) {
            remove_comments (buffer, &in_comment) ;
            printf ("%s\n", buffer) ;
        }

    Note that IN_COMMENT must be set to 0 before processing an input
    file.

    REMOVE_COMMENTS handles multi-line comments and multi-comment lines.
    Comment delimiters occuring inside a C string (i.e., they aren't
    really comment delimiters) will throw REMOVE_COMMENTS off.


    Invocation:

        remove_comments (line_of_text, &in_comment) ;

    where

        <line_of_text>
            is the next line of text from a C source file.  If this line
            contains comments or is part of a comment, REMOVE_COMMENTS
            will modify the text - in place.
        <in_comment>
            is a pointer to a flag that keeps track of whether REMOVE_COMMENTS
            is in the middle of a multi-line comment.


File/Record References:
Name                      Use    Description
----                      ---    -----------
None

System Variables:
Mnemonic name      Process name        Use   Description
-------------      ------------        ---   -----------
None

Notes:

*******************************************************************************/

/*PDL----------------------------PDL--------------------------PDL**

    TBD

**PDL----------------------------PDL--------------------------PDL*/


remove_comments (line_of_text, in_comment)

    char  *line_of_text ;
    int  *in_comment ;

{    /* Local variables. */
    char  *s, *t ;



    s = line_of_text ;

/* If we're in the middle of a multi-line comment, then see if this new
   line contains the end of the comment. */

    if (*in_comment) {
        t = str_index (line_of_text, -1, "*/") ;
        if (t == NULL) {			/* End delimiter not seen. */
            *s = '\0' ;
        } else {				/* End of comment found. */
            t++ ;  t++ ;
            while (s != t)  *s++ = ' ' ;
            *in_comment = 0 ;
        }
    }

/* Remove any in-line comments and check for the beginning of a multi-line
   comment. */

    while (s = str_index (s, -1, "/*")) {
        t = str_index (s, -1, "*/") ;
        if (t == NULL) {			/* Start of multi-line comment. */
            *s = '\0' ;  *in_comment = 1 ;  break ;
        } else {				/* In-line comment. */
            t++ ;  t++ ;
            while (s != t)  *s++ = ' ' ;
            *in_comment = 0 ;
        }
    }

    str_trim (line_of_text, -1) ;		/* Trim trailing blanks. */

}
