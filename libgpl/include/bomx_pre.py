# $Id: bomx_pre.py,v 1.2 2011/07/18 17:55:53 alex Exp $
#


global gAliasMap, gCDRMap, gCustomList, gEnumMap
global gHookMap, gMarshalMap, gRenameMap, gTypeMap


# NORMAL conflicts with one of the standard CORBA definitions, so rename
# all of the Gdk::CrossMode enumeration values.

gRenameMap["CrossMode::NORMAL"] = "GDK_NORMAL"
gRenameMap["CrossMode::GRAB"] = "GDK_GRAB"
gRenameMap["CrossMode::UNGRAB"] = "GDK_UNGRAB"


# Gdk's "Time" type conflicts with a SelectoryType constant in CORBA's
# "Security.idl".

gRenameMap["Gdk::Time"] = "GdkTime"


# Bonobo's "PropertySet" type conflicts with a type of the same name
# in CORBA's CosPropertyService.

gRenameMap["Bonobo::PropertySet"] = "BonoboPropertySet"


# WRITE conflicts with one of the standard CORBA definitions, so rename
# all of the Bonobo::DBFlags enumeration values.

gRenameMap["DBFlags::DEFAULT"] = "DBF_DEFAULT"
gRenameMap["DBFlags::WRITE"] = "DBF_WRITE"
gRenameMap["DBFlags::MANDATORY"] = "DBF_MANDATORY"


# WRITE conflicts with one of the standard CORBA definitions, so rename
# all of the Bonobo::Storage::Unknown OpenMode constants.

gRenameMap["Bonobo::READ"] = "BONOBO_READ"
gRenameMap["Bonobo::WRITE"] = "BONOBO_WRITE"
gRenameMap["Bonobo::CREATE"] = "BONOBO_CREATE"
gRenameMap["Bonobo::FAILIFEXIST"] = "BONOBO_FAILIFEXIST"
gRenameMap["Bonobo::COMPRESSED"] = "BONOBO_COMPRESSED"
gRenameMap["Bonobo::TRANSACTED"] = "BONOBO_TRANSACTED"


# "int32" conflicts with a Nintendo DS type of the same name.

gRenameMap["Canvas::int32"] = "Int32"


#*******************************************************************************
#    declareGdkEvent() - declare the Gdk::Event union type; "idl2h.py" doesn't
#        handle the qualified enumerated types - yet.
#*******************************************************************************

def declareGdkEventUnion (name):
    global gLastPrint, gTypeMap

    print
    print "typedef  struct  Event {"
    print "    EventType  which ;"
    print "    union {"
    print "			/* FOCUS */"
    print "        FocusEvent  focus ;"
    print "			/* KEY */"
    print "        KeyEvent  key ;"
    print "			/* MOTION */"
    print "        MotionEvent  motion ;"
    print "			/* BUTTON */"
    print "        ButtonEvent  button ;"
    print "			/* CROSSING */"
    print "        CrossingEvent  crossing ;"
    print "    }  data ;"
    print "}  Event ;"

    gLastPrint = "union"
    gTypeMap["Event"] = "Gdk"

    return "ignore"


gHookMap["Gdk::Event"] = declareGdkEventUnion
