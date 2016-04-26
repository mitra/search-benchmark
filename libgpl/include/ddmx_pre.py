# $Id: ddmx_pre.py,v 1.2 2004/08/09 21:06:11 alex Exp $
#


global gRenameMap


#    Add a prefix to the DCPSState enumeration values; "REGISTERED"
#    conflicts with a #define in Windows' "nb30.h" header file.

gRenameMap["DCPSState::INITIAL"]	= "DCPS_INITIAL"
gRenameMap["DCPSState::REGISTERED"]	= "DCPS_REGISTERED"
gRenameMap["DCPSState::ENABLED"]	= "DCPS_ENABLED"


	#    Marshaling function names are too long for VMS linker.
gRenameMap["DCPS::OfferedIncompatibleQosStatus"]	= "OfferedIncompatibleQosSt"
gRenameMap["DCPS::RequestedDeadlineMissedStatus"]	= "RequestedDeadlineMissedSt"
gRenameMap["DCPS::RequestedIncompatibleQosStatus"]	= "RequestedIncompatibleQosSt"
gRenameMap["DCPS::DestinationOrderQosPolicyKind"]	= "DestinationOrderQosPolKind"
gRenameMap["DCPS::PresentationQosPolicyAccessScopeKind"]	= "PresentationQosPolicyKind"
