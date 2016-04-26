/* $Id: words_net.c,v 1.1 2005/01/10 19:31:55 alex Exp alex $ */
/*******************************************************************************

File:

    words_net.c

    Network Utilities.


Author:    Alex Measday


Purpose:

    The WORDS_NET package defines words for translating IP addresses,
    host names, and service ports:

        "<host>" NET-ADDR
        <address> dotted? NET-HOST
        "<service>" udp? NET-PORT


Public Procedures:

    buildWordsNET() - registers the words with the FICL system.

Private Procedures:

    word_NET_ADDR() - implements the NET-ADDR word.
    word_NET_HOST() - implements the NET-HOST word.
    word_NET_PORT() - implements the NET-PORT word.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */
#include  "net_util.h"			/* Networking utilities. */
#include  "skt_util.h"			/* Socket support functions. */
#include  "str_util.h"			/* String manipulation functions. */
#include  "finc.h"			/* Forth-Inspired Network Commands. */


/*******************************************************************************
    Private functions.
*******************************************************************************/

static  void  word_NET_ADDR P_((ficlVm *vm)) ;
static  void  word_NET_HOST P_((ficlVm *vm)) ;
static  void  word_NET_PORT P_((ficlVm *vm)) ;

/*!*****************************************************************************

Procedure:

    buildWordsNET ()

    Enter the NET Words into the Dictionary.


Purpose:

    Function buildWordsNET() enters the NET words into the system dictionary.


    Invocation:

        buildWordsNET (sys) ;

    where

        <sys>	- I
            is the FICL system.

*******************************************************************************/


void  buildWordsNET (

#    if PROTOTYPES
        ficlSystem  *sys)
#    else
        sys)

        ficlSystem  *sys ;
#    endif

{

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "NET-ADDR", word_NET_ADDR,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "NET-HOST", word_NET_HOST,
                                FICL_WORD_DEFAULT) ;

    ficlDictionarySetPrimitive (ficlSystemGetDictionary (sys),
                                "NET-PORT", word_NET_PORT,
                                FICL_WORD_DEFAULT) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_NET_ADDR ()

    Translate Host Name to IP Address.


Purpose:

    Function word_NET_ADDR() implements the NET-ADDR word, which translates
    a host name to an IP address.

        NET-ADDR

            ( c-addr u1 -- u2 )

        Lookup the host name represented by c-addr,u1 and return its
        IP address in network-byte-order in u2.


    Invocation:

        word_NET_ADDR (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_NET_ADDR (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *hostname ;
    ficlCell  cell ;
    unsigned  long  ipAddress, length ;



    FICL_STACK_CHECK (vm->dataStack, 2, 1) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Host name. */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        hostname = NULL ;
    } else {
        hostname = (char *) cell.p ;
        if (hostname[length] != '\0')  hostname = strndup (hostname, length) ;
    }

/* Translate the host name to its IP address. */

    ipAddress = netAddrOf (hostname) ;

    if (hostname != cell.p)  free (hostname) ;

/* Return the IP address on the stack. */

    cell.u = ipAddress ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_NET_HOST ()

    Translate IP Address to Host Name.


Purpose:

    Function word_NET_HOST() implements the NET-HOST word, which looks up
    an IP address and returns the corresponding host name.

        NET-HOST

            ( u1 f -- c-addr u2 )

        Lookup the IP addres u1 and return the corresponding host name string
        as c-addr,u2.  If the flag f is true, return the address in dotted IP
        format.  The string is NUL-terminated and stored internally; it should
        be used or duplicated before calling NET-HOST again.


    Invocation:

        word_NET_HOST (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_NET_HOST (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    bool  dotted ;
    char  *hostname ;
    ficlCell  cell ;
    unsigned  long  ipAddress ;



    FICL_STACK_CHECK (vm->dataStack, 2, 2) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* Dotted? */
    dotted = (cell.i != 0) ;
    cell = ficlVmPop (vm) ;			/* IP address. */
    ipAddress = cell.u ;

/* Translate the IP address to its host name. */

    hostname = (char *) netHostOf (ipAddress, dotted) ;

/* Return the host name on the stack. */

    cell.p = hostname ;
    ficlVmPush (vm, cell) ;
    cell.u = strlen (hostname) ;
    ficlVmPush (vm, cell) ;

    return ;

}

/*!*****************************************************************************

Procedure:

    word_NET_PORT ()

    Translate Service Name to Server Port.


Purpose:

    Function word_NET_PORT() implements the NET-PORT word, which looks up
    a server's name in the network services database (the "/etc/services"
    file) and returns the server's port number.

        NET-PORT

            ( c-addr u f -- i )

        Lookup the service name c-addr,u in the network services database
        and return the corresponding port number.  If flag f is true, the
        "udp" port is returned; otherwise, the "tcp" port is returned.


    Invocation:

        word_NET_PORT (vm) ;

    where

        <vm>	- I
            is the FICL virtual machine in which the word is being executed.

*******************************************************************************/


static  void  word_NET_PORT (

#    if PROTOTYPES
        ficlVm  *vm)
#    else
        vm)

        ficlVm  *vm ;
#    endif

{    /* Local variables. */
    char  *protocol, *service ;
    ficlCell  cell ;
    int  port ;
    unsigned  long  length ;



    FICL_STACK_CHECK (vm->dataStack, 3, 1) ;

/* Get the arguments from the stack. */

    cell = ficlVmPop (vm) ;			/* UDP port? */
    protocol = (cell.i == 0) ? "tcp" : "udp" ;
    cell = ficlVmPop (vm) ;			/* Character count. */
    length = cell.u ;
    cell = ficlVmPop (vm) ;			/* Service name. */
    if ((length == 0) || (cell.p == NULL)) {
        cell.p = NULL ;
        service = NULL ;
    } else {
        service = (char *) cell.p ;
        if (service[length] != '\0')  service = strndup (service, length) ;
    }

/* Translate the service name to its port number. */

    port = netPortOf (service, protocol) ;

    if (service != cell.p)  free (service) ;

/* Return the port number on the stack. */

    cell.i = port ;
    ficlVmPush (vm, cell) ;

    return ;

}
