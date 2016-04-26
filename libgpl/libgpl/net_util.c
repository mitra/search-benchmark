/* $Id: net_util.c,v 1.19 2011/07/18 17:40:57 alex Exp alex $ */
/*******************************************************************************

File:

    net_util.c

    Network Utilities.


Author:    Alex Measday


Purpose:

    The NET_UTIL package is a collection of miscellaneous network functions
    primarily intended to isolate operating system dependencies in networking
    code.


Public Procedures:

    netAddrOf() - translates a host name to the host's IP address.
    netHostOf() - translates an IP address to its host name.
    netPortOf() - translates a service name to its server port.

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */

#include  <stdio.h>			/* Standard I/O definitions. */
#include  <stdlib.h>			/* Standard C Library definitions. */
#include  <string.h>			/* C Library string functions. */

#include  "skt_util.h"			/* TCP/IP networking utilities. */

#if !defined(_WIN32) && !defined(__palmos__)
#    include  <netdb.h>			/* Network database definitions. */
#    include  <sys/types.h>		/* System type definitions. */
#    include  <sys/socket.h>		/* Socket-related definitions. */
#    ifdef DECLARE_GETHOSTNAME
        extern  int  gethostname (char *, size_t) ;
#    else
#        include  <unistd.h>		/* UNIX I/O definitions. */
#    endif
#    include  <netinet/in.h>		/* Internet IPC domain definitions. */
#    ifndef NDS
#        include  <arpa/inet.h>		/* Internet library definitions. */
#    endif
#    if defined(HAVE_H_ADDR) && !HAVE_H_ADDR
#        define  h_addr  h_addr_list[0]	/* Use old name for "hostent" field. */
#    endif
#endif

#include  "net_util.h"			/* Networking utilities. */

#ifndef MAXHOSTNAMELEN
#    define  MAXHOSTNAMELEN  64
#endif

/*!*****************************************************************************

Procedure:

    netAddrOf ()

    Translate Host Name to IP Address.


Purpose:

    Function netAddrOf() looks up a host by name and returns its IP address.


    Invocation:

        ipAddress = netAddrOf (hostName) ;

    where

        <hostName>	- I
            is the host's name.  If this argument is NULL, the IP address
            of the local host is returned.
        <ipAddress>	- O
            returns the IP adddress of the host in network-byte order; zero
            is returned in the case of an error.

*******************************************************************************/


in_addr_t  netAddrOf (

#    if PROTOTYPES
        const  char  *hostName)
#    else
        hostName)

        char  *hostName ;
#    endif

{    /* Local variables. */
    char  localHost[MAXHOSTNAMELEN+1] ;
    in_addr_t  ipAddress ;




/* If no host name was specified, use the local machine's host name. */

    if (hostName == NULL) {
        if (gethostname (localHost, sizeof localHost)) {
            SET_ERRNO (GET_NETERRNO ()) ;
            if (errno == 0)  SET_ERRNO (EINVAL) ;
            LGE "(netAddrOf) Error getting local host name.\ngethostname: ") ;
            return (0) ;
        }
        hostName = localHost ;
    }

/* If the host name was specified using the internet dot notation, then
   convert it to a binary address. */

    ipAddress = inet_addr (hostName) ;
    if (ipAddress != INADDR_NONE)  return (ipAddress) ;

/* Otherwise, an actual name was specified.  Look up the name in the
   operating system's host database. */

#ifdef VXWORKS
    if ((ipAddress = hostGetByName ((char *) hostName)) == (unsigned long) ERROR) {
        LGE "(netAddrOf) Error getting host address for \"%s\".\nhostGetByName: ",
            hostName) ;
        return (0) ;
    }
#else
  { struct  hostent  *hostEntry ;

    if ((hostEntry = gethostbyname (hostName)) == NULL) {
        SET_ERRNO (EINVAL) ;
        LGE "(netAddrOf) Error getting host entry for \"%s\".\ngethostbyname: ",
            hostName) ;
        return (0) ;
    }
			/* Try to avoid big-endian/little-endian problems
			   by grabbing address as an unsigned long value. */
    if (hostEntry->h_length != sizeof (in_addr_t)) {
        SET_ERRNO (EINVAL) ;
        LGE "(netAddrOf) %s's address length is %d bytes, not %lu bytes.\n",
            hostName, hostEntry->h_length,
            (unsigned long) sizeof (in_addr_t)) ;
        return (0) ;
    }
    ipAddress = *((in_addr_t *) hostEntry->h_addr) ;
  }
#endif

    return (ipAddress) ;

}

/*!*****************************************************************************

Procedure:

    netHostOf ()

    Translate IP Address to Host Name.


Purpose:

    Function netHostOf() looks up an IP address and returns the corresponding
    host name.  The local host name can be easily retrieved by the following
    composite call:

        char  *localHost = netHostOf (netAddrOf (NULL), false) ;


    Invocation:

        hostName = netHostOf (ipAddress, dotted) ;

    where

        <ipAddress>	- I
            is the IP adddress of the host.
        <dotted>	- I
            if true (non-zero), specifies that the ASCII dotted IP address
            ("a.b.c.d") is to be returned instead of the translated name.
        <hostName>	- O
            returns the host's name; NULL is returned in the event of an
            error.  The ASCII name string is stored local to netHostOf()
            and it should be used or duplicated before calling netHostOf()
            again.

*******************************************************************************/


const  char  *netHostOf (

#    if PROTOTYPES
        in_addr_t  ipAddress,
        bool  dotted)
#    else
        ipAddress, dotted)

        in_addr_t  ipAddress ;
        bool  dotted ;
#    endif

{    /* Local variables. */
    struct  in_addr  address ;
    static  char  hostName[MAXHOSTNAMELEN+1]  OCD ("net_util") ;



/* Convert the IP address to its dotted format, "a.b.c.d". */

    address.s_addr = ipAddress ;
#ifdef VXWORKS
    inet_ntoa_b (address, hostName) ;
#else
    strcpy (hostName, inet_ntoa (address)) ;
#endif

    if (dotted)  return (hostName) ;

/* Query the name server for the host's name. */

#ifdef VXWORKS
    if (hostGetByAddr (ipAddress, hostName)) {
        inet_ntoa_b (address, hostName) ;		/* Restore IP address. */
    }
#elif !defined(HAVE_GETHOSTBYADDR) || HAVE_GETHOSTBYADDR
  { struct  hostent  *host ;

    host = gethostbyaddr ((const char *) &address, sizeof address, AF_INET) ;
    if (host != NULL)  strcpy (hostName, host->h_name) ;	/* Actual name. */
  }
#endif

    return (hostName) ;

}

/*!*****************************************************************************

Procedure:

    netPortOf ()

    Translate Service Name to Server Port.


Purpose:

    Function netPortOf() looks up a server's name in the network services
    database (the "/etc/services" file) and returns the server's port
    number.

    NOTE:  netPortOf() makes use of the system getservbyname(3) function
    under operating system's that support that function.  Otherwise, it
    manually reads the "/etc/services" file in search of the server name.
    In this latter case, a file other than "/etc/services" can be used as
    the services database by storing the full pathname of the file in
    environment variable, "SERVICES_FILE".


    Invocation:

        port = netPortOf (serverName, protocol) ;

    where

        <serverName>	- I
            is the name of the service being looked up.
        <protocol>	- I
            specifies a network protocol; e.g., "tcp" or "udp".  A given
            service name may appear under different protocols in the
            services file.  If PROTOCOL is specified, then netPortOf()
            returns the server's port for that protocol.  If PROTOCOL is
            NULL, then netPortOf() returns the first port for the service
            name, regardless of the protocol.
        <port>		- O
            returns the requested service's port number in host byte order;
            -1 is returned in the event of an error (e.g., the service was
            not found).

*******************************************************************************/


int  netPortOf (

#    if PROTOTYPES
        const  char  *serverName,
        const  char  *protocol)
#    else
        serverName, protocol)

        char  *serverName ;
        char  *protocol ;
#    endif

{    /* Local variables. */
    char  *s ;
    int  portNumber ;
#if defined(HAVE_GETSERVBYNAME) && !HAVE_GETSERVBYNAME
    char  *fileName, inbuf[128] ;
    char  *s_name, *s_port, *s_protocol ;
    FILE  *infile ;
#    ifdef VXWORKS			/* Use reentrant version of STRTOK(). */
#        define  strtok(string, separators) \
                    strtok_r (string, separators, &last)
        char  *last ;
#    endif
#else
    struct  servent  *serverEntry ;
#endif





/* If the server "name" is simply the desired port number in ASCII,
   then convert and return the binary port number to the caller. */

    portNumber = strtol ((char *) serverName, &s, 0) ;
    if (*s == '\0') {
        if (portNumber < 0) {
            SET_ERRNO (EINVAL) ;
            LGE "(netPortOf) Invalid port number: %d\n", portNumber) ;
        }
        return (portNumber) ;
    }


#if !defined(HAVE_GETSERVBYNAME) || HAVE_GETSERVBYNAME

/* Under operating systems that support getservbyname(3), call it. */

    if ((serverEntry = getservbyname (serverName, NULL)) == NULL) {
        if (errno == 0)  SET_ERRNO (EINVAL) ;
        LGE "(netPortOf) Error getting server entry for %s.\ngetservbyname: ",
            serverName) ;
        return (-1) ;
    }

    return (ntohs (serverEntry->s_port)) ;

#else


/*******************************************************************************
    Open the network services file.  The name of the file is retrieved
    from environment variable "SERVICES_FILE".  If that variable has
    no translation, then the file name defaults to the "/etc/services"
    system file.
*******************************************************************************/

    fileName = getenv ("SERVICES_FILE") ;
    if (fileName == NULL)  fileName = "/etc/services" ;

    infile = fopen (fileName, "r") ;
    if (infile == NULL) {
        LGE "(netPortOf) Error opening \"%s\" file.\nfopen: ", fileName) ;
        return (-1) ;
    }


/*******************************************************************************
    Scan the network services file for the requested service name.
*******************************************************************************/

    for ( ; ; ) {

/* Read the next server entry from the file.  Blank lines and comments
   are skipped. */

						/* Read next line from file. */
        if (fgets (inbuf, (sizeof inbuf), infile) == NULL) {
            if (ferror (infile))
                LGE "(netPortOf) Error reading %s.\nfgets: ", fileName) ;
            fclose (infile) ;
            return (-1) ;
        }					/* Strip comments and form feeds. */
        if ((s = strchr (inbuf, '#')) != NULL)  *s = '\0' ;
        while ((s = strchr (inbuf, '\f')) != NULL)  *s = ' ' ;

/* Extract the service name, port number, and protocol from the entry. */

#ifdef VXWORKS
        last = NULL ;				/* For STRTOK_R(2). */
#endif
        s_name = strtok (inbuf, " \t\n/") ;	/* Service name. */
        if (s_name == NULL)  continue;		/* Skip blank lines. */
        s_port = strtok (NULL, " \t\n/") ;	/* Port number. */
        if (s_port == NULL)  continue ;		/* No port number? */
        s_protocol = strtok (NULL, " \t\n/") ;	/* Protocol. */
        if (s_protocol == NULL)  continue ;	/* No protocol? */

/* If the protocols match, then scan the name and the aliases for the
   desired service name. */

        if ((protocol != NULL) && (strcmp (protocol, s_protocol) != 0))
            continue ;				/* Protocols don't match? */

        while (s_name != NULL) {		/* Scan name and aliases. */
            if (strcmp (s_name, serverName) == 0)  break ;
            s_name = strtok (NULL, " \t\n/") ;
        }

        if (s_name != NULL)  break ;		/* Desired service found? */

    }


/* An entry for the desired service was found.  Close the services file and
   return the service's port number to the calling routine. */

    fclose (infile) ;

    return (atoi (s_port)) ;

#endif    /* HAVE_GETSERVBYNAME */

}

#ifdef  TEST

/*******************************************************************************

    Program to test the NET_UTIL functions.

    Under UNIX:
        Compile, link, and run as follows:
            % cc -DTEST net_util.c <libraries> -o net_test
            % net_test <port>

    Under VxWorks:
        Compile and link as follows:
            % cc -DTEST net_util.c <libraries> -o net_test.vx.o
        Load and run the server with the following commands:
            -> ld <net_test.vx.o
            -> sp net_test, "<port>"

    The test program is a basic FTP server that listens for clients
    at the specified network port.  Try connecting to it from within
    ftp(1):

        % ftp
        ftp> open <host> <port>
        ... enter username and password ...
        ftp> pwd
        ... see current directory ...
        ftp> ls
        ... list current directory ...
        ftp> close
        ... connection to server is closed ...
        ftp>

*******************************************************************************/

#ifdef VXWORKS

    void  net_test (
        char  *commandLine)

#else

    main (argc, argv)
        int  argc ;
        char  *argv[] ;

#endif

{

#ifdef VXWORKS
    char  **argv ;
    int  argc ;
		/* Parse command string into an ARGC/ARGV array of arguments. */
    opt_create_argv ("net_test", commandLine, &argc, &argv) ;
#endif

    aperror_print = 1 ;

    printf ("Address of %s = 0x%X\n", argv[1], netAddrOf (argv[1])) ;

    printf ("   Host of %s = %s\n", argv[1],
            netHostOf ((in_addr_t) strtoul (argv[1], NULL, 0), false)) ;

    printf ("   Port of %s = %d\n", argv[1], netPortOf (argv[1], NULL)) ;

    exit (0) ;

}

#endif  /* TEST */
