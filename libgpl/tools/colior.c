/* $Id: colior.c,v 1.3 2016/02/19 04:24:19 alex Exp $ */
/*******************************************************************************

Program:

    colior.c

    Dump CORBA Interoperable Object Reference (IOR).


Author:    Alex Measday


Purpose:

    COLIOR (COLI IOR Dump) dumps a CORBA IOR string of the form "IOR:..."
    or a "corbaloc:..." URL in human-readable form.  If no IOR or URL is
    specified, COLIOR prints out a usage message and also dumps an
    internally stored IOR test string.


    Invocation:

        % colior [<ior>|<url>]

    where:

        "<ior>"
            is the IOR string to be dumped.  The "IOR:" prefix is optional.
        "<url>"
            is a "corbaloc:" URL.  If a URL is specified, the URL is converted
            to an IOR string and the IOR string is then dumped.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "coli_util.h"			/* CORBA-Lite utilities. */
#include  "meo_util.h"			/* Memory operations. */
#include  "opt_util.h"			/* Option scanning definitions. */
#include  "str_util.h"			/* String manipulation functions. */

/*******************************************************************************
    COLIOR's Main Program.
*******************************************************************************/

extern  int  main P_((int argc, char *argv[]))  OCD ("sio_appl") ;

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
    char  *argument, *iorString, *url ;
    int  errflg, option ;
    IOR  ior ;
    OptContext  scan ;
    unsigned  long  i, j ;

    const  char  *testIOR =		/* Sample IOR for testing. */
        "IOR:010000001900000049444c3a48656c6c6f576f726c642f48656c6c6f3a312e3000000000010000000000000070000000010101cd0900000046414354554d323500cd30091b00000014010f00525354088d983b99730d00000000000100000001000000cd03000000000000000800000001cdcdcd004f4154010000001400000001cdcdcd01000100000000000901010000000000004f41540400000001cd0000" ;

    const  char  *optionList[] = {	/* Command line options. */
        NULL
    } ;




#if HAVE_SIGNAL && defined(SIGPIPE)
    signal (SIGPIPE, SIG_IGN) ;
#endif
    aperror_print = 1 ;


/*******************************************************************************
    Scan the command line options.
*******************************************************************************/

    iorString = NULL ;

    opt_init (argc, argv, NULL, optionList, &scan) ;
    errflg = 0 ;

    while ((option = opt_get (scan, &argument))) {

        switch (option) {
        case NONOPT:
            if (iorString == NULL)
                iorString = argument ;
            else
                errflg++ ;
            break ;
        case OPTERR:
            errflg++ ;  break ;
        default:  break ;
        }

    }

    opt_term (scan) ;

    if (errflg || (iorString == NULL)) {
        fprintf (stderr, "Usage:  colior <ior>|<url>\n") ;
        if (errflg)  exit (EINVAL) ;
        iorString = (char *) testIOR ;		/* Dump test IOR. */
        printf ("\n         Test IOR = %s\n", iorString) ;
    }

/*******************************************************************************
    Decode the IOR string and dump the contents of the resulting IOR.
*******************************************************************************/

/* If a "corbaloc:" URL was specified, convert it to an "IOR:" string. */

    if (strstr (iorString, "corbaloc:") == iorString) {
        if (coliURL2O (iorString, &ior)) {
            LGE "[%s] Invalid URL.\n") ;
            exit (errno) ;
        }
        iorString = coliO2S (&ior, true) ;
        comxErase ((ComxFunc) gimxIOR, &ior) ;
    }

    url = coliS2URL (iorString, false) ;
    if (url == NULL) {
        LGE "[%s] Invalid IOR string.\n") ;
        exit (errno) ;
    }

    printf ("\n              URL = %s\n", url) ;

    if (coliS2O (iorString, &ior)) {
        LGE "[%s] Invalid IOR string.\n") ;
        exit (errno) ;
    }

    if (ior.type_id != NULL) {
        printf ("\n    Repository ID = %s\n", ior.type_id) ;
    }

    for (i = 0 ;  i < ior.profiles.count ;  i++) {

        TaggedProfile  *profile = &ior.profiles.elements[i] ;

        printf ("\n%s Profile\n", coliToName (ProfileIdLUT, profile->which)) ;

        if (profile->which == IOP_TAG_INTERNET_IOP) {

            ProfileBody  *body = &profile->data.iiop_body ;
            ObjectKey  key = body->object_key ;

            printf ("          Version:  %d.%d\n",
                    body->iiop_version.major, body->iiop_version.minor) ;

            printf ("          Address:  %hu@%s\n",
                    body->port, body->host) ;

            printf ("       Object Key =\n") ;
            meoDumpX (stdout, "         ", 0, key.elements, key.count) ;

            for (j = 0 ;  j < body->components.count ;  j++) {
                TaggedComponent  *component = &body->components.elements[j] ;
                printf ("        Component = %s\n",
                        coliToName (ComponentIdLUT, component->tag)) ;
                if (component->tag == IOP_TAG_CODE_SETS) {
                    CodeSetComponentInfo  codeSets ;
                    if (comxEncapsule (body->iiop_version, MxDECODE,
                        &component->component_data,
                        gimxCodeSetComponentInfo, &codeSets, NULL)) {
                        LGE "[%s] Error decoding encapsulated code information.\ncomxEncapsule: ",
                            argv[0]) ;
                        meoDumpX (stdout, "         ", 0,
                                  component->component_data.elements,
                                  component->component_data.count) ;
                    } else {
                        printf ("           Normal:  %s\n",
                                coliToName (CodeSetIdLUT,
                                            codeSets.ForCharData.native_code_set)) ;
                        printf ("             Wide:  %s\n",
                                coliToName (CodeSetIdLUT,
                                            codeSets.ForWcharData.native_code_set)) ;
                    }
                } else {
                    meoDumpX (stdout, "         ", 0,
                              component->component_data.elements,
                              component->component_data.count) ;
                }
            }

        } else if (profile->which == IOP_TAG_MULTIPLE_COMPONENTS) {
            printf ("...\n") ;
        } else {
            OctetSeq  unknown = profile->data.profile_data ;
            printf ("     Profile Data =\n") ;
            meoDumpX (stdout, "         ", 0,
                      unknown.elements, unknown.count) ;
        }

    }

    exit (0) ;

}
