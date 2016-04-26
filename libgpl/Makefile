# $Id$
#

GENERIC_SUBDIRS = \
	libxdr \
	libgpl \
	tools \
	anise

PROJECT_SUBDIRS =

SUBDIRS = $(GENERIC_SUBDIRS) $(PROJECT_SUBDIRS)

all::	children self

children:
	for subdir in $(SUBDIRS) ; do		\
		(cd $$subdir && $(MAKE)) ;	\
	done

self:
