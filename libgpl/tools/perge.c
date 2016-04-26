/*
%Z%  FILE: %M%  RELEASE: %I%  DATE: %G%, %U%
*/
/*******************************************************************************

    perge.c

    Permutation Generator.


    Alex Measday, ISI


    Invocation:

        % perge [-alphanumeric] [-lower] [-upper] <numCharacters>

    where:

        "-alphanumeric"
            specifies that letters AND numbers are to be used.
        "-lower"
            specifies that letters are to be generated in lower case.
        "-upper"
            specifies that letters are to be generated in upper case.
        "<numCharacters>"
            specifies the number of characters to be permutated.

*******************************************************************************/


#include  <errno.h>			/* System error definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "vperror.h"			/* VPERROR() definitions. */




int  main (

#    if __STDC__
        int  argc,
        char  *argv[])
#    else
        argc, argv)

        int  argc ;
        char  *argv[] ;
#    endif

{  /* Local variables. */
    char  *argument, buffer[64] ;
    int  alpha, carry, errflg, i, numCharacters, numeric, option, upper ;

    static  char  *optionList[] = {	/* Command line options. */
        "{alphanumeric}", "{lower}", "{numeric}", "{upper}",
        NULL
    } ;




    vperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    alpha = 1 ;
    numCharacters = 3 ;
    numeric = 0 ;
    upper = 0 ;

    opt_init (argc, argv, 1, optionList, NULL) ;
    errflg = 0 ;

    while (option = opt_get (NULL, &argument)) {

        switch (option) {
        case 1:			/* "-alphanumeric" */
            alpha = 1 ;  numeric = 1 ;
            break ;
        case 2:			/* "-lower" */
            upper = 0 ;
            break ;
        case 3:			/* "-numeric" */
            alpha = 0 ;  numeric = 1 ;
            break ;
        case 4:			/* "-upper" */
            upper = 1 ;
            break ;
        case NONOPT:
            numCharacters = atoi (argument) ;
            if (numCharacters <= 0)  errflg++ ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default :  break ;
        }

    }

    if (errflg) {
        fprintf (stderr, "Usage:  perge [-alphanumeric] [-lower] [-numeric] [-upper] <numCharacters>\n") ;
        exit (EINVAL) ;
    }


    for (i = 0 ;  i < numCharacters ;  i++)
        buffer[i] = upper ? 'A' : 'a' ;
    buffer[i] = '\0' ;


    for ( ; ; ) {

        printf ("%s\n", buffer) ;

        for (carry = 0, i = numCharacters ;  i-- > 0 ; ) {
            if (buffer[i] == (upper ? 'Z' : 'z')) {
                buffer[i] = upper ? 'A' : 'a' ;  carry = 1 ;
            } else {
                buffer[i]++ ;  carry = 0 ;  break ;
            }
        }

        if (carry)  break ;

    }


    exit (0) ;

}
