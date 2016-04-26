# $Id$
#

PROGRAMS = \
	colior dirtree dump duop ffc frob gflow hls \
	pjoin rdate rename scanet talknet wst

ROOT = ..
INCLUDE_DIR = $(ROOT)/include
LIBS = \
	$(ROOT)/libgpl/libgpl.a

INSTALL_DIR = $(HOME)/local/bin/$(ARCH)

AR = ar
ARFLAGS = rv
CFLAGS = -g
CPPFLAGS = \
	-DHAVE_STDBOOL_H=0 \
	-I. \
	-I$(ROOT)/include
LINK.c = $(CC)
LDFLAGS = -g
RM = rm -f

all::	$(PROGRAMS)

colior:  colior.o
	$(LINK.c) $(LDFLAGS) -o $@ colior.o $(LIBS)

dirtree:  dirtree.o
	$(LINK.c) $(LDFLAGS) -o $@ dirtree.o $(LIBS)

dump:  dump.o
	$(LINK.c) $(LDFLAGS) -o $@ dump.o $(LIBS)

duop:  duop.o
	$(LINK.c) $(LDFLAGS) -o $@ duop.o $(LIBS)

ffc:  ffc.o
	$(LINK.c) $(LDFLAGS) -o $@ ffc.o $(LIBS)

frob:  frob.o
	$(LINK.c) $(LDFLAGS) -o $@ frob.o $(LIBS)

gflow:  gflow.o
	$(LINK.c) $(LDFLAGS) -o $@ gflow.o $(LIBS)

hls:  hls.o
	$(LINK.c) $(LDFLAGS) -o $@ hls.o $(LIBS)

pjoin:  pjoin.o
	$(LINK.c) $(LDFLAGS) -o $@ pjoin.o $(LIBS)

rdate:  rdate.o
	$(LINK.c) $(LDFLAGS) -o $@ rdate.o $(LIBS)

rename:  rename.o
	$(LINK.c) $(LDFLAGS) -o $@ rename.o $(LIBS)

scanet:  scanet.o
	$(LINK.c) $(LDFLAGS) -o $@ scanet.o $(LIBS)

talknet:  talknet.o
	$(LINK.c) $(LDFLAGS) -o $@ talknet.o $(LIBS)

wst:  wst.o
	$(LINK.c) $(LDFLAGS) -o $@ wst.o $(LIBS)

clean::
	-$(RM) $(PROGRAMS)
	-$(RM) *.o

install:
	cp $(PROGRAMS) $(INSTALL_DIR)
