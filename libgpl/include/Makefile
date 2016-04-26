# $Id: Makefile,v 1.2 2009/09/23 13:25:02 alex Exp $
#

IDL_DIR = ../idl

IDL_CPPFLAGS = \
	-D_COMPONENT_REPOSITORY_ \
	-DGIOP_1_2 \
	-DNO_ESCAPED_IDENTIFIERS \
	-D_PRE_3_0_COMPILER_ \
	-I$(IDL_DIR) \
	-I$(IDL_DIR)/CORBA_IDL \
	-I$(IDL_DIR)/CORBAservices \
	-I$(IDL_DIR)/Interop \
	-I$(IDL_DIR)/Misc

#    CORBA IDL.

CORBA_IDL = gimx_all.idl
CORBA_IDL_H = gimx_idl.h.new

#    GNOME Bonobo IDL.

BONOBO_IDL = \
	$(IDL_DIR)/GNOME/bonobo-2.0/Bonobo.idl \
	$(IDL_DIR)/GNOME/bonobo-activation-2.0/Bonobo_ObjectDirectory.idl
BONOBO_IDL_H = bomx_idl.h.new

#    Data Acquisition from Industrial Systems (DAIS) IDL.

DAIS_IDL = damx_all.idl
DAIS_IDL_H = damx_idl.h.new

#    Data Distribution Service for Real-Time Systems (DDS) IDL.

DDS_IDL = $(IDL_DIR)/DDS/dds_dlrl.idl
DDS_IDL_H = ddmx_idl.h.new

#    Laboratory Equipment Control Interface Specification (LECIS) IDL.

LECIS_IDL = $(IDL_DIR)/LECIS/LECIS.idl
LECIS_IDL_H = lemx_idl.h.new

IDL2H = idl2h.py

all::	$(CORBA_IDL_H) $(DAIS_IDL_H) $(DDS_IDL_H) $(LECIS_IDL_H)

$(CORBA_IDL_H): $(CORBA_IDL) $(IDL2H)
	-$(RM) $@
	$(IDL2H) --cppCommand "gcc -E -nostdinc $(IDL_CPPFLAGS)" --include --preDefs comx_post.py --preDefs gimx_pre.py --postDefs gimx_post.py --prefix gimx $(CORBA_IDL) > $@

$(BONOBO_IDL_H): $(BONOBO_IDL) $(IDL2H)
	-$(RM) $@
	$(IDL2H) --cppCommand "gcc -E -nostdinc $(IDL_CPPFLAGS) -I$(IDL_DIR)/GNOME/bonobo-2.0 -I$(IDL_DIR)/GNOME/bonobo-activation-2.0" --include --preDefs gimx_post.py --preDefs bomx_pre.py --postDefs bomx_post.py --prefix bomx $(BONOBO_IDL) > $@

$(DAIS_IDL_H): $(DAIS_IDL) $(IDL2H)
	-$(RM) $@
	$(IDL2H) --cppCommand "gcc -E -nostdinc $(IDL_CPPFLAGS) -I$(IDL_DIR)/DAIS" --include --preDefs gimx_post.py --preDefs damx_pre.py --postDefs damx_post.py --prefix damx $(DAIS_IDL) > $@

$(DDS_IDL_H): $(DDS_IDL) $(IDL2H)
	-$(RM) $@
	$(IDL2H) --cppCommand "gcc -E -nostdinc $(IDL_CPPFLAGS) -I$(IDL_DIR)/DDS" --include --preDefs gimx_post.py --preDefs ddmx_pre.py --postDefs ddmx_post.py --prefix ddmx $(DDS_IDL) > $@

$(LECIS_IDL_H): $(LECIS_IDL) $(IDL2H)
	-$(RM) $@
	$(IDL2H) --cppCommand "gcc -E -nostdinc $(IDL_CPPFLAGS)" --include --preDefs gimx_post.py --postDefs lemx_post.py --prefix lemx $(LECIS_IDL) > $@

clean::
	-$(RM) $(CORBA_IDL_H) $(DAIS_IDL_H) $(DDS_IDL_H) $(LECIS_IDL_H)
