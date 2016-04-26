$Id: README.txt,v 1.3 2009/05/22 15:57:43 alex Exp $


                                   CSOFT
                                   =====


Hi!  This is my general-purpose C library (LIBGPL) consisting of
networking, regular expression, string manipulation, and assorted
other functions. The code is implemented as "packages", usually
with a "<prefix>_util.h" header file and a "<prefix>_util.c" code
file, where <prefix> is a mnemonic prefix used for the package's
function names.

A great deal of the code (e.g., the basic TCP/IP networking code)
has been in use for nearly 20 years.  In the early to mid-1990's,
after reading Bertrand Meyers' OBJECT-ORIENTED SOFTWARE CONSTRUCTION
(1st Edition) ( http://www.geonius.com/writing/other/oops.html ),
I reworked most of the APIs and internals to implement a simple
object-based approach to using the packages.

More documentation about the software as a whole can be found at:

	http://www.geonius.com/software/

The web pages for the individual packages basically echo the source
file prologs.

If you have any questions or find any problems, please let me know.
The software is free to use without asking; you can leave my name
in, take my name out, modify the software, whatever.  I appreciate
suggestions on how to improve or enhance the sofware - even
constructive criticism - but the likelihood of my implementing
suggested changes depends on the time I have available to do so
and on the number of programs I have that would be affected by
the changes.

				Alex Measday
				c.a.measday@ieee.org
				http://www.geonius.com/


CONTENTS
--------

The archive file ("csoft.zip" or "csoft.tgz") consists of the following
directories/software:

    include		- The "<prefix>_util.h" headers for LIBGPL.
    include/palm	- PalmOS-specific headers.
    include/rpc		- XDR headers.
    include/vms		- VAX/VMS-specific headers.

    libxdr		- Basic XDR library for platforms that need it.

    libgpl		- The "<prefix>_util.c" source for LIBGPL.

    anise		- A simple HTTP/FTP server based on LIBGPL.

    tools		- Miscellaneous test programs that use LIBGPL.


INSTALLATION
------------

Choose the correct distribution file for your platform:

    csoft.tgz = all text files have LF-terminated lines.

    csoft.zip - all text files have CR/LF-terminated lines.

Be sure and use "csoft.zip" on Windows; otherwise Visual C++ will croak on
the project and workspace files.  (The contents of the two archive files
are identical except for the line termination characters in text files.
Advanced users can work with either archive file by setting the appropriate
line-termination options when extracting the files.)

Extract the archive into a directory you've set up.  If your platform
(e.g., UNIX or LINUX) has an XDR/RPC library, get rid of CSOFT's
"include/rpc" and "libxdr" directories.


BUILD
-----

On Windows, open the "csoft.dsw" workspace file; there is a macro in
"csoft.dsm" that will build all of the libraries and programs in order.
The test programs are console applications, so screen output is abysmally
slow.  (My build and test environment is Windows 2K and Visual C++ 6.0.)

On other platforms, the different directories have Make and build files:

    Makefile.linux	- I've used Redhat 6.2, 7.3, and Fedora Core 2.
    Makefile.solaris	- If you've got GCC, you can comment out Sun's
			  compiler.  (Or comment it in depending on *my*
			  current configuration when I zipped the file!)
    Makefile.freebsd	- Every so often, I build, but don't test, CSOFT
			  in my web hosting service's FreeBSD shell account.
    Makefile.tcc	- Tiny C Compiler ( http://www.tinycc.org/ ).  The
			  compiler has a problem with bytes or shorts in
			  structs, which causes problems in my CORBA stuff.
			  Aside from that, it is an amazing compiler!
    Makefile.palm68k	- PalmOS using prc-tools.  See the PALMOS_OPTIONS
			  environment variable below.
    Makefile.nds	- Nintendo DS using devkitARM.
    vaxc.com		- DCL script that builds the library or programs
			  under VAX/VMS.  I used to have access to an old
			  VAXstation which croaked during a power outage
			  in Fall 2004; before then, I was able to build
			  and test the software, including the CORBA stuff.
			  I haven't had access to workstations running
			  newer versions of VMS and DEC C.

Go into the subdirectories and run "make -f Makefile.<platform>" or
"@vaxc.com".  For Nintendo DS, PalmOS, VAX/VMS, and Windows, you'll
need to build LIBXDR.

On UNIX/LINUX platforms, I usually construct a parallel directory of symbolic
links using "lndir" and build in that directory; for example:

    ~/src/include	- Source directories.
    ~/src/libgpl
    ...

    ~/obj/include	- Object directories containing links to source files.
    ~/obj/libgpl
    ~/obj/...

Under VMS, programs that expect a UNIX-like, argc/argv command line must be
defined in DCL as foreign commands.


Nintendo DS
===========

My command-line programs make use of libgpl/StdIONDS.c as the main program,
which then calls my programs' "main" routines.  See anise/Makefile.nds or
tools/Makefile.nds for the preprocessor magic that makes this possible.

(I'll expand this section when time and inclination permit!)


PalmOS
======

    NOTE:  Many of the EPOCH3 tools make use of my CORBA middleware
    implementation.  Early in 2006, the EPOCH3-specific IDL grew so large
    that one of the code sections overflowed and the CORBA-based EPOCH3
    tools would no longer build under PalmOS.  Base CORBA clients such as
    colior still build successfully.  Because of the code section overflow
    and the global variable problem described below, I haven't done any
    more PalmOS development since early 2006.  (Plus, as of August 2007,
    I'm still using an old Palm M105, mostly to read E-books I download
    from Project Gutenberg!)

PalmOS ... aarrgghh!  I built the software using the latest version of
prc-tools under Fedora Core 2.  Because of the Palm's restrictions on the
size of code and data regions, I had to resort to explicitly allocating
different LIBGPL/LIBEPL packages to separate, GCC object-code sections.
Throughout the header files and in some of the source files, you'll see
this allocation:

    OCD ("<prefix>_util")

The "OCD" macro is defined in "pragmatics.h" for the PalmOS platform.
What a pain!  To make matters worse, the Makefile has to actually
build each program twice, the first time to determine which sections
are actually needed and the second to time to build it without the
unused sections.

I build the programs as command-line StdIO applications that can be
run from the "Preferences->Network->Options->View Log" screen.
A couple of the programs I have actually downloaded and run on my
Palm M105, linked by a serial cable to my pppd-running Linux box.
Mostly, though, I use the Palm OS Emulator (POSE).

I've run into a problem with global variables and static variables.
Although disassembled object files show the variables being correctly
allocated and initialized, the linker seems to be mapping groups of
variables to the same memory location.  I've searched the news groups
and on-line forums for answers without success.  The start-up flags
for the programs show that the programs *do* have access to global
variables, but things are just not working correctly.


#*******************************************************************************
#
#    Environment variable PALM_OS_OPTIONS is passed to GCC
#    in the palm68k Makefiles.
#
#    -DHAVE_FLPBUFFERATOF=0
#        PalmOS 3.5 doesn't have the FlpBuffer* floating-point conversion
#        functions needed for GCC users.
#
#    -DPRE_VERSION_4
#        PalmOS 4 and later declare SioMain()'s ARGV argument as "const".
#        LIBGPL's SioMain.c uses this macro to avoid compilation errors.
#
#    -DI_DEFAULT_HANDLER=HostFPrintF
#    "-DI_DEFAULT_PARAMS=HostLogFile(),"
#        These definitions override the definitions in "pragmatics.h" and
#        cause debug output to be written to the PALM emulator's (POSE)
#        default log file.
#
#*******************************************************************************

setenv PALMOS_OPTIONS '-palmos3.5 -DHAVE_FLPBUFFERATOF=0 -DPRE_VERSION_4 -DI_DEFAULT_HANDLER=HostFPrintF "-DI_DEFAULT_PARAMS=HostLogFile(),"'
#setenv PALMOS_OPTIONS '-palmos4 -DI_DEFAULT_HANDLER=HostFPrintF "-DI_DEFAULT_PARAMS=HostLogFile(),"'
