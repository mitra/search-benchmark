# $Id: gimx_pre.py,v 1.5 2011/03/31 22:17:29 alex Exp $
#


global gAliasMap, gCDRMap, gCustomList, gEnumMap
global gHookMap, gMarshalMap, gRenameMap, gTypeMap


#    Non-CDR definitions.

gCDRMap["any"]		= "Any"		# Not really a CDR primitive type!

#    Custom marshaling functions (i.e., not automatically generated).

gCustomTypes = gCustomTypes + ["Any", "AnySeq", "ObjectKey", "ProfileBody",
                               "TaggedProfile", "Timeval", "TimevalSeq"]

gMarshalMap["Any"]			= "gimxAny"
gMarshalMap["AnySeq"]			= "gimxAnySeq"
gMarshalMap["ObjectKey"]		= "gimxObjectKey"
gMarshalMap["ProfileBody"]		= "gimxProfileBody"
gMarshalMap["TaggedProfile"]		= "gimxTaggedProfile"

gTypeMap["Timeval"]			= "None"
addAlias ("Timeval", "struct  timeval")
gMarshalMap["struct  timeval"]		= "gimxTimeval"

gTypeMap["TimevalSeq"]			= "None"
addAlias ("sequence<Timeval>", "TimevalSeq")
addAlias ("sequence<struct  timeval>", "TimevalSeq")
gMarshalMap["TimevalSeq"]		= "gimxTimevalSeq"

#    Ignore the following two enumeration types defined in
#    "CORBA_StandardExceptions.idl".  One of the compilers I use has trouble
#    with the "completion_status" type and a field of the same name in the
#    "GIOP::SystemExceptionReplyBody" structure.  The "exception_type"
#    enumerations conflict with the "GIOP::ReplyStatusType" enumerations.

gTypeMap["completion_status"]		= "None"
gTypeMap["exception_type"]		= "None"

#    Ignore the "MessageError" structure defined in "CORBAservices/SECIOP.idl";
#    it conflicts with an enumerated value of the same name in the
#    "GIOP::MsgType_1_1" enumeration.

gTypeMap["MessageError"]		= "None"

#    Rename "Kyes" in "CORBAservices/LifeCycleService.idl"; it conflicts
#    with a Nintendo DS definition in "libnds/include/nds/arm9/keyboard.h".

gRenameMap["LifeCycleService::Keys"]	= "COSLCSKeys"

#    Rename "EventType" in "CORBAservices/Security.idl"; it conflicts
#    with a PalmOS definition in "Core/UI/Event.h".

gRenameMap["Security::EventType"]	= "COSSEventType"

#    Rename some CosTrading types and enumerations whose names conflict
#    with core CORBA names.

gRenameMap["CosTrading::Policy"]	= "COSTPolicy"
gRenameMap["CosTrading::PolicySeq"]	= "COSTPolicySeq"
gRenameMap["HowManyProps::none"]	= "props_none"
gRenameMap["HowManyProps::some"]	= "props_some"
gRenameMap["HowManyProps::all"]		= "props_all"

#    Rename the latest versions of version-specific GIOP enumerated types
#    so that they have version-less type names.  Skip the earlier versions
#    of the types to prevent conflicting enumeration value names.

gEnumMap["LocateStatusType"]			= "None"
gTypeMap["LocateStatusType"]			= "None"
gRenameMap["GIOP::LocateStatusType_1_0"]	= "LocateStatusType"
gRenameMap["GIOP::LocateStatusType_1_2"]	= "LocateStatusType"

gEnumMap["GIOPMsgType"]				= "None"
gTypeMap["GIOPMsgType"]				= "None"
gRenameMap["GIOP::MsgType_1_0"]			= "GIOPMsgType"
gRenameMap["GIOP::MsgType_1_1"]			= "GIOPMsgType"
gRenameMap["GIOP::MsgType_1_2"]			= "GIOPMsgType"
gRenameMap["GIOP::MsgType_1_3"]			= "GIOPMsgType"

gEnumMap["ReplyStatusType"]			= "None"
gTypeMap["ReplyStatusType"]			= "None"
gRenameMap["GIOP::ReplyStatusType_1_0"]		= "ReplyStatusType"
gRenameMap["GIOP::ReplyStatusType_1_1"]		= "ReplyStatusType"
gRenameMap["GIOP::ReplyStatusType_1_2"]		= "ReplyStatusType"

#    Rename the latest versions of version-specific GIOP/IIOP structure types
#    so that they have version-less type names.  The earlier versions of the
#    types will be processed, with the version numbers remaining in their names.

gRenameMap["GIOP::FragmentHeader_1_2"]		= "FragmentHeader"
gRenameMap["GIOP::FragmentHeader_1_3"]		= "FragmentHeader"

gRenameMap["GIOP::LocateReplyHeader_1_2"]	= "LocateReplyHeader"
gRenameMap["GIOP::LocateReplyHeader_1_3"]	= "LocateReplyHeader"

gRenameMap["GIOP::LocateRequestHeader_1_2"]	= "LocateRequestHeader"
gRenameMap["GIOP::LocateRequestHeader_1_3"]	= "LocateRequestHeader"

gRenameMap["GIOP::MessageHeader_1_1"]		= "MessageHeader"
gRenameMap["GIOP::MessageHeader_1_2"]		= "MessageHeader"
gRenameMap["GIOP::MessageHeader_1_3"]		= "MessageHeader"

gRenameMap["IIOP::ProfileBody_1_0"]		= "ProfileBody"
gRenameMap["IIOP::ProfileBody_1_1"]		= "ProfileBody"

gRenameMap["GIOP::ReplyHeader_1_2"]		= "ReplyHeader"
gRenameMap["GIOP::ReplyHeader_1_3"]		= "ReplyHeader"

gRenameMap["GIOP::RequestHeader_1_2"]		= "RequestHeader"
gRenameMap["GIOP::RequestHeader_1_3"]		= "RequestHeader"

#    Rename constants that conflict with TAO definitions.

gRenameMap["CORBA::OMGVMCID"] = "CORBA_OMGVMCID"

gRenameMap["CORBA::BAD_POLICY"] = "CORBA_BAD_POLICY"
gRenameMap["CORBA::UNSUPPORTED_POLICY"] = "CORBA_UNSUPPORTED_POLICY"
gRenameMap["CORBA::BAD_POLICY_TYPE"] = "CORBA_BAD_POLICY_TYPE"
gRenameMap["CORBA::BAD_POLICY_VALUE"] = "CORBA_BAD_POLICY_VALUE"
gRenameMap["CORBA::UNSUPPORTED_POLICY_VALUE"] = "CORBA_UNSUPPORTED_POLICY_VALUE"

gRenameMap["CORBA::SecConstruction"] = "CORBA_SecConstruction"

gRenameMap["CORBA::PRIVATE_MEMBER"] = "CORBA_PRIVATE_MEMBER"
gRenameMap["CORBA::PUBLIC_MEMBER"] = "CORBA_PUBLIC_MEMBER"

gRenameMap["CORBA::VM_NONE"] = "CORBA_VM_NONE"
gRenameMap["CORBA::VM_CUSTOM"] = "CORBA_VM_CUSTOM"
gRenameMap["CORBA::VM_ABSTRACT"] = "CORBA_VM_ABSTRACT"
gRenameMap["CORBA::VM_TRUNCATABLE"] = "CORBA_VM_TRUNCATABLE"

gRenameMap["IOP::TAG_INTERNET_IOP"] = "IOP_TAG_INTERNET_IOP"
gRenameMap["IOP::TAG_MULTIPLE_COMPONENTS"] = "IOP_TAG_MULTIPLE_COMPONENTS"
gRenameMap["IOP::TAG_SCCP_IOP"] = "IOP_TAG_SCCP_IOP"

gRenameMap["IOP::TAG_ORB_TYPE"] = "IOP_TAG_ORB_TYPE"
gRenameMap["IOP::TAG_CODE_SETS"] = "IOP_TAG_CODE_SETS"
gRenameMap["IOP::TAG_POLICIES"] = "IOP_TAG_POLICIES"
gRenameMap["IOP::TAG_ALTERNATE_IIOP_ADDRESS"] = "IOP_TAG_ALTERNATE_IIOP_ADDRESS"
gRenameMap["IOP::TAG_ASSOCIATION_OPTIONS"] = "IOP_TAG_ASSOCIATION_OPTIONS"
gRenameMap["IOP::TAG_SEC_NAME"] = "IOP_TAG_SEC_NAME"
gRenameMap["IOP::TAG_SPKM_1_SEC_MECH"] = "IOP_TAG_SPKM_1_SEC_MECH"
gRenameMap["IOP::TAG_SPKM_2_SEC_MECH"] = "IOP_TAG_SPKM_2_SEC_MECH"
gRenameMap["IOP::TAG_KerberosV5_SEC_MECH"] = "IOP_TAG_KerberosV5_SEC_MECH"
gRenameMap["IOP::TAG_CSI_ECMA_Secret_SEC_MECH"] = "IOP_TAG_CSI_ECMA_Secret_SEC_MECH"
gRenameMap["IOP::TAG_CSI_ECMA_Hybrid_SEC_MECH"] = "IOP_TAG_CSI_ECMA_Hybrid_SEC_MECH"
gRenameMap["IOP::TAG_SSL_SEC_TRANS"] = "IOP_TAG_SSL_SEC_TRANS"
gRenameMap["IOP::TAG_CSI_ECMA_Public_SEC_MECH"] = "IOP_TAG_CSI_ECMA_Public_SEC_MECH"
gRenameMap["IOP::TAG_GENERIC_SEC_MECH"] = "IOP_TAG_GENERIC_SEC_MECH"
gRenameMap["IOP::TAG_FIREWALL_TRANS"] = "IOP_TAG_FIREWALL_TRANS"
gRenameMap["IOP::TAG_SCCP_CONTACT_INFO"] = "IOP_TAG_SCCP_CONTACT_INFO"
gRenameMap["IOP::TAG_JAVA_CODEBASE"] = "IOP_TAG_JAVA_CODEBASE"
gRenameMap["IOP::TAG_TRANSACTION_POLICY"] = "IOP_TAG_TRANSACTION_POLICY"
gRenameMap["IOP::TAG_MESSAGE_ROUTER"] = "IOP_TAG_MESSAGE_ROUTER"
gRenameMap["IOP::TAG_OTS_POLICY"] = "IOP_TAG_OTS_POLICY"
gRenameMap["IOP::TAG_INV_POLICY"] = "IOP_TAG_INV_POLICY"
gRenameMap["IOP::TAG_CSI_SEC_MECH_LIST"] = "IOP_TAG_CSI_SEC_MECH_LIST"
gRenameMap["IOP::TAG_NULL_TAG"] = "IOP_TAG_NULL_TAG"
gRenameMap["IOP::TAG_SECIOP_SEC_TRANS"] = "IOP_TAG_SECIOP_SEC_TRANS"
gRenameMap["IOP::TAG_TLS_SEC_TRANS"] = "IOP_TAG_TLS_SEC_TRANS"
gRenameMap["IOP::TAG_ACTIVITY_POLICY"] = "IOP_TAG_ACTIVITY_POLICY"
gRenameMap["IOP::TAG_COMPLETE_OBJECT_KEY"] = "IOP_TAG_COMPLETE_OBJECT_KEY"
gRenameMap["IOP::TAG_ENDPOINT_ID_POSITION"] = "IOP_TAG_ENDPOINT_ID_POSITION"
gRenameMap["IOP::TAG_LOCATION_POLICY"] = "IOP_TAG_LOCATION_POLICY"
gRenameMap["IOP::TAG_DCE_STRING_BINDING"] = "IOP_TAG_DCE_STRING_BINDING"
gRenameMap["IOP::TAG_DCE_BINDING_NAME"] = "IOP_TAG_DCE_BINDING_NAME"
gRenameMap["IOP::TAG_DCE_NO_PIPES"] = "IOP_TAG_DCE_NO_PIPES"
gRenameMap["IOP::TAG_DCE_SEC_MECH"] = "IOP_TAG_DCE_SEC_MECH"
gRenameMap["IOP::TAG_INET_SEC_TRANS"] = "IOP_TAG_INET_SEC_TRANS"

gRenameMap["IOP::TransactionService"] = "IOP_TransactionService"
gRenameMap["IOP::CodeSets"] = "IOP_CodeSets"
gRenameMap["IOP::ChainBypassCheck"] = "IOP_ChainBypassCheck"
gRenameMap["IOP::ChainBypassInfo"] = "IOP_ChainBypassInfo"
gRenameMap["IOP::LogicalThreadId"] = "IOP_LogicalThreadId"
gRenameMap["IOP::BI_DIR_IIOP"] = "IOP_BI_DIR_IIOP"
gRenameMap["IOP::SendingContextRunTime"] = "IOP_SendingContextRunTime"
gRenameMap["IOP::INVOCATION_POLICIES"] = "IOP_INVOCATION_POLICIES"
gRenameMap["IOP::FORWARDED_IDENTITY"] = "IOP_FORWARDED_IDENTITY"
gRenameMap["IOP::UnknownExceptionInfo"] = "IOP_UnknownExceptionInfo"
gRenameMap["IOP::RTCorbaPriority"] = "IOP_RTCorbaPriority"
gRenameMap["IOP::RTCorbaPriorityRange"] = "IOP_RTCorbaPriorityRange"
gRenameMap["IOP::FT_GROUP_VERSION"] = "IOP_FT_GROUP_VERSION"
gRenameMap["IOP::FT_REQUEST"] = "IOP_FT_REQUEST"
gRenameMap["IOP::ExceptionDetailMessage"] = "IOP_ExceptionDetailMessage"
gRenameMap["IOP::SecurityAttributeService"] = "IOP_SecurityAttributeService"
gRenameMap["IOP::ActivityService"] = "IOP_ActivityService"

gRenameMap["IOP::ENCODING_CDR_ENCAPS"] = "IOP_ENCODING_CDR_ENCAPS"

gRenameMap["CSI::OMGVMCID"] = "CSI_OMGVMCID"

gRenameMap["CSIIOP::TAG_SECIOP_SEC_TRANS"] = "IOP_TAG_SECIOP_SEC_TRANS"
gRenameMap["CSIIOP::TAG_TLS_SEC_TRANS"] = "IOP_TAG_TLS_SEC_TRANS"

gRenameMap["DCE_CIOPSecurity::TAG_DCE_SEC_MECH"] = "IOP_TAG_DCE_SEC_MECH"

gRenameMap["SECIOP::TAG_GENERIC_SEC_MECH"] = "IOP_TAG_GENERIC_SEC_MECH"
gRenameMap["SECIOP::TAG_ASSOCIATION_OPTIONS"] = "IOP_TAG_ASSOCIATION_OPTIONS"
gRenameMap["SECIOP::TAG_SEC_NAME"] = "IOP_TAG_SEC_NAME"
gRenameMap["SECIOP::TAG_SPKM_1_SEC_MECH"] = "IOP_TAG_SPKM_1_SEC_MECH"
gRenameMap["SECIOP::TAG_SPKM_2_SEC_MECH"] = "IOP_TAG_SPKM_2_SEC_MECH"
gRenameMap["SECIOP::TAG_KerberosV5_SEC_MECH"] = "IOP_TAG_KerberosV5_SEC_MECH"
gRenameMap["SECIOP::TAG_CSI_ECMA_Secret_SEC_MECH"] = "IOP_TAG_CSI_ECMA_Secret_SEC_MECH"
gRenameMap["SECIOP::TAG_CSI_ECMA_Hybrid_SEC_MECH"] = "IOP_TAG_CSI_ECMA_Hybrid_SEC_MECH"
gRenameMap["SECIOP::TAG_CSI_ECMA_Public_SEC_MECH"] = "IOP_TAG_CSI_ECMA_Public_SEC_MECH"

gRenameMap["Messaging::TRANSPARENT_REBIND"] = "MESSAGING_TRANSPARENT_REBIND"
gRenameMap["Messaging::NO_REBIND"] = "MESSAGING_NO_REBIND"
gRenameMap["Messaging::NO_RECONNECT"] = "MESSAGING_NO_RECONNECT"

gRenameMap["Messaging::SYNC_NONE"] = "MESSAGING_SYNC_NONE"
gRenameMap["Messaging::SYNC_WITH_TRANSPORT"] = "MESSAGING_SYNC_WITH_TRANSPORT"
gRenameMap["Messaging::SYNC_WITH_SERVER"] = "MESSAGING_SYNC_WITH_SERVER"
gRenameMap["Messaging::SYNC_WITH_TARGET"] = "MESSAGING_SYNC_WITH_TARGET"

gRenameMap["Messaging::ROUTE_NONE"] = "MESSAGING_ROUTE_NONE"
gRenameMap["Messaging::ROUTE_FORWARD"] = "MESSAGING_ROUTE_FORWARD"
gRenameMap["Messaging::ROUTE_STORE_AND_FORWARD"] = "MESSAGING_ROUTE_STORE_AND_FORWARD"

gRenameMap["Messaging::ORDER_ANY"] = "MESSAGING_ORDER_ANY"
gRenameMap["Messaging::ORDER_TEMPORAL"] = "MESSAGING_ORDER_TEMPORAL"
gRenameMap["Messaging::ORDER_PRIORITY"] = "MESSAGING_ORDER_PRIORITY"
gRenameMap["Messaging::ORDER_DEADLINE"] = "MESSAGING_ORDER_DEADLINE"

gRenameMap["Messaging::REBIND_POLICY_TYPE"] = "MESSAGING_REBIND_POLICY_TYPE"
gRenameMap["Messaging::SYNC_SCOPE_POLICY_TYPE"] = "MESSAGING_SYNC_SCOPE_POLICY_TYPE"
gRenameMap["Messaging::REQUEST_PRIORITY_POLICY_TYPE"] = "MESSAGING_REQUEST_PRIORITY_POLICY_TYPE"
gRenameMap["Messaging::REPLY_PRIORITY_POLICY_TYPE"] = "MESSAGING_REPLY_PRIORITY_POLICY_TYPE"
gRenameMap["Messaging::REQUEST_START_TIME_POLICY_TYPE"] = "MESSAGING_REQUEST_START_TIME_POLICY_TYPE"
gRenameMap["Messaging::REQUEST_END_TIME_POLICY_TYPE"] = "MESSAGING_REQUEST_END_TIME_POLICY_TYPE"
gRenameMap["Messaging::REPLY_START_TIME_POLICY_TYPE"] = "MESSAGING_REPLY_START_TIME_POLICY_TYPE"
gRenameMap["Messaging::REPLY_END_TIME_POLICY_TYPE"] = "MESSAGING_REPLY_END_TIME_POLICY_TYPE"
gRenameMap["Messaging::RELATIVE_REQ_TIMEOUT_POLICY_TYPE"] = "MESSAGING_RELATIVE_REQ_TIMEOUT_POLICY_TYPE"
gRenameMap["Messaging::RELATIVE_RT_TIMEOUT_POLICY_TYPE"] = "MESSAGING_RELATIVE_RT_TIMEOUT_POLICY_TYPE"
gRenameMap["Messaging::ROUTING_POLICY_TYPE"] = "MESSAGING_ROUTING_POLICY_TYPE"
gRenameMap["Messaging::MAX_HOPS_POLICY_TYPE"] = "MESSAGING_MAX_HOPS_POLICY_TYPE"
gRenameMap["Messaging::QUEUE_ORDER_POLICY_TYPE"] = "MESSAGING_QUEUE_ORDER_POLICY_TYPE"

#*******************************************************************************
#    declareAny() - outputs declarations for the CORBA Any and AnySeq types.
#*******************************************************************************

def declareAny ():
    global gLastPrint, gTypeMap

    print
    print "typedef  enum  TCKind {"
    print "    tk_null,"
    print "    tk_void,"
    print "    tk_short,"
    print "    tk_long,"
    print "    tk_ushort,"
    print "    tk_ulong,"
    print "    tk_float,"
    print "    tk_double,"
    print "    tk_boolean,"
    print "    tk_char,"
    print "    tk_octet,"
    print "    tk_any,"
    print "    tk_TypeCode,"
    print "    tk_Principal,"
    print "    tk_objref,"
    print "    tk_struct,"
    print "    tk_union,"
    print "    tk_enum,"
    print "    tk_string,"
    print "    tk_sequence,"
    print "    tk_array,"
    print "    tk_alias,"
    print "    tk_except,"
    print "    tk_longlong,"
    print "    tk_ulonglong,"
    print "    tk_longdouble,"
    print "    tk_wchar,"
    print "    tk_wstring,"
    print "    tk_fixed,"
    print "    tk_value,"
    print "    tk_value_box,"
    print "    tk_native,"
    print "    tk_abstract_interface,"
    print "    tk_local_interface,"
    print "    tk_component,"
    print "    tk_home,"
    print "    tk_event"
    print "}  TCKind ;"
    gLastPrint = "enum"
    gEnumMap["TCKind"] = "CORBA"
    gTypeMap["TCKind"] = "None"
    gHookMap["CORBA::TCKind"] = "ignore"

    print
    print "typedef  struct  Any {"
    print "    TCKind  which ;"
    print "    union  {"
    print "			/* tk_short */"
    print "        short  vShort ;"
    print "			/* tk_long */"
    print "        long  vLong ;"
    print "			/* tk_ushort */"
    print "        unsigned  short  vUShort ;"
    print "			/* tk_ulong */"
    print "        unsigned  long  vULong ;"
    print "			/* tk_float */"
    print "        float  vFloat ;"
    print "			/* tk_double */"
    print "        double  vDouble ;"
    print "			/* tk_boolean */"
    print "        bool  vBoolean ;"
    print "			/* tk_char */"
    print "        char  vChar ;"
    print "			/* tk_octet */"
    print "        octet  vOctet ;"
    print "			/* tk_TypeCode */"
    print "        TCKind  vTypeCode ;"
    print "			/* tk_Principal */"
    print "        OctetSeq  vPrincipal ;"
    print "			/* tk_string */"
    print "        char  *vString ;"
    print "			/* tk_longlong */"
    print "        LONGLONG  vLongLong ;"
    print "			/* tk_ulonglong */"
    print "        ULONGLONG  vULongLong ;"
    print "			/* tk_longdouble */"
    print "        LONGDOUBLE  vLongDouble ;"
    print "			/* tk_wchar */"
    print "        wchar_t  vWChar ;"
    print "			/* tk_wstring */"
    print "        wchar_t  *vWString ;"
    print "			/* 0xFFFFFFFF */"
    print "        long  vIndirection ;"
    print "			/* tk_null */"
    print "			/* tk_void */"
    print "			/* tk_any */"
    print "			/* tk_objref */"
    print "			/* tk_struct */"
    print "			/* tk_union */"
    print "			/* tk_enum */"
    print "			/* tk_sequence */"
    print "			/* tk_array */"
    print "			/* tk_alias */"
    print "			/* tk_except */"
    print "			/* tk_fixed */"
    print "			/* tk_value */"
    print "			/* tk_value_box */"
    print "			/* tk_native */"
    print "			/* tk_abstract_interface */"
    print "			/* tk_local_interface */"
    print "    }  data ;"
    print "}  Any ;"
    print
    print "typedef  SEQUENCE (Any, AnySeq) ;"
    gLastPrint = "struct"
    gTypeMap["Any"] = "CORBA"
    gTypeMap["AnySeq"] = "CORBA"
    return

#*******************************************************************************
#    declareIOR() - outputs declarations for the CORBA IOR and related types.
#*******************************************************************************

def declareIOR ():
    global gLastPrint, gTypeMap

    print
    print "/* Module: IOP */"

    print
    print "typedef  unsigned  long  ComponentId ;"
    gTypeMap["ComponentId"] = "IOP"
    gHookMap["IOP::ComponentId"] = "ignore"

    print
    print "typedef  struct  TaggedComponent {"
    print "    ComponentId  tag ;"
    print "    OctetSeq  component_data ;"
    print "}  TaggedComponent ;"
    gTypeMap["TaggedComponent"] = "IOP"
    gHookMap["IOP::TaggedComponent"] = "ignore"

    print
    print "typedef  SEQUENCE (TaggedComponent, MultipleComponentProfile) ;"
    gTypeMap["MultipleComponentProfile"] = "IOP"
    gHookMap["IOP::MultipleComponentProfile"] = "ignore"

    print
    print "typedef  struct  ProfileBody {"
    print "    Version  iiop_version ;"
    print "    char  *host ;"
    print "    unsigned  short  port ;"
    print "    ObjectKey  object_key ;"
    print "    SEQUENCE (TaggedComponent, components) ;	/* Ignore in IIOP 1.0! */"
    print "}  ProfileBody ;"
    gTypeMap["ProfileBody"] = "IOP"

    print
    print "typedef  unsigned  long  ProfileId ;"
    gTypeMap["ProfileId"] = "IOP"
    gHookMap["IOP::ProfileId"] = "ignore"

    print
    print "typedef  struct  TaggedProfile {"
    print "    ProfileId  which ;"
    print "    union  {"
    print "			/* TAG_INTERNET_IOP */"
    print "        ProfileBody  iiop_body ;"
    print "			/* TAG_MULTIPLE_COMPONENTS */"
    print "        MultipleComponentProfile  components ;"
    print "			/* <default> */"
    print "        OctetSeq  profile_data ;"
    print "    }  data ;"
    print "}  TaggedProfile ;"
    gTypeMap["TaggedProfile"] = "IOP"
    gHookMap["IOP::TaggedProfile"] = "ignore"

    print
    print "typedef  struct  IOR {"
    print "    char  *type_id ;"
    print "    SEQUENCE (TaggedProfile, profiles) ;"
    print "}  IOR ;"
    gTypeMap["IOR"] = "IOP"
    gHookMap["IOP::IOR"] = "ignore"

    print
    print "typedef  IOR  Object ;"
    gLastPrint = "struct"
    addAlias ("Object", "IOR")
    gTypeMap["Object"] = "IOP"
    gHookMap["IOP::Object"] = "ignore"

    return

#*******************************************************************************
#    declareGIOPHeader() - outputs declarations for earlier versions of
#        GIOP request and reply headers.
#*******************************************************************************

def declareGIOPHeader (name):
    global gLastPrint, gTypeMap

    if name == "GIOP::LocateReplyHeader_1_2":
        print
        print "typedef  struct  LocateReplyHeader_1_0 {"
        print "    unsigned  long  request_id ;"
        print "    LocateStatusType  locate_status ;"
        print "}  LocateReplyHeader_1_0 ;"
        print
        print "typedef  LocateReplyHeader_1_0  LocateReplyHeader_1_1 ;"
        gLastPrint = "struct"
        gTypeMap["LocateReplyHeader_1_0"] = "GIOP"
        addAlias ("LocateReplyHeader_1_1", "LocateReplyHeader_1_0")

    elif name == "GIOP::ReplyHeader_1_2":
        print
        print "typedef  struct  ReplyHeader_1_0 {"
        print "    ServiceContextList  service_context ;"
        print "    unsigned  long  request_id ;"
        print "    ReplyStatusType  reply_status ;"
        print "}  ReplyHeader_1_0 ;"
        print
        print "typedef  ReplyHeader_1_0  ReplyHeader_1_1 ;"
        gLastPrint = "struct"
        gTypeMap["ReplyHeader_1_0"] = "GIOP"
        addAlias ("ReplyHeader_1_1", "ReplyHeader_1_0")

    return ""

#*******************************************************************************
#    Pre-declare types and such that are not handled as desired in the IDL.
#*******************************************************************************

print "/*******************************************************************************"
print "    Declarations for Any, IOR, and IOR-related types."
print "*******************************************************************************/"

declareAny ()
declareIOR ()

print "\f"

#*******************************************************************************
#    Insert old versions of GIOP headers that are #ifdef'ed out.
#*******************************************************************************

gHookMap["GIOP::LocateReplyHeader_1_2"] = declareGIOPHeader
gHookMap["GIOP::ReplyHeader_1_2"] = declareGIOPHeader
