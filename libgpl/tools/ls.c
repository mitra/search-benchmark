#include  <stdio.h>
#include  "aperror.h"
#include  "drs_util.h"

int  main (int argc, char *argv[])
{
    char  *fileName ;
    DirectoryScan  scan ;

    aperror_print = 1 ;
    drs_util_debug = 1 ;

    if (drsCreate (argv[1], &scan))  return (0) ;
    fileName = drsFirst (scan) ;
    while (fileName != NULL) {
        printf ("\"%s\"\n", fileName) ;
        fileName = drsNext (scan) ;
    }
    drsDestroy (scan) ;
    return (0) ;
}