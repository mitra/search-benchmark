/* $Id$ */
/*******************************************************************************

    SioMain() - is a wrapper function for an application's main routine.
    The PilotMain() function in StdIOPalm calls the SioMain() function
    in the application.  Rather than renaming the main() routine in the
    application to SioMain(), simply link StdIOPalm.o, this SioMain.o,
    and your application's object files - in that order - to create the
    executable.  (#define'ing "main" as "SioMain" doesn't work because
    GCC complains about the function signatures not matching.)

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <StdIOPalm.h>			/* Palm SDK header. */

					/* Application's main routine. */
extern  int  main (int argc, char *argv[])  OCD ("sio_appl") ;


Int16  SioMain (

    UInt16  argc,
#ifdef PRE_VERSION_4
    Char  *argv[])
#else
    const  Char  *argv[])
#endif

{

    return (main ((int) argc, (char **) argv)) ;

}
