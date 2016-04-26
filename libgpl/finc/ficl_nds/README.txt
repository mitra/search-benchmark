
                     Building Ficl for the Nintendo DS
                     ---------------------------------

These are the files you need to build the Ficl library and the FICL stand-alone
intepreter for the Nintendo DS.

(1) Download the Ficl distribution and extract it to your target directory.

(2) Be sure that environmant variables DEVKITPRO and DEVKITARM are defined
    for your devkitPro toolchain.

(3) Drop "Makefile.nds" and "StdIONDS.c" into your Ficl directory.  (NOTE
    that this "StdIONDS.c" is a simplified version of the file used by the
    FINC executables.)

(4) Run "make -f Makefile.nds".  This will build the Ficl library, "libficl.a",
    and the FICL executable, "ficl.nds".  Ignore the many warnings generated
    while building the Ficl library.

You're done!  When you run "ficl.nds" on your Nintendo DS, it will look for
a configuration file, "/etc/ficl.conf".  FICL only allows a single command-line
argument - the name of a Forth file to read and interpret.  If you have such
a file, the configuration file will consist of a single line:

    ficl <file>

(The Forth file and "ficl.nds" should be in the same directory.)  If there is
no configuration file, a keyboard will pop up and you will be prompted for the
command-line arguments.  Don't enter any and you will be taken to Ficl's
command-line prompt, "ok> ".  Go crazy typing in Forth commands!
