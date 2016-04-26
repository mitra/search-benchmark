/* $Id: wst.c,v 1.2 2005/02/12 00:44:18 alex Exp $ */
/*******************************************************************************

    wst.c


    Wordstar Translation Filter.


    Program WST filters a Wordstar-generated file, translating Wordstar
    text highlighting codes to the corresponding ANSI escape sequences.


    Invocation:

        % wst [-none] [-printer <type>] [<input_file>]

    where:

        "-none"
            strips the Wordstar highlighting codes from the text and
            does NOT substitute the ANSI escape sequences.
        "-printer <type>"
            specifies what type printer your are using:
                "dec"  - DEClaser
                "hp"   - Hewlett-Packard Laser-Jet
                "html" - HyperText Markup Language
                "lpb8" - Canon laser printer (default)
                "roff" - NROFF/TROFF-compatible output
                "vt"   - VT100
        <input_file>
            is the input file to be translated; if no file is specified,
            standard input is read.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "opt_util.h"			/* Option scanning definitions. */

#define  CONTROL_B  '\002'		/* Wordstar BOLD indicator. */
#define  CONTROL_D  '\004'		/* Wordstar DOUBLE_STRIKE indicator. */
#define  CONTROL_E  '\005'		/* Wordstar Custom indicator - expanded. */
#define  CONTROL_Q  '\021'		/* Wordstar Custom indicator - shaded. */
#define  CONTROL_R  '\022'		/* Wordstar Custom indicator - reverse. */
#define  CONTROL_S  '\023'		/* Wordstar UNDERLINE indicator. */
#define  CONTROL_T  '\024'		/* Tall (Wordstar superscript indicator). */
#define  CONTROL_V  '\026'		/* Shaded box (Wordstar subscript indicator). */
#define  CONTROL_W  '\027'		/* Wordstar Custom indicator - wide. */
#define  CONTROL_X  '\030'		/* Box (Wordstar strikeout indicator). */
#define  CONTROL_Y  '\031'		/* Wordstar ITALICS indicator. */




int  main (argc, argv)

    int  argc ;
    char  *argv[] ;

{  /* Local variables. */
    char  *argument, c, *file_name, *printer ;
    int  bold_flag, box_flag, double_strike_flag ;
    int  errflg, expanded_flag, italics_flag ;
    int  option, reverse_flag ;
    int  shaded_box_flag, shaded_flag, strip_flag ;
    int  tall_flag, underline_flag, wide_flag ;
    FILE  *infile ;
    OptContext  scan ;

    char  *bold_on, *bold_off ;	/* Control sequences for text attributes. */
    char  *double_strike_on, *double_strike_off ;
    char  *expanded_on, *expanded_off ;
    char  *shaded_on, *shaded_off ;
    char  *reverse_on, *reverse_off ;
    char  *underline_on, *underline_off ;
    char  *tall_on, *tall_off ;
    char  *shaded_box_on, *shaded_box_off ;
    char  *wide_on, *wide_off ;
    char  *box_on, *box_off ;
    char  *italics_on, *italics_off ;
    char  *reset_printer ;

    const  char  *optionList[] = {	/* Command line options. */
        "{none}", "{printer:}", NULL
    } ;

/*******************************************************************************

  Scan the command line options.

*******************************************************************************/

    file_name = NULL ;  printer = "lpb8" ;  strip_flag = 0 ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case 1:			/* "-none" */
            strip_flag = 1 ;
            break ;
        case 2:			/* "-printer <type>" */
            printer = argument ;
            break ;
        case NONOPT:
            if (file_name == NULL)  file_name = argument ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    opt_term (scan) ;

    if (errflg) {
        fprintf (stderr, "Usage:  wst [-none] [-printer <type>] [<input_file>]\n") ;
        exit (-1) ;
    }


/*******************************************************************************
    Set up the printer-dependent control sequences to effect the various
    text attributes.
*******************************************************************************/

/* DEClaser 2100. */

    if (strcmp (printer, "dec") == 0) {
        bold_on = "\033[1m" ;  bold_off = "\033[22m" ;
        double_strike_on = "\033[1m" ;  double_strike_off = "\033[22m" ;
        expanded_on = "" ;  expanded_off = "" ;
        shaded_on = "" ;  shaded_off = "" ;
        reverse_on = "\033[1m" ;  reverse_off = "\033[22m" ;
        underline_on = "\033[4m" ;  underline_off = "\033[24m" ;
        tall_on = "" ;  tall_off = "" ;
        shaded_box_on = "" ;  shaded_box_off = "" ;
        wide_on = "" ;  wide_off = "" ;
        box_on = "" ;  box_off = "" ;
        italics_on = "\033[3m" ;  italics_off = "\033[23m" ;
        reset_printer = "" ;
    }

/* HP Laser-Jet II in Courier, 10-point portrait mode. */

    else if (strcmp (printer, "hp") == 0) {
        bold_on = "\033(s3B" ;  bold_off = "\033(s0B" ;
        expanded_on = "" ;  expanded_off = "" ;
        shaded_on = "" ;  shaded_off = "" ;
        tall_on = "" ;  tall_off = "" ;
        underline_on = "\033&dD" ;  underline_off = "\033&d@" ;
        shaded_box_on = "" ;  shaded_box_off = "" ;
        wide_on = "" ;  wide_off = "" ;
        box_on = "" ;  box_off = "" ;
        reset_printer = "\033E" ;
        /* Make do with what we have ... */
        double_strike_on = "\033(s3B\033&dD" ;  double_strike_off = "\033&d@\033(s0B" ;
        italics_on = "\033(s1S" ;  italics_off = "\033(s0S" ;
        reverse_on = double_strike_on ;  reverse_off = double_strike_off ;
    }

/* HyperText Markup Language. */

    else if (strcmp (printer, "html") == 0) {
        bold_on = "<B>" ;  bold_off = "</B>" ;
        double_strike_on = "<FONT SIZE=*2>" ;  double_strike_off = "</FONT>" ;
        expanded_on = "<FONT SIZE=*2>" ;  expanded_off = "</FONT>" ;
        shaded_on = "" ;  shaded_off = "" ;
        reverse_on = "" ;  reverse_off = "" ;
        underline_on = "<EM>" ;  underline_off = "</EM>" ;
        tall_on = "" ;  tall_off = "" ;
        shaded_box_on = "\n<PRE>\n" ;  shaded_box_off = "\n</PRE>\n" ;
        wide_on = "" ;  wide_off = "" ;
        box_on = "\n<PRE>\n" ;  box_off = "\n</PRE>\n" ;
        italics_on = "<I>" ;  italics_off = "</I>" ;
        reset_printer = "" ;
    }

/* Canon LPB8-II Laser Printer in ISO, portrait mode. */

    else if (strcmp (printer, "lpb8") == 0) {
        bold_on = "\033[1m" ;  bold_off = "\033[22m" ;
        double_strike_on = "\033[21m" ;  double_strike_off = "\033[24m" ;
        expanded_on = "\033[200;200 B" ;  expanded_off = "\033[100;100 B" ;
        shaded_on = "\033[5m" ;  shaded_off = "\033[25m" ;
        reverse_on = "\033[7m" ;  reverse_off = "\033[27m" ;
        underline_on = "\033[4m" ;  underline_off = "\033[24m" ;
        tall_on = "\033[200;100 B" ;  tall_off = "\033[100;100 B" ;
        shaded_box_on = "\033[s" ;  shaded_box_off = "\033[r" ;
        wide_on = "\033[100;200 B" ;  wide_off = "\033[100;100 B" ;
        box_on = "\033[{" ;  box_off = "\033[}" ;
        italics_on = "\033[3m" ;  italics_off = "\033[23m" ;
        reset_printer = "" ;
    }

/* NROFF/TROFF-compatible output.  Only bold printing, underlining, and
   italics are supported. */

    else if (strcmp (printer, "roff") == 0) {
        bold_on = "\\fB" ;  bold_off = "\\fP" ;
        double_strike_on = "\\fU" ;  double_strike_off = "\\fP" ;
        expanded_on = "" ;  expanded_off = "" ;
        shaded_on = "" ;  shaded_off = "" ;
        reverse_on = "" ;  reverse_off = "" ;
        underline_on = "\\fU" ;  underline_off = "\\fP" ;
        tall_on = "" ;  tall_off = "" ;
        shaded_box_on = "" ;  shaded_box_off = "" ;
        wide_on = "" ;  wide_off = "" ;
        box_on = "" ;  box_off = "" ;
        italics_on = "\\fI" ;  italics_off = "\\fP" ;
        reset_printer = "" ;
    } else {
        fprintf (stderr, "wst: Invalid printer type: %s\n", printer) ;
        errno = EINVAL ;
        exit (errno) ;
    }

/*******************************************************************************

    Open, read, and translate the file.

*******************************************************************************/


    infile = fopen (file_name, "r") ;
    if (infile == NULL) {
        perror ("fopen") ;
        exit (errno) ;
    }


    bold_flag = 0 ;
    box_flag = 0 ;
    double_strike_flag = 0 ;
    expanded_flag = 0 ;
    italics_flag = 0 ;
    reverse_flag = 0 ;
    shaded_box_flag = 0 ;
    shaded_flag = 0 ;
    tall_flag = 0 ;
    wide_flag = 0 ;
    underline_flag = 0 ;


    while ((c = getc (infile)) != EOF) {

        c = c & 0x7F ;			/* Mask out WordStar control bit. */

        switch (c) {

        case (CONTROL_B):
            if (strip_flag)  break ;
            printf ("%s", (bold_flag = !bold_flag) ? bold_on : bold_off) ;
            break ;

        case (CONTROL_D):
            if (strip_flag)  break ;
            printf ("%s", (double_strike_flag = !double_strike_flag) ? double_strike_on : double_strike_off) ;
            break ;

        case (CONTROL_E):
            if (strip_flag)  break ;
            printf ("%s", (expanded_flag = !expanded_flag) ? expanded_on : expanded_off) ;
            break ;

        case (CONTROL_Q):
            if (strip_flag)  break ;
            printf ("%s", (shaded_flag = !shaded_flag) ? shaded_on : shaded_off) ;
            break ;

        case (CONTROL_R):
            if (strip_flag)  break ;
            printf ("%s", (reverse_flag = !reverse_flag) ? reverse_on : reverse_off) ;
            break ;

        case (CONTROL_S):
            if (strip_flag)  break ;
            printf ("%s", (underline_flag = !underline_flag) ? underline_on : underline_off) ;
            break ;

        case (CONTROL_T):
            if (strip_flag)  break ;
            printf ("%s", (tall_flag = !tall_flag) ? tall_on : tall_off) ;
            break ;

        case (CONTROL_V):
            if (strip_flag)  break ;
            printf ("%s", (shaded_box_flag = !shaded_box_flag) ? shaded_box_on : shaded_box_off) ;
            break ;

        case (CONTROL_W):
            if (strip_flag)  break ;
            printf ("%s", (wide_flag = !wide_flag) ? wide_on : wide_off) ;
            break ;

        case (CONTROL_X):
            if (strip_flag)  break ;
            printf ("%s", (box_flag = !box_flag) ? box_on : box_off) ;
            break ;

        case (CONTROL_Y):
            if (strip_flag)  break ;
            printf ("%s", (italics_flag = !italics_flag) ? italics_on : italics_off) ;
            break ;

        default:  putchar (c) ;  break ;

        }

    }


    printf ("%s", reset_printer) ;

    exit (0) ;

}
