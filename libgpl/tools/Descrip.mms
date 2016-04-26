# descrip.mms   - A VMS makefile
#
# This makefile builds all the tools executables
#
# created 11/30/92 for XSAR by John Chapman Buell (CSC)
# modified 04/21/93 for XSAR by Bill Keiper to add EXTRA_CCFLAGS variable
#
CCFLAGS = /DEBUG=ALL/OPT
EXTRA_CCFLAGS = /DEFINES=(__STDC__)/STANDARDS=PORTABLE

INCLUDES = /INCLUDE=([],[INCLUDE])

LINKFLAGS =

LIBRARY = -
	[LIB.LIBDS]DS/LIB, -
	[LIB.LIBUTILGEN]UTILGEN/LIB, -
	[LIB.LIBUTILVMS]UTILVMS/LIB, -
	[LIB.LIBUTILGEN]UTILGEN/LIB, -
	SYS$LIBRARY:UCX$IPC/LIB, -
	SYS$LIBRARY:VAXCRTL/LIB

all : -
	events_reader.exe -
	ffc.exe -
	maple.exe -
	netecho.exe -
	prolog.exe -
	prosper.exe -
	sdp.exe -
	syncsrc.exe -
	talknet.exe -
	wst.exe
	!

clean :
	delete *.exe;*
	delete *.obj;*
	!

install :
	copy/log *.exe xsar$images:

events_reader.exe :  events_reader.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) events_reader.c
	link $(LINKFLAGS) events_reader.obj, $(LIBRARY)

ffc.exe :  ffc.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) ffc.c
	link $(LINKFLAGS) ffc.obj, $(LIBRARY)

maple.exe :  maple.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) maple.c
	link $(LINKFLAGS) maple.obj, $(LIBRARY)

netecho.exe :  netecho.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) netecho.c
	link $(LINKFLAGS) netecho.obj, $(LIBRARY)

prolog.exe :  prolog.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) prolog.c
	link $(LINKFLAGS) prolog.obj, $(LIBRARY)

prosper.exe :  prosper.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) prosper.c
	link $(LINKFLAGS) prosper.obj, $(LIBRARY)

sdp.exe :  sdp.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) sdp.c
	link $(LINKFLAGS) sdp.obj, $(LIBRARY), SYS$LIBRARY:VAXCCURSE/LIB

syncsrc.exe :  syncsrc.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) syncsrc.c
	link $(LINKFLAGS) syncsrc.obj, $(LIBRARY)

talknet.exe :  talknet.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) talknet.c
	link $(LINKFLAGS) talknet.obj, $(LIBRARY)

wst.exe :  wst.c
	cc $(CCFLAGS)$(EXTRA_CCFLAGS) $(INCLUDES) wst.c
	link $(LINKFLAGS) wst.obj, $(LIBRARY)


#*******************************************************************************
#    Dependencies.
#*******************************************************************************

events_reader.exe :	[include]fd.h
events_reader.exe :	[include]getopt.h
events_reader.exe :	[include]strm_svr.h
events_reader.exe :	[include]event_msg.h
ffc.exe :	[include]vmsparam.h
ffc.exe :	[include]getopt.h
ffc.exe :	[include]fparse.h
maple.exe :	[include]vmstypes.h
maple.exe :	[include]getopt.h
netecho.exe :	[include]getopt.h
prolog.exe :	[include]getopt.h
prosper.exe :	[include]getopt.h
prosper.exe :	[include]libutilgen.h
prosper.exe :	[include]libutilvms.h
sdp.exe :	[include]cv.h
sdp.exe :	[include]status_flags.h
sdp.exe :	[include]tag_struct.h
sdp.exe :	[include]getopt.h
sdp.exe :	[include]vterm.h
syncsrc.exe :	[include]getopt.h
syncsrc.exe :	[include]cv.h
syncsrc.exe :	[include]ds_size.h
syncsrc.exe :	[include]status_flags.h
syncsrc.exe :	[include]strm_svr.h
syncsrc.exe :	[include]tag_struct.h
talknet.exe :	[include]getopt.h
wst.exe :	[include]getopt.h
