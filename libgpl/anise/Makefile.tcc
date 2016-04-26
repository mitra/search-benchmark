# $Id: Makefile.tcc,v 1.2 2004/01/17 01:24:34 alex Exp alex $
#

PROGRAM = anise

SRCS = \
	anise.c \
	ftpClient.c \
	http_util.c \
	passClient.c \
	wwwClient.c

OBJS = $(SRCS:.c=.o)

ROOT = ..
LIBS = \
	$(ROOT)/libgpl/libgpl.a \
	-lgdbm

CC = tcc
CFLAGS = -g
CPPFLAGS = \
	-I. \
	-I$(ROOT)/include
LINK.c = $(CC)

all::	$(PROGRAM)

$(PROGRAM): $(OBJS)
	$(LINK.c) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean::
	-$(RM) $(OBJS) $(PROGRAM)
