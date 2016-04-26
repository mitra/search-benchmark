# $Id: Makefile.tcc,v 1.1 2005/01/10 19:31:55 alex Exp alex $
#

PROGRAMS = finc nicl

LIBRARY = libfinc.a

SRCS = \
	words_drs.c \
	words_lfn.c \
	words_misc.c \
	words_net.c \
	words_iox.c \
	words_skt.c \
	words_tcp.c \
	words_tv.c

OBJS = $(SRCS:.c=.o)

ROOT = ..
LIBS = \
	$(ROOT)/libgpl/libgpl.a \
	-lficl -lm
INSTALL_DIR = $(HOME)/local/bin/$(arch)

ARFLAGS = rv
CC = tcc
CFLAGS = -g
CPPFLAGS = \
	-DHAVE_STDBOOL_H=0 \
	-I. \
	-I$(ROOT)/include \
	-I/usr/local/include/ficl
LINK.c = $(CC)
LDFLAGS = -g
RANLIB = ranlib
RM = rm -f

all::	$(LIBRARY) $(PROGRAMS)

finc: finc.o $(LIBRARY)
	$(LINK.c) $(LDFLAGS) -o $@ finc.o $(LIBRARY) $(LIBS)

nicl: nicl.o $(LIBRARY)
	$(LINK.c) $(LDFLAGS) -o $@ nicl.o $(LIBRARY) $(LIBS)

$(LIBRARY): $(OBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $@ $(OBJS)
	$(RANLIB) $@

clean::
	-$(RM) *.o $(LIBRARY) $(PROGRAMS)

install:
	cp $(PROGRAMS) $(INSTALL_DIR)
