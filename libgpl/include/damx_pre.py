# $Id: damx_pre.py,v 1.1 2004/07/08 01:14:18 alex Exp $
#


global gHookMap


#*******************************************************************************
#    Ignore types that are imported by hand from other modules.
#*******************************************************************************

gHookMap["DAFDescriptions::ResourceID"]			=	"ignore"
gHookMap["DAFDescriptions::URI"]			=	"ignore"

gHookMap["DAFQuery::ResourceID"]			=	"ignore"
gHookMap["DAFQuery::ResourceDescription"]		=	"ignore"
gHookMap["DAFQuery::ResourceDescriptionIterator"]	=	"ignore"
