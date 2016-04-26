/* $Id: gimx_idl.h,v 1.15 2011/03/31 22:17:29 alex Exp $ */
/*******************************************************************************
    Declarations for Any, IOR, and IOR-related types.
*******************************************************************************/

typedef  enum  TCKind {
    tk_null,
    tk_void,
    tk_short,
    tk_long,
    tk_ushort,
    tk_ulong,
    tk_float,
    tk_double,
    tk_boolean,
    tk_char,
    tk_octet,
    tk_any,
    tk_TypeCode,
    tk_Principal,
    tk_objref,
    tk_struct,
    tk_union,
    tk_enum,
    tk_string,
    tk_sequence,
    tk_array,
    tk_alias,
    tk_except,
    tk_longlong,
    tk_ulonglong,
    tk_longdouble,
    tk_wchar,
    tk_wstring,
    tk_fixed,
    tk_value,
    tk_value_box,
    tk_native,
    tk_abstract_interface,
    tk_local_interface,
    tk_component,
    tk_home,
    tk_event
}  TCKind ;

typedef  struct  Any {
    TCKind  which ;
    union  {
			/* tk_short */
        short  vShort ;
			/* tk_long */
        long  vLong ;
			/* tk_ushort */
        unsigned  short  vUShort ;
			/* tk_ulong */
        unsigned  long  vULong ;
			/* tk_float */
        float  vFloat ;
			/* tk_double */
        double  vDouble ;
			/* tk_boolean */
        bool  vBoolean ;
			/* tk_char */
        char  vChar ;
			/* tk_octet */
        octet  vOctet ;
			/* tk_TypeCode */
        TCKind  vTypeCode ;
			/* tk_Principal */
        OctetSeq  vPrincipal ;
			/* tk_string */
        char  *vString ;
			/* tk_longlong */
        LONGLONG  vLongLong ;
			/* tk_ulonglong */
        ULONGLONG  vULongLong ;
			/* tk_longdouble */
        LONGDOUBLE  vLongDouble ;
			/* tk_wchar */
        wchar_t  vWChar ;
			/* tk_wstring */
        wchar_t  *vWString ;
			/* 0xFFFFFFFF */
        long  vIndirection ;
			/* tk_null */
			/* tk_void */
			/* tk_any */
			/* tk_objref */
			/* tk_struct */
			/* tk_union */
			/* tk_enum */
			/* tk_sequence */
			/* tk_array */
			/* tk_alias */
			/* tk_except */
			/* tk_fixed */
			/* tk_value */
			/* tk_value_box */
			/* tk_native */
			/* tk_abstract_interface */
			/* tk_local_interface */
    }  data ;
}  Any ;

typedef  SEQUENCE (Any, AnySeq) ;

/* Module: IOP */

typedef  unsigned  long  ComponentId ;

typedef  struct  TaggedComponent {
    ComponentId  tag ;
    OctetSeq  component_data ;
}  TaggedComponent ;

typedef  SEQUENCE (TaggedComponent, MultipleComponentProfile) ;

typedef  struct  ProfileBody {
    Version  iiop_version ;
    char  *host ;
    unsigned  short  port ;
    ObjectKey  object_key ;
    SEQUENCE (TaggedComponent, components) ;	/* Ignore in IIOP 1.0! */
}  ProfileBody ;

typedef  unsigned  long  ProfileId ;

typedef  struct  TaggedProfile {
    ProfileId  which ;
    union  {
			/* TAG_INTERNET_IOP */
        ProfileBody  iiop_body ;
			/* TAG_MULTIPLE_COMPONENTS */
        MultipleComponentProfile  components ;
			/* <default> */
        OctetSeq  profile_data ;
    }  data ;
}  TaggedProfile ;

typedef  struct  IOR {
    char  *type_id ;
    SEQUENCE (TaggedProfile, profiles) ;
}  IOR ;

typedef  IOR  Object ;

/*******************************************************************************
    gimx_all.idl
*******************************************************************************/

/* Module: CORBA */

typedef  IOR  ConstructionPolicy ;
typedef  IOR  DomainManager ;
typedef  IOR  Policy ;
typedef  IOR  AbstractInterfaceDef ;
typedef  IOR  AliasDef ;
typedef  IOR  ArrayDef ;
typedef  IOR  AttributeDef ;
typedef  IOR  ConstantDef ;
typedef  IOR  Contained ;
typedef  IOR  Container ;
typedef  IOR  EnumDef ;
typedef  IOR  ExceptionDef ;
typedef  IOR  ExtInterfaceDef ;
typedef  IOR  ExtValueDef ;
typedef  IOR  ExtAbstractInterfaceDef ;
typedef  IOR  ExtLocalInterfaceDef ;
typedef  IOR  FixedDef ;
typedef  IOR  IDLType ;
typedef  IOR  InterfaceDef ;
typedef  IOR  IRObject ;
typedef  IOR  LocalInterfaceDef ;
typedef  IOR  ModuleDef ;
typedef  IOR  NativeDef ;
typedef  IOR  OperationDef ;
typedef  IOR  PrimitiveDef ;
typedef  IOR  Repository ;
typedef  IOR  SequenceDef ;
typedef  IOR  StringDef ;
typedef  IOR  StructDef ;
typedef  IOR  TypeCode ;
typedef  IOR  TypedefDef ;
typedef  IOR  UnionDef ;
typedef  IOR  ValueDef ;
typedef  IOR  ValueBoxDef ;
typedef  IOR  ValueMemberDef ;
typedef  IOR  WstringDef ;

typedef  char  *Identifier ;

#define  CORBA_OMGVMCID  (0x4f4d0000UL)

typedef  unsigned  long  PolicyType ;

typedef  SEQUENCE (Policy, PolicyList) ;

typedef  SEQUENCE (PolicyType, PolicyTypeSeq) ;

typedef  short  PolicyErrorCode ;

#define  CORBA_BAD_POLICY  (0)
#define  CORBA_UNSUPPORTED_POLICY  (1)
#define  CORBA_BAD_POLICY_TYPE  (2)
#define  CORBA_BAD_POLICY_VALUE  (3)
#define  CORBA_UNSUPPORTED_POLICY_VALUE  (4)
#define  CORBA_SecConstruction  (11)

typedef  SEQUENCE (DomainManager, DomainManagersList) ;

typedef  char  *ScopedName ;

typedef  char  *RepositoryId ;

typedef  enum  DefinitionKind {
    dk_none,
    dk_all,
    dk_Attribute,
    dk_Constant,
    dk_Exception,
    dk_Interface,
    dk_Module,
    dk_Operation,
    dk_Typedef,
    dk_Alias,
    dk_Struct,
    dk_Union,
    dk_Enum,
    dk_Primitive,
    dk_String,
    dk_Sequence,
    dk_Array,
    dk_Repository,
    dk_Wstring,
    dk_Fixed,
    dk_Value,
    dk_ValueBox,
    dk_ValueMember,
    dk_Native,
    dk_AbstractInterface,
    dk_LocalInterface,
    dk_Component,
    dk_Home,
    dk_Factory,
    dk_Finder,
    dk_Emits,
    dk_Publishes,
    dk_Consumes,
    dk_Provides,
    dk_Uses,
    dk_Event
}  DefinitionKind ;

typedef  char  *VersionSpec ;

typedef  struct  Description {
    DefinitionKind  kind ;
    Any  value ;
}  Description ;

typedef  SEQUENCE (InterfaceDef, InterfaceDefSeq) ;

typedef  SEQUENCE (ValueDef, ValueDefSeq) ;

typedef  SEQUENCE (AbstractInterfaceDef, AbstractInterfaceDefSeq) ;

typedef  SEQUENCE (LocalInterfaceDef, LocalInterfaceDefSeq) ;

typedef  SEQUENCE (ExtInterfaceDef, ExtInterfaceDefSeq) ;

typedef  SEQUENCE (ExtValueDef, ExtValueDefSeq) ;

typedef  SEQUENCE (ExtAbstractInterfaceDef, ExtAbstractInterfaceDefSeq) ;

typedef  SEQUENCE (ExtLocalInterfaceDef, ExtLocalInterfaceDefSeq) ;

typedef  SEQUENCE (Contained, ContainedSeq) ;

typedef  struct  StructMember {
    Identifier  name ;
    TypeCode  type ;
    IDLType  type_def ;
}  StructMember ;

typedef  SEQUENCE (StructMember, StructMemberSeq) ;

typedef  struct  Initializer {
    StructMemberSeq  members ;
    Identifier  name ;
}  Initializer ;

typedef  SEQUENCE (Initializer, InitializerSeq) ;

typedef  struct  UnionMember {
    Identifier  name ;
    Any  label ;
    TypeCode  type ;
    IDLType  type_def ;
}  UnionMember ;

typedef  struct  ExceptionDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    TypeCode  type ;
}  ExceptionDescription ;

typedef  SEQUENCE (ExceptionDescription, ExcDescriptionSeq) ;

typedef  struct  ExtInitializer {
    StructMemberSeq  members ;
    ExcDescriptionSeq  exceptions ;
    Identifier  name ;
}  ExtInitializer ;

typedef  SEQUENCE (ExtInitializer, ExtInitializerSeq) ;

typedef  SEQUENCE (UnionMember, UnionMemberSeq) ;

typedef  SEQUENCE (Identifier, EnumMemberSeq) ;

/*===== "Description" already defined =====*/

typedef  SEQUENCE (Description, DescriptionSeq) ;

typedef  enum  PrimitiveKind {
    pk_null,
    pk_void,
    pk_short,
    pk_long,
    pk_ushort,
    pk_ulong,
    pk_float,
    pk_double,
    pk_boolean,
    pk_char,
    pk_octet,
    pk_any,
    pk_TypeCode,
    pk_Principal,
    pk_string,
    pk_objref,
    pk_longlong,
    pk_ulonglong,
    pk_longdouble,
    pk_wchar,
    pk_wstring,
    pk_value_base
}  PrimitiveKind ;

typedef  struct  ModuleDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
}  ModuleDescription ;

typedef  struct  ConstantDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    TypeCode  type ;
    Any  value ;
}  ConstantDescription ;

typedef  struct  TypeDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    TypeCode  type ;
}  TypeDescription ;

typedef  enum  AttributeMode {
    ATTR_NORMAL,
    ATTR_READONLY
}  AttributeMode ;

typedef  struct  AttributeDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    TypeCode  type ;
    AttributeMode  mode ;
}  AttributeDescription ;

typedef  struct  ExtAttributeDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    TypeCode  type ;
    AttributeMode  mode ;
    ExcDescriptionSeq  get_exceptions ;
    ExcDescriptionSeq  put_exceptions ;
}  ExtAttributeDescription ;

typedef  IOR  ExtAttributeDef ;

typedef  enum  OperationMode {
    OP_NORMAL,
    OP_ONEWAY
}  OperationMode ;

typedef  enum  ParameterMode {
    PARAM_IN,
    PARAM_OUT,
    PARAM_INOUT
}  ParameterMode ;

typedef  struct  ParameterDescription {
    Identifier  name ;
    TypeCode  type ;
    IDLType  type_def ;
    ParameterMode  mode ;
}  ParameterDescription ;

typedef  SEQUENCE (ParameterDescription, ParDescriptionSeq) ;

typedef  Identifier  ContextIdentifier ;

typedef  SEQUENCE (ContextIdentifier, ContextIdSeq) ;

typedef  SEQUENCE (ExceptionDef, ExceptionDefSeq) ;

typedef  struct  OperationDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    TypeCode  result ;
    OperationMode  mode ;
    ContextIdSeq  contexts ;
    ParDescriptionSeq  parameters ;
    ExcDescriptionSeq  exceptions ;
}  OperationDescription ;

typedef  SEQUENCE (RepositoryId, RepositoryIdSeq) ;

typedef  SEQUENCE (OperationDescription, OpDescriptionSeq) ;

typedef  SEQUENCE (AttributeDescription, AttrDescriptionSeq) ;

typedef  SEQUENCE (ExtAttributeDescription, ExtAttrDescriptionSeq) ;

typedef  struct  FullInterfaceDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    OpDescriptionSeq  operations ;
    AttrDescriptionSeq  attributes ;
    RepositoryIdSeq  base_interfaces ;
    TypeCode  type ;
    bool  is_abstract ;
}  FullInterfaceDescription ;

typedef  struct  InterfaceDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    RepositoryIdSeq  base_interfaces ;
    bool  is_abstract ;
}  InterfaceDescription ;

typedef  IOR  InterfaceAttrExtension ;

typedef  struct  ExtFullInterfaceDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    OpDescriptionSeq  operations ;
    ExtAttrDescriptionSeq  attributes ;
    RepositoryIdSeq  base_interfaces ;
    TypeCode  type ;
}  ExtFullInterfaceDescription ;

typedef  short  Visibility ;

#define  CORBA_PRIVATE_MEMBER  (0)
#define  CORBA_PUBLIC_MEMBER  (1)

typedef  struct  ValueMember {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    TypeCode  type ;
    IDLType  type_def ;
    Visibility  access ;
}  ValueMember ;

typedef  SEQUENCE (ValueMember, ValueMemberSeq) ;

typedef  struct  FullValueDescription {
    Identifier  name ;
    RepositoryId  id ;
    bool  is_abstract ;
    bool  is_custom ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    OpDescriptionSeq  operations ;
    AttrDescriptionSeq  attributes ;
    ValueMemberSeq  members ;
    InitializerSeq  initializers ;
    RepositoryIdSeq  supported_interfaces ;
    RepositoryIdSeq  abstract_base_values ;
    bool  is_truncatable ;
    RepositoryId  base_value ;
    TypeCode  type ;
}  FullValueDescription ;

typedef  struct  ValueDescription {
    Identifier  name ;
    RepositoryId  id ;
    bool  is_abstract ;
    bool  is_custom ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    RepositoryIdSeq  supported_interfaces ;
    RepositoryIdSeq  abstract_base_values ;
    bool  is_truncatable ;
    RepositoryId  base_value ;
}  ValueDescription ;

typedef  struct  ExtFullValueDescription {
    Identifier  name ;
    RepositoryId  id ;
    bool  is_abstract ;
    bool  is_custom ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    OpDescriptionSeq  operations ;
    ExtAttrDescriptionSeq  attributes ;
    ValueMemberSeq  members ;
    ExtInitializerSeq  initializers ;
    RepositoryIdSeq  supported_interfaces ;
    RepositoryIdSeq  abstract_base_values ;
    bool  is_truncatable ;
    RepositoryId  base_value ;
    TypeCode  type ;
}  ExtFullValueDescription ;

/* Module: CORBA::ComponentIR */

typedef  IOR  ComponentDef ;
typedef  IOR  HomeDef ;
typedef  IOR  EventDef ;
typedef  IOR  ProvidesDef ;

typedef  struct  ProvidesDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    RepositoryId  interface_type ;
}  ProvidesDescription ;

typedef  IOR  UsesDef ;

typedef  struct  UsesDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    RepositoryId  interface_type ;
    bool  is_multiple ;
}  UsesDescription ;

typedef  IOR  EventPortDef ;

typedef  struct  EventPortDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    RepositoryId  event ;
}  EventPortDescription ;

typedef  IOR  EmitsDef ;
typedef  IOR  PublishesDef ;
typedef  IOR  ConsumesDef ;

typedef  SEQUENCE (ProvidesDescription, ProvidesDescriptionSeq) ;

typedef  SEQUENCE (UsesDescription, UsesDescriptionSeq) ;

typedef  SEQUENCE (EventPortDescription, EventPortDescriptionSeq) ;

typedef  struct  ComponentDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    RepositoryId  base_component ;
    RepositoryIdSeq  supported_interfaces ;
    ProvidesDescriptionSeq  provided_interfaces ;
    UsesDescriptionSeq  used_interfaces ;
    EventPortDescriptionSeq  emits_events ;
    EventPortDescriptionSeq  publishes_events ;
    EventPortDescriptionSeq  consumes_events ;
    ExtAttrDescriptionSeq  attributes ;
    TypeCode  type ;
}  ComponentDescription ;

typedef  IOR  FactoryDef ;
typedef  IOR  FinderDef ;

typedef  struct  HomeDescription {
    Identifier  name ;
    RepositoryId  id ;
    RepositoryId  defined_in ;
    VersionSpec  version ;
    RepositoryId  base_home ;
    RepositoryId  managed_component ;
    ValueDescription  primary_key ;
    OpDescriptionSeq  factories ;
    OpDescriptionSeq  finders ;
    OpDescriptionSeq  operations ;
    ExtAttrDescriptionSeq  attributes ;
    TypeCode  type ;
}  HomeDescription ;

typedef  short  ValueModifier ;

#define  CORBA_VM_NONE  (0)
#define  CORBA_VM_CUSTOM  (1)
#define  CORBA_VM_ABSTRACT  (2)
#define  CORBA_VM_TRUNCATABLE  (3)

/*===== "AnySeq" already defined =====*/

/*===== "BooleanSeq" already defined =====*/

/*===== "CharSeq" already defined =====*/

/*===== "WCharSeq" already defined =====*/

/*===== "OctetSeq" already defined =====*/

/*===== "ShortSeq" already defined =====*/

/*===== "UShortSeq" already defined =====*/

/*===== "LongSeq" already defined =====*/

/*===== "ULongSeq" already defined =====*/

/*===== "LongLongSeq" already defined =====*/

/*===== "ULongLongSeq" already defined =====*/

/*===== "FloatSeq" already defined =====*/

/*===== "DoubleSeq" already defined =====*/

/*===== "LongDoubleSeq" already defined =====*/

/*===== "StringSeq" already defined =====*/

/*===== "WStringSeq" already defined =====*/

/* Module: BiDirPolicy */

typedef  unsigned  short  BidirectionalPolicyValue ;

#define  NORMAL  (0)
#define  BOTH  (1)
#define  BIDIRECTIONAL_POLICY_TYPE  (37)

typedef  IOR  BidirectionalPolicy ;

/* Module: CONV_FRAME */

typedef  unsigned  long  CodeSetId ;

typedef  struct  CodeSetComponent {
    CodeSetId  native_code_set ;
    SEQUENCE (CodeSetId, conversion_code_sets) ;
}  CodeSetComponent ;

typedef  struct  CodeSetComponentInfo {
    CodeSetComponent  ForCharData ;
    CodeSetComponent  ForWcharData ;
}  CodeSetComponentInfo ;

typedef  struct  CodeSetContext {
    CodeSetId  char_data ;
    CodeSetId  wchar_data ;
}  CodeSetContext ;

/* Module: IOP */

#define  IOP_TAG_INTERNET_IOP  (0)
#define  IOP_TAG_MULTIPLE_COMPONENTS  (1)
#define  IOP_TAG_SCCP_IOP  (2)

#define  IOP_TAG_ORB_TYPE  (0)
#define  IOP_TAG_CODE_SETS  (1)
#define  IOP_TAG_POLICIES  (2)
#define  IOP_TAG_ALTERNATE_IIOP_ADDRESS  (3)
#define  IOP_TAG_ASSOCIATION_OPTIONS  (13)
#define  IOP_TAG_SEC_NAME  (14)
#define  IOP_TAG_SPKM_1_SEC_MECH  (15)
#define  IOP_TAG_SPKM_2_SEC_MECH  (16)
#define  IOP_TAG_KerberosV5_SEC_MECH  (17)
#define  IOP_TAG_CSI_ECMA_Secret_SEC_MECH  (18)
#define  IOP_TAG_CSI_ECMA_Hybrid_SEC_MECH  (19)
#define  IOP_TAG_SSL_SEC_TRANS  (20)
#define  IOP_TAG_CSI_ECMA_Public_SEC_MECH  (21)
#define  IOP_TAG_GENERIC_SEC_MECH  (22)
#define  IOP_TAG_FIREWALL_TRANS  (23)
#define  IOP_TAG_SCCP_CONTACT_INFO  (24)
#define  IOP_TAG_JAVA_CODEBASE  (25)
#define  IOP_TAG_TRANSACTION_POLICY  (26)
#define  IOP_TAG_MESSAGE_ROUTER  (30)
#define  IOP_TAG_OTS_POLICY  (31)
#define  IOP_TAG_INV_POLICY  (32)
#define  IOP_TAG_CSI_SEC_MECH_LIST  (33)
#define  IOP_TAG_NULL_TAG  (34)
#define  IOP_TAG_SECIOP_SEC_TRANS  (35)
#define  IOP_TAG_TLS_SEC_TRANS  (36)
#define  IOP_TAG_ACTIVITY_POLICY  (37)
#define  IOP_TAG_COMPLETE_OBJECT_KEY  (5)
#define  IOP_TAG_ENDPOINT_ID_POSITION  (6)
#define  IOP_TAG_LOCATION_POLICY  (12)
#define  IOP_TAG_DCE_STRING_BINDING  (100)
#define  IOP_TAG_DCE_BINDING_NAME  (101)
#define  IOP_TAG_DCE_NO_PIPES  (102)
#define  IOP_TAG_DCE_SEC_MECH  (103)
#define  IOP_TAG_INET_SEC_TRANS  (123)

typedef  unsigned  long  ServiceId ;

typedef  struct  ServiceContext {
    ServiceId  context_id ;
    OctetSeq  context_data ;
}  ServiceContext ;

typedef  SEQUENCE (ServiceContext, ServiceContextList) ;

#define  IOP_TransactionService  (0)
#define  IOP_CodeSets  (1)
#define  IOP_ChainBypassCheck  (2)
#define  IOP_ChainBypassInfo  (3)
#define  IOP_LogicalThreadId  (4)
#define  IOP_BI_DIR_IIOP  (5)
#define  IOP_SendingContextRunTime  (6)
#define  IOP_INVOCATION_POLICIES  (7)
#define  IOP_FORWARDED_IDENTITY  (8)
#define  IOP_UnknownExceptionInfo  (9)
#define  IOP_RTCorbaPriority  (10)
#define  IOP_RTCorbaPriorityRange  (11)
#define  IOP_FT_GROUP_VERSION  (12)
#define  IOP_FT_REQUEST  (13)
#define  IOP_ExceptionDetailMessage  (14)
#define  IOP_SecurityAttributeService  (15)
#define  IOP_ActivityService  (16)

typedef  short  EncodingFormat ;

#define  IOP_ENCODING_CDR_ENCAPS  (0)

typedef  struct  Encoding {
    EncodingFormat  format ;
    octet  major_version ;
    octet  minor_version ;
}  Encoding ;

/* Module: CSI */

#define  CSI_OMGVMCID  (0x4F4D0UL)

typedef  OctetSeq  X509CertificateChain ;

typedef  OctetSeq  X501DistinguishedName ;

typedef  OctetSeq  UTF8String ;

typedef  OctetSeq  OID ;

typedef  SEQUENCE (OID, OIDList) ;

typedef  OctetSeq  GSSToken ;

typedef  OctetSeq  GSS_NT_ExportedName ;

typedef  SEQUENCE (GSS_NT_ExportedName, GSS_NT_ExportedNameList) ;

typedef  short  MsgType ;

#define  MTEstablishContext  (0)
#define  MTCompleteEstablishContext  (1)
#define  MTContextError  (4)
#define  MTMessageInContext  (5)

typedef  ULONGLONG  ContextId ;

typedef  unsigned  long  AuthorizationElementType ;

#define  X509AttributeCertChain  (OMGVMCID | 1)

typedef  OctetSeq  AuthorizationElementContents ;

typedef  struct  AuthorizationElement {
    AuthorizationElementType  the_type ;
    AuthorizationElementContents  the_element ;
}  AuthorizationElement ;

typedef  SEQUENCE (AuthorizationElement, AuthorizationToken) ;

typedef  unsigned  long  IdentityTokenType ;

#define  ITTAbsent  (0)
#define  ITTAnonymous  (1)
#define  ITTPrincipalName  (2)
#define  ITTX509CertChain  (4)
#define  ITTDistinguishedName  (8)

typedef  OctetSeq  IdentityExtension ;

typedef  struct  IdentityToken {
    IdentityTokenType  which ;
    union {
			/* ITTAbsent */
        bool  absent ;
			/* ITTAnonymous */
        bool  anonymous ;
			/* ITTPrincipalName */
        GSS_NT_ExportedName  principal_name ;
			/* ITTX509CertChain */
        X509CertificateChain  certificate_chain ;
			/* ITTDistinguishedName */
        X501DistinguishedName  dn ;
			/* <default> */
        IdentityExtension  id ;
    }  data ;
}  IdentityToken ;

typedef  struct  EstablishContext {
    ContextId  client_context_id ;
    AuthorizationToken  authorization_token ;
    IdentityToken  identity_token ;
    GSSToken  client_authentication_token ;
}  EstablishContext ;

typedef  struct  CompleteEstablishContext {
    ContextId  client_context_id ;
    bool  context_stateful ;
    GSSToken  final_context_token ;
}  CompleteEstablishContext ;

typedef  struct  ContextError {
    ContextId  client_context_id ;
    long  major_status ;
    long  minor_status ;
    GSSToken  error_token ;
}  ContextError ;

typedef  struct  MessageInContext {
    ContextId  client_context_id ;
    bool  discard_context ;
}  MessageInContext ;

typedef  struct  SASContextBody {
    MsgType  which ;
    union {
			/* MTEstablishContext */
        EstablishContext  establish_msg ;
			/* MTCompleteEstablishContext */
        CompleteEstablishContext  complete_msg ;
			/* MTContextError */
        ContextError  error_msg ;
			/* MTMessageInContext */
        MessageInContext  in_context_msg ;
    }  data ;
}  SASContextBody ;

typedef  char  *StringOID ;

#define  KRB5MechOID  ("oid:1.2.840.113554.1.2.2")
#define  GSS_NT_Export_Name_OID  ("oid:1.3.6.1.5.6.4")
#define  GSS_NT_Scoped_Username_OID  ("oid:2.23.130.1.2.1")

/* Module: CSIIOP */

typedef  unsigned  short  AssociationOptions ;

#define  NoProtection  (1)
#define  Integrity  (2)
#define  Confidentiality  (4)
#define  DetectReplay  (8)
#define  DetectMisordering  (16)
#define  EstablishTrustInTarget  (32)
#define  EstablishTrustInClient  (64)
#define  NoDelegation  (128)
#define  SimpleDelegation  (256)
#define  CompositeDelegation  (512)
#define  IdentityAssertion  (1024)
#define  DelegationByClient  (2048)

typedef  unsigned  long  ServiceConfigurationSyntax ;

#define  SCS_GeneralNames  (OMGVMCID | 0)
#define  SCS_GSSExportedName  (OMGVMCID | 1)

typedef  OctetSeq  ServiceSpecificName ;

typedef  struct  ServiceConfiguration {
    ServiceConfigurationSyntax  syntax ;
    ServiceSpecificName  name ;
}  ServiceConfiguration ;

typedef  SEQUENCE (ServiceConfiguration, ServiceConfigurationList) ;

typedef  struct  AS_ContextSec {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    OID  client_authentication_mech ;
    GSS_NT_ExportedName  target_name ;
}  AS_ContextSec ;

typedef  struct  SAS_ContextSec {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    ServiceConfigurationList  privilege_authorities ;
    OIDList  supported_naming_mechanisms ;
    IdentityTokenType  supported_identity_types ;
}  SAS_ContextSec ;

typedef  struct  CompoundSecMech {
    AssociationOptions  target_requires ;
    TaggedComponent  transport_mech ;
    AS_ContextSec  as_context_mech ;
    SAS_ContextSec  sas_context_mech ;
}  CompoundSecMech ;

typedef  SEQUENCE (CompoundSecMech, CompoundSecMechanisms) ;

typedef  struct  CompoundSecMechList {
    bool  stateful ;
    CompoundSecMechanisms  mechanism_list ;
}  CompoundSecMechList ;

typedef  struct  TransportAddress {
    char  *host_name ;
    unsigned  short  port ;
}  TransportAddress ;

typedef  SEQUENCE (TransportAddress, TransportAddressList) ;

/*===== "TAG_SECIOP_SEC_TRANS" previously defined in IOP =====*/

typedef  struct  SECIOP_SEC_TRANS {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    OID  mech_oid ;
    GSS_NT_ExportedName  target_name ;
    TransportAddressList  addresses ;
}  SECIOP_SEC_TRANS ;

/*===== "TAG_TLS_SEC_TRANS" previously defined in IOP =====*/

typedef  struct  TLS_SEC_TRANS {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    TransportAddressList  addresses ;
}  TLS_SEC_TRANS ;

/* Module: GIOP */

typedef  OctetSeq  Principal ;

/*===== "GIOPMsgType" already defined =====*/

/*===== "GIOPMsgType" already defined =====*/

typedef  struct  MessageHeader_1_0 {
    char  magic[4] ;
    Version  GIOP_version ;
    bool  byte_order ;
    octet  message_type ;
    unsigned  long  message_size ;
}  MessageHeader_1_0 ;

typedef  struct  MessageHeader {
    char  magic[4] ;
    Version  GIOP_version ;
    octet  flags ;
    octet  message_type ;
    unsigned  long  message_size ;
}  MessageHeader ;

/*===== "MessageHeader" already defined =====*/

/*===== "MessageHeader" already defined =====*/

typedef  struct  RequestHeader_1_0 {
    ServiceContextList  service_context ;
    unsigned  long  request_id ;
    bool  response_expected ;
    OctetSeq  object_key ;
    char  *operation ;
    Principal  requesting_principal ;
}  RequestHeader_1_0 ;

typedef  struct  RequestHeader_1_1 {
    ServiceContextList  service_context ;
    unsigned  long  request_id ;
    bool  response_expected ;
    octet  reserved[3] ;
    OctetSeq  object_key ;
    char  *operation ;
    Principal  requesting_principal ;
}  RequestHeader_1_1 ;

typedef  short  AddressingDisposition ;

#define  KeyAddr  (0)
#define  ProfileAddr  (1)
#define  ReferenceAddr  (2)

typedef  struct  IORAddressingInfo {
    unsigned  long  selected_profile_index ;
    IOR  ior ;
}  IORAddressingInfo ;

typedef  struct  TargetAddress {
    AddressingDisposition  which ;
    union {
			/* KeyAddr */
        OctetSeq  object_key ;
			/* ProfileAddr */
        TaggedProfile  profile ;
			/* ReferenceAddr */
        IORAddressingInfo  ior ;
    }  data ;
}  TargetAddress ;

typedef  struct  RequestHeader {
    unsigned  long  request_id ;
    octet  response_flags ;
    octet  reserved[3] ;
    TargetAddress  target ;
    char  *operation ;
    ServiceContextList  service_context ;
}  RequestHeader ;

/*===== "RequestHeader" already defined =====*/

typedef  struct  ReplyHeader_1_0 {
    ServiceContextList  service_context ;
    unsigned  long  request_id ;
    ReplyStatusType  reply_status ;
}  ReplyHeader_1_0 ;

typedef  ReplyHeader_1_0  ReplyHeader_1_1 ;

typedef  struct  ReplyHeader {
    unsigned  long  request_id ;
    ReplyStatusType  reply_status ;
    ServiceContextList  service_context ;
}  ReplyHeader ;

/*===== "ReplyHeader" already defined =====*/

typedef  struct  SystemExceptionReplyBody {
    char  *exception_id ;
    unsigned  long  minor_code_value ;
    unsigned  long  completion_status ;
}  SystemExceptionReplyBody ;

typedef  struct  CancelRequestHeader {
    unsigned  long  request_id ;
}  CancelRequestHeader ;

typedef  struct  LocateRequestHeader_1_0 {
    unsigned  long  request_id ;
    OctetSeq  object_key ;
}  LocateRequestHeader_1_0 ;

typedef  LocateRequestHeader_1_0  LocateRequestHeader_1_1 ;

typedef  struct  LocateRequestHeader {
    unsigned  long  request_id ;
    TargetAddress  target ;
}  LocateRequestHeader ;

/*===== "LocateRequestHeader" already defined =====*/

typedef  struct  LocateReplyHeader_1_0 {
    unsigned  long  request_id ;
    LocateStatusType  locate_status ;
}  LocateReplyHeader_1_0 ;

typedef  LocateReplyHeader_1_0  LocateReplyHeader_1_1 ;

typedef  struct  LocateReplyHeader {
    unsigned  long  request_id ;
    LocateStatusType  locate_status ;
}  LocateReplyHeader ;

/*===== "LocateReplyHeader" already defined =====*/

typedef  struct  FragmentHeader {
    unsigned  long  request_id ;
}  FragmentHeader ;

/*===== "FragmentHeader" already defined =====*/

/* Module: GSSUP */

#define  GSSUPMechOID  ("oid:2.23.130.1.1.1")

typedef  struct  InitialContextToken {
    UTF8String  username ;
    UTF8String  password ;
    GSS_NT_ExportedName  target_name ;
}  InitialContextToken ;

typedef  unsigned  long  ErrorCode ;

typedef  struct  ErrorToken {
    ErrorCode  error_code ;
}  ErrorToken ;

#define  GSS_UP_S_G_UNSPECIFIED  (1)
#define  GSS_UP_S_G_NOUSER  (2)
#define  GSS_UP_S_G_BAD_PASSWORD  (3)
#define  GSS_UP_S_G_BAD_TARGET  (4)

/* Module: IIOP */

/*===== "ProfileBody" already defined =====*/

/*===== "ProfileBody" already defined =====*/

typedef  struct  ListenPoint {
    char  *host ;
    unsigned  short  port ;
}  ListenPoint ;

typedef  SEQUENCE (ListenPoint, ListenPointList) ;

typedef  struct  BiDirIIOPServiceContext {
    ListenPointList  listen_points ;
}  BiDirIIOPServiceContext ;

/* Module: IOP */

typedef  struct  EndpointIdPositionComponent {
    unsigned  short  begin ;
    unsigned  short  end ;
}  EndpointIdPositionComponent ;

/* Module: SendingContext */

typedef  IOR  RunTime ;
typedef  IOR  CodeBase ;

typedef  char  *URL ;

typedef  SEQUENCE (URL, URLSeq) ;

typedef  SEQUENCE (FullValueDescription, ValueDescSeq) ;

/* Module: CosTransactions */

typedef  IOR  Current ;
typedef  IOR  TransactionFactory ;
typedef  IOR  Control ;
typedef  IOR  Terminator ;
typedef  IOR  Coordinator ;
typedef  IOR  RecoveryCoordinator ;
typedef  IOR  Resource ;
typedef  IOR  Synchronization ;
typedef  IOR  SubtransactionAwareResource ;
typedef  IOR  TransactionalObject ;

typedef  enum  Status {
    StatusActive,
    StatusMarkedRollback,
    StatusPrepared,
    StatusCommitted,
    StatusRolledBack,
    StatusUnknown,
    StatusNoTransaction,
    StatusPreparing,
    StatusCommitting,
    StatusRollingBack
}  Status ;

typedef  enum  Vote {
    VoteCommit,
    VoteRollback,
    VoteReadOnly
}  Vote ;

typedef  struct  otid_t {
    long  formatID ;
    long  bqual_length ;
    OctetSeq  tid ;
}  otid_t ;

typedef  struct  TransIdentity {
    Coordinator  coord ;
    Terminator  term ;
    otid_t  otid ;
}  TransIdentity ;

typedef  struct  PropagationContext {
    unsigned  long  timeout ;
    TransIdentity  current ;
    SEQUENCE (TransIdentity, parents) ;
    Any  implementation_specific_data ;
}  PropagationContext ;

/* Module: CosConcurrencyControl */

typedef  enum  lock_mode {
    Read,
    Write,
    Upgrade,
    Intention_read,
    Intention_write
}  lock_mode ;

typedef  IOR  LockCoordinator ;
typedef  IOR  LockSet ;
typedef  IOR  TransactionalLockSet ;
typedef  IOR  LockSetFactory ;

/* Module: CosCollection */

typedef  IOR  Collection ;

typedef  SEQUENCE (Any, AnySequence) ;

typedef  char  *Istring ;

typedef  struct  NVPair {
    Istring  name ;
    Any  value ;
}  NVPair ;

typedef  SEQUENCE (NVPair, ParameterList) ;

typedef  enum  IteratorInvalidReason {
    is_invalid,
    is_not_for_collection,
    is_const
}  IteratorInvalidReason ;

typedef  enum  ElementInvalidReason {
    element_type_invalid,
    positioning_property_invalid,
    element_exists
}  ElementInvalidReason ;

typedef  IOR  Operations ;
typedef  IOR  Command ;
typedef  IOR  Comparator ;
typedef  IOR  Iterator ;
typedef  IOR  OrderedIterator ;
typedef  IOR  SequentialIterator ;
typedef  IOR  KeyIterator ;
typedef  IOR  EqualityIterator ;
typedef  IOR  EqualityKeyIterator ;
typedef  IOR  SortedIterator ;

typedef  enum  LowerBoundStyle {
    equal_lo,
    greater,
    greater_or_equal
}  LowerBoundStyle ;

typedef  enum  UpperBoundStyle {
    equal_up,
    less,
    less_or_equal
}  UpperBoundStyle ;

typedef  IOR  KeySortedIterator ;
typedef  IOR  EqualitySortedIterator ;
typedef  IOR  EqualityKeySortedIterator ;
typedef  IOR  EqualitySequentialIterator ;
typedef  IOR  OrderedCollection ;
typedef  IOR  SequentialCollection ;
typedef  IOR  SortedCollection ;
typedef  IOR  EqualityCollection ;
typedef  IOR  KeyCollection ;
typedef  IOR  EqualityKeyCollection ;
typedef  IOR  KeySortedCollection ;
typedef  IOR  EqualitySortedCollection ;
typedef  IOR  EqualityKeySortedCollection ;
typedef  IOR  EqualitySequentialCollection ;
typedef  IOR  KeySet ;
typedef  IOR  KeyBag ;
typedef  IOR  Map ;
typedef  IOR  Relation ;
typedef  IOR  Set ;
typedef  IOR  Bag ;
typedef  IOR  KeySortedSet ;
typedef  IOR  KeySortedBag ;
typedef  IOR  SortedMap ;
typedef  IOR  SortedRelation ;
typedef  IOR  SortedSet ;
typedef  IOR  SortedBag ;
typedef  IOR  CSequence ;
typedef  IOR  EqualitySequence ;
typedef  IOR  Heap ;
typedef  IOR  RestrictedAccessCollection ;
typedef  IOR  Queue ;
typedef  IOR  Deque ;
typedef  IOR  Stack ;
typedef  IOR  PriorityQueue ;
typedef  IOR  CollectionFactory ;
typedef  IOR  CollectionFactories ;
typedef  IOR  RACollectionFactory ;
typedef  IOR  RACollectionFactories ;
typedef  IOR  KeySetFactory ;
typedef  IOR  KeyBagFactory ;
typedef  IOR  MapFactory ;
typedef  IOR  RelationFactory ;
typedef  IOR  SetFactory ;
typedef  IOR  BagFactory ;
typedef  IOR  KeySortedSetFactory ;
typedef  IOR  KeySortedBagFactory ;
typedef  IOR  SortedMapFactory ;
typedef  IOR  SortedRelationFactory ;
typedef  IOR  SortedSetFactory ;
typedef  IOR  SortedBagFactory ;
typedef  IOR  SequenceFactory ;
typedef  IOR  EqualitySequenceFactory ;
typedef  IOR  HeapFactory ;
typedef  IOR  QueueFactory ;
typedef  IOR  StackFactory ;
typedef  IOR  DequeFactory ;
typedef  IOR  PriorityQueueFactory ;

/* Module: CosObjectIdentity */

typedef  unsigned  long  ObjectIdentifier ;

typedef  IOR  IdentifiableObject ;

/* Module: CosRelationships */

typedef  IOR  RoleFactory ;
typedef  IOR  RelationshipFactory ;
typedef  IOR  Relationship ;
typedef  IOR  Role ;
typedef  IOR  RelationshipIterator ;

typedef  Object  RelatedObject ;

typedef  SEQUENCE (Role, Roles) ;

typedef  char  *RoleName ;

typedef  SEQUENCE (RoleName, RoleNames) ;

typedef  struct  NamedRole {
    RoleName  name ;
    Role  aRole ;
}  NamedRole ;

typedef  SEQUENCE (NamedRole, NamedRoles) ;

typedef  struct  RelationshipHandle {
    Relationship  the_relationship ;
    ObjectIdentifier  constant_random_id ;
}  RelationshipHandle ;

typedef  SEQUENCE (RelationshipHandle, RelationshipHandles) ;

typedef  struct  NamedRoleType {
    RoleName  name ;
    InterfaceDef  named_role_type ;
}  NamedRoleType ;

typedef  SEQUENCE (NamedRoleType, NamedRoleTypes) ;

typedef  SEQUENCE (InterfaceDef, InterfaceDefs) ;

/* Module: CosGraphs */

typedef  IOR  TraversalFactory ;
typedef  IOR  Traversal ;
typedef  IOR  TraversalCriteria ;
typedef  IOR  Node ;
typedef  IOR  NodeFactory ;
typedef  IOR  EdgeIterator ;

typedef  struct  NodeHandle {
    Node  the_node ;
    ObjectIdentifier  constant_random_id ;
}  NodeHandle ;

typedef  SEQUENCE (NodeHandle, NodeHandles) ;

/*===== "NamedRole" already defined =====*/

/*===== "NamedRoles" already defined =====*/

typedef  struct  EndPoint {
    NodeHandle  the_node ;
    NamedRole  the_role ;
}  EndPoint ;

typedef  SEQUENCE (EndPoint, EndPoints) ;

typedef  struct  Edge {
    EndPoint  from ;
    RelationshipHandle  the_relationship ;
    EndPoints  relatives ;
}  Edge ;

typedef  SEQUENCE (Edge, Edges) ;

typedef  enum  PropagationValue {
    deep,
    shallow,
    none,
    inhibit
}  PropagationValue ;

typedef  enum  Mode {
    depthFirst,
    breadthFirst,
    bestFirst
}  Mode ;

typedef  unsigned  long  TraversalScopedId ;

typedef  struct  ScopedEndPoint {
    EndPoint  point ;
    TraversalScopedId  id ;
}  ScopedEndPoint ;

typedef  SEQUENCE (ScopedEndPoint, ScopedEndPoints) ;

typedef  struct  ScopedRelationship {
    RelationshipHandle  scoped_relationship ;
    TraversalScopedId  id ;
}  ScopedRelationship ;

typedef  struct  ScopedEdge {
    ScopedEndPoint  from ;
    ScopedRelationship  the_relationship ;
    ScopedEndPoints  relatives ;
}  ScopedEdge ;

typedef  SEQUENCE (ScopedEdge, ScopedEdges) ;

typedef  struct  WeightedEdge {
    Edge  the_edge ;
    unsigned  long  weight ;
    SEQUENCE (NodeHandle, next_nodes) ;
}  WeightedEdge ;

typedef  SEQUENCE (WeightedEdge, WeightedEdges) ;

/*===== "Roles" already defined =====*/

/* Module: CosContainment */

typedef  IOR  ContainsRole ;
typedef  IOR  ContainedInRole ;

/* Module: CosNaming */

/*===== "Istring" already defined =====*/

typedef  struct  NameComponent {
    Istring  id ;
    Istring  kind ;
}  NameComponent ;

typedef  SEQUENCE (NameComponent, Name) ;

typedef  enum  BindingType {
    nobject,
    ncontext
}  BindingType ;

typedef  struct  Binding {
    Name  binding_name ;
    BindingType  binding_type ;
}  Binding ;

typedef  SEQUENCE (Binding, BindingList) ;

typedef  IOR  BindingIterator ;
typedef  IOR  NamingContext ;

typedef  enum  NotFoundReason {
    missing_node,
    not_context,
    not_object
}  NotFoundReason ;

/* Module: CosLifeCycle */

typedef  Name  Key ;

typedef  Object  Factory ;

typedef  SEQUENCE (Factory, Factories) ;

typedef  struct  NameValuePair {
    Istring  name ;
    Any  value ;
}  NameValuePair ;

typedef  SEQUENCE (NameValuePair, Criteria) ;

typedef  IOR  FactoryFinder ;
typedef  IOR  LifeCycleObject ;
typedef  IOR  GenericFactory ;

/* Module: CosStream */

typedef  IOR  StreamIO ;
typedef  IOR  Streamable ;
typedef  IOR  StreamableFactory ;

/*===== "RelationshipHandle" already defined =====*/
typedef  IOR  PropagationCriteriaFactory ;

/* Module: CosExternalizationContainment */

/* Module: CosExternalization */

typedef  IOR  Stream ;
typedef  IOR  StreamFactory ;
typedef  IOR  FileStreamFactory ;

/* Module: CosReference */

typedef  IOR  ReferencesRole ;
typedef  IOR  ReferencedByRole ;

/* Module: CosExternalizationReference */

/* Module: CosEventComm */

typedef  IOR  PushConsumer ;
typedef  IOR  PushSupplier ;
typedef  IOR  PullSupplier ;
typedef  IOR  PullConsumer ;

/* Module: CosPropertyService */

typedef  char  *PropertyName ;

typedef  struct  Property {
    PropertyName  property_name ;
    Any  property_value ;
}  Property ;

typedef  enum  PropertyModeType {
    normal,
    read_only,
    fixed_normal,
    fixed_readonly,
    undefined
}  PropertyModeType ;

typedef  struct  PropertyDef {
    PropertyName  property_name ;
    Any  property_value ;
    PropertyModeType  property_mode ;
}  PropertyDef ;

typedef  struct  PropertyMode {
    PropertyName  property_name ;
    PropertyModeType  property_mode ;
}  PropertyMode ;

typedef  SEQUENCE (PropertyName, PropertyNames) ;

typedef  SEQUENCE (Property, Properties) ;

typedef  SEQUENCE (PropertyDef, PropertyDefs) ;

typedef  SEQUENCE (PropertyMode, PropertyModes) ;

typedef  SEQUENCE (TypeCode, PropertyTypes) ;

typedef  IOR  PropertyNamesIterator ;
typedef  IOR  PropertiesIterator ;
typedef  IOR  PropertySetFactory ;
typedef  IOR  PropertySetDef ;
typedef  IOR  PropertySet ;

typedef  enum  ExceptionReason {
    invalid_property_name,
    conflicting_property,
    property_not_found,
    unsupported_type_code,
    unsupported_property,
    unsupported_mode,
    fixed_property,
    read_only_property
}  ExceptionReason ;

typedef  struct  PropertyException {
    ExceptionReason  reason ;
    PropertyName  failing_property_name ;
}  PropertyException ;

typedef  SEQUENCE (PropertyException, PropertyExceptions) ;

typedef  IOR  PropertySetDefFactory ;

/* Module: CosLicensingManager */

typedef  Object  ProducerSpecificNotification ;

typedef  enum  ActionRequired {
    Continue,
    Terminate
}  ActionRequired ;

typedef  enum  Answer {
    yes,
    no
}  Answer ;

typedef  struct  Action {
    ActionRequired  action ;
    Answer  notification_required ;
    Answer  wait_for_user_confirmation_after_notification ;
    unsigned  long  notification_duration ;
    ProducerSpecificNotification  producer_notification ;
    char  *notification_text ;
}  Action ;

typedef  struct  ChallengeData {
    unsigned  long  challenge_index ;
    unsigned  long  random_number ;
    char  *digest ;
}  ChallengeData ;

typedef  enum  ChallengeProtocol {
    default_protocol,
    producer_defined
}  ChallengeProtocol ;

typedef  struct  Challenge {
    ChallengeProtocol  challenge_protocol ;
    unsigned  long  challenge_data_size ;
    Any  challenge_data ;
}  Challenge ;

typedef  Any  LicenseHandle ;

typedef  IOR  ProducerSpecificLicenseService ;
typedef  IOR  LicenseServiceManager ;

/* Module: CosCompoundLifeCycle */

typedef  IOR  OperationsFactory ;

typedef  enum  Operation {
    Copy,
    Move,
    Remove
}  Operation ;

/*===== "RelationshipHandle" already defined =====*/

/* Module: CosLifeCycleContainment */

/* Module: CosLifeCycleReference */

/* Module: CosPersistencePID */

typedef  IOR  PID ;

/* Module: CosPersistenceDDO */

typedef  IOR  DDO ;

/* Module: CosPersistenceDS_CLI */

typedef  IOR  UserEnvironment ;
typedef  IOR  Connection ;
typedef  IOR  ConnectionFactory ;
typedef  IOR  Cursor ;
typedef  IOR  CursorFactory ;
typedef  IOR  PID_CLI ;
typedef  IOR  Datastore_CLI ;

/* Module: CosPersistencePDS */

typedef  IOR  PDS ;

/* Module: CosPersistencePDS_DA */

typedef  char  *DAObjectID ;

typedef  IOR  PID_DA ;
typedef  IOR  DAObject ;
typedef  IOR  DAObjectFactory ;
typedef  IOR  DAObjectFactoryFinder ;
typedef  IOR  PDS_DA ;

/* Module: CosPersistencePO */

typedef  IOR  PO ;
typedef  IOR  SD ;

/* Module: CosPersistencePOM */

typedef  IOR  POM ;

/* Module: CosQueryCollection */

typedef  enum  ValueType {
    TypeBoolean,
    TypeChar,
    TypeOctet,
    TypeShort,
    TypeUShort,
    TypeLong,
    TypeULong,
    TypeFloat,
    TypeDouble,
    TypeString,
    TypeObject,
    TypeAny,
    TypeSmallInt,
    TypeInteger,
    TypeReal,
    TypeDoublePrecision,
    TypeCharacter,
    TypeDecimal,
    TypeNumeric
}  ValueType ;

typedef  struct  Decimal {
    long  precision ;
    long  scale ;
    OctetSeq  value ;
}  Decimal ;

typedef  struct  Value {
    ValueType  which ;
    union {
			/* TypeBoolean */
        bool  b ;
			/* TypeChar */
        char  c ;
			/* TypeOctet */
        octet  o ;
			/* TypeShort */
        short  s ;
			/* TypeUShort */
        unsigned  short  us ;
			/* TypeLong */
        long  l ;
			/* TypeULong */
        unsigned  long  ul ;
			/* TypeFloat */
        float  f ;
			/* TypeDouble */
        double  d ;
			/* TypeString */
        char  *str ;
			/* TypeObject */
        Object  obj ;
			/* TypeAny */
        Any  a ;
			/* TypeSmallInt */
        short  si ;
			/* TypeInteger */
        long  i ;
			/* TypeReal */
        float  r ;
			/* TypeDoublePrecision */
        double  dp ;
			/* TypeCharacter */
        char  *ch ;
			/* TypeDecimal */
        Decimal  dec ;
			/* TypeNumeric */
        Decimal  n ;
    }  data ;
}  Value ;

typedef  bool  Null ;

typedef  struct  FieldValue {
    Null  which ;
    union {
			/* FALSE */
        Value  v ;
    }  data ;
}  FieldValue ;

typedef  SEQUENCE (FieldValue, Record) ;

/*===== "Istring" already defined =====*/

/*===== "NVPair" already defined =====*/

/*===== "ParameterList" already defined =====*/

/* Module: CosQuery */

typedef  enum  QueryStatus {
    complete,
    incomplete
}  QueryStatus ;

/*===== "ParameterList" already defined =====*/

typedef  InterfaceDef  QLType ;

typedef  IOR  Query ;
typedef  IOR  QueryLanguageType ;
typedef  IOR  SQLQuery ;
typedef  IOR  SQL_92Query ;
typedef  IOR  OQL ;
typedef  IOR  OQLBasic ;
typedef  IOR  OQL_93 ;
typedef  IOR  OQL_93Basic ;
typedef  IOR  QueryEvaluator ;

typedef  SEQUENCE (QLType, QLTypes) ;

typedef  IOR  QueryableCollection ;
typedef  IOR  QueryManager ;

/* Module: TimeBase */

typedef  ULONGLONG  TimeT ;

typedef  TimeT  InaccuracyT ;

typedef  short  TdfT ;

typedef  struct  UtcT {
    TimeT  time ;
    unsigned  long  inacclo ;
    unsigned  short  inacchi ;
    TdfT  tdf ;
}  UtcT ;

typedef  struct  IntervalT {
    TimeT  lower_bound ;
    TimeT  upper_bound ;
}  IntervalT ;

/* Module: CosTime */

typedef  enum  TimeComparison {
    TCEqualTo,
    TCLessThan,
    TCGreaterThan,
    TCIndeterminate
}  TimeComparison ;

typedef  enum  ComparisonType {
    IntervalC,
    MidC
}  ComparisonType ;

typedef  enum  OverlapType {
    OTContainer,
    OTContained,
    OTOverlap,
    OTNoOverlap
}  OverlapType ;

typedef  IOR  TIO ;
typedef  IOR  UTO ;
typedef  IOR  TimeService ;

/* Module: CosTimerEvent */

typedef  enum  TimeType {
    TTAbsolute,
    TTRelative,
    TTPeriodic
}  TimeType ;

typedef  enum  EventStatus {
    ESTimeSet,
    ESTimeCleared,
    ESTriggered,
    ESFailedTrigger
}  EventStatus ;

typedef  struct  TimerEventT {
    UtcT  utc ;
    Any  event_data ;
}  TimerEventT ;

typedef  IOR  TimerEventHandler ;
typedef  IOR  TimerEventService ;

/* Module: CosTrading */

typedef  IOR  Lookup ;
typedef  IOR  Register ;
typedef  IOR  Link ;
typedef  IOR  Proxy ;
typedef  IOR  Admin ;
typedef  IOR  OfferIterator ;
typedef  IOR  OfferIdIterator ;

/*===== "Istring" already defined =====*/

typedef  Object  TypeRepository ;

/*===== "PropertyName" already defined =====*/

typedef  SEQUENCE (PropertyName, PropertyNameSeq) ;

typedef  Any  PropertyValue ;

/*===== "Property" already defined =====*/

typedef  SEQUENCE (Property, PropertySeq) ;

typedef  struct  Offer {
    Object  reference ;
    PropertySeq  properties ;
}  Offer ;

typedef  SEQUENCE (Offer, OfferSeq) ;

typedef  char  *OfferId ;

typedef  SEQUENCE (OfferId, OfferIdSeq) ;

typedef  Istring  ServiceTypeName ;

typedef  Istring  Constraint ;

typedef  enum  FollowOption {
    local_only,
    if_no_local,
    always
}  FollowOption ;

typedef  Istring  LinkName ;

typedef  SEQUENCE (LinkName, LinkNameSeq) ;

typedef  LinkNameSeq  TraderName ;

typedef  char  *PolicyName ;

typedef  SEQUENCE (PolicyName, PolicyNameSeq) ;

typedef  Any  PolicyValue ;

typedef  struct  COSTPolicy {
    PolicyName  name ;
    PolicyValue  value ;
}  COSTPolicy ;

typedef  SEQUENCE (COSTPolicy, COSTPolicySeq) ;

typedef  IOR  TraderComponents ;
typedef  IOR  SupportAttributes ;
typedef  IOR  ImportAttributes ;
typedef  IOR  LinkAttributes ;

typedef  Istring  Preference ;

typedef  enum  HowManyProps {
    props_none,
    props_some,
    props_all
}  HowManyProps ;

typedef  struct  SpecifiedProps {
    HowManyProps  which ;
    union {
			/* props_some */
        PropertyNameSeq  prop_names ;
    }  data ;
}  SpecifiedProps ;

typedef  struct  OfferInfo {
    Object  reference ;
    ServiceTypeName  type ;
    PropertySeq  properties ;
}  OfferInfo ;

typedef  struct  LinkInfo {
    Lookup  target ;
    Register  target_reg ;
    FollowOption  def_pass_on_follow_rule ;
    FollowOption  limiting_follow_rule ;
}  LinkInfo ;

typedef  Istring  ConstraintRecipe ;

typedef  struct  ProxyInfo {
    ServiceTypeName  type ;
    Lookup  target ;
    PropertySeq  properties ;
    bool  if_match_all ;
    ConstraintRecipe  recipe ;
    COSTPolicySeq  policies_to_pass_on ;
}  ProxyInfo ;

/*===== "OctetSeq" already defined =====*/

/* Module: CosTradingDynamic */

typedef  IOR  DynamicPropEval ;

typedef  struct  DynamicProp {
    DynamicPropEval  eval_if ;
    TypeCode  returned_type ;
    Any  extra_info ;
}  DynamicProp ;

/* Module: CosTradingRepos */

typedef  IOR  ServiceTypeRepository ;

typedef  SEQUENCE (ServiceTypeName, ServiceTypeNameSeq) ;

/*===== "PropertyMode" already defined =====*/

typedef  struct  PropStruct {
    PropertyName  name ;
    TypeCode  value_type ;
    PropertyMode  mode ;
}  PropStruct ;

typedef  SEQUENCE (PropStruct, PropStructSeq) ;

/*===== "Identifier" already defined =====*/

typedef  struct  IncarnationNumber {
    unsigned  long  high ;
    unsigned  long  low ;
}  IncarnationNumber ;

typedef  struct  TypeStruct {
    Identifier  if_name ;
    PropStructSeq  props ;
    ServiceTypeNameSeq  super_types ;
    bool  masked ;
    IncarnationNumber  incarnation ;
}  TypeStruct ;

typedef  enum  ListOption {
    all,
    since
}  ListOption ;

typedef  struct  SpecifiedServiceTypes {
    ListOption  which ;
    union {
			/* since */
        IncarnationNumber  incarnation ;
    }  data ;
}  SpecifiedServiceTypes ;

/* Module: CosTSPortability */

typedef  long  ReqId ;

typedef  IOR  Sender ;
typedef  IOR  Receiver ;

/* Module: CosEventChannelAdmin */

typedef  IOR  ProxyPushConsumer ;
typedef  IOR  ProxyPullSupplier ;
typedef  IOR  ProxyPullConsumer ;
typedef  IOR  ProxyPushSupplier ;
typedef  IOR  ConsumerAdmin ;
typedef  IOR  SupplierAdmin ;
typedef  IOR  EventChannel ;

/* Module: CosTypedEventComm */

typedef  IOR  TypedPushConsumer ;
typedef  IOR  TypedPullSupplier ;

/* Module: CosTypedEventChannelAdmin */

/*===== "Key" already defined =====*/

typedef  IOR  TypedProxyPushConsumer ;
typedef  IOR  TypedProxyPullSupplier ;
typedef  IOR  TypedSupplierAdmin ;
typedef  IOR  TypedConsumerAdmin ;
typedef  IOR  TypedEventChannel ;

/* Module: DCE_CIOPSecurity */

/*===== "TAG_DCE_SEC_MECH" previously defined in IOP =====*/

typedef  unsigned  short  DCEAuthorization ;

#define  DCEAuthorizationNone  (0)
#define  DCEAuthorizationName  (1)
#define  DCEAuthorizationDCE  (2)

typedef  struct  DCESecurityMechanismInfo {
    DCEAuthorization  authorization_service ;
    SEQUENCE (TaggedComponent, components) ;
}  DCESecurityMechanismInfo ;

/* Module: LifeCycleService */

/*===== "PolicyList" already defined =====*/

typedef  SEQUENCE (Key, COSLCSKeys) ;

typedef  SEQUENCE (NameValuePair, PropertyList) ;

typedef  SEQUENCE (NameComponent, NameComponents) ;

typedef  IOR  LifeCycleServiceAdmin ;

typedef  IOR  LNameComponent ;
typedef  IOR  LName ;

/* Module: Security */

typedef  char  *SecurityName ;

typedef  OctetSeq  Opaque ;

#define  SecurityLevel1  (1)
#define  SecurityLevel2  (2)
#define  NonRepudiation  (3)
#define  SecurityORBServiceRaady  (4)
#define  SecurityServiceReady  (5)
#define  ReplaceORBServices  (6)
#define  ReplaceSecurityServices  (7)
#define  StandardSecureInteroperability  (8)
#define  DCESecureInteroperability  (9)
#define  CommonInteroperabilityLevel0  (10)
#define  CommonInteroperabilityLevel1  (11)
#define  CommonInteroperabilityLevel2  (12)
#define  SecurityMechanismType  (1)
#define  SecurityAttribute  (2)

typedef  struct  ExtensibleFamily {
    unsigned  short  family_definer ;
    unsigned  short  family ;
}  ExtensibleFamily ;

typedef  char  *MechanismType ;

typedef  struct  SecurityMechandName {
    MechanismType  mech_type ;
    SecurityName  security_name ;
}  SecurityMechandName ;

typedef  SEQUENCE (MechanismType, MechanismTypeList) ;

typedef  SEQUENCE (SecurityMechandName, SecurityMechandNameList) ;

typedef  unsigned  long  SecurityAttributeType ;

#define  AuditId  (1)
#define  AccountingId  (2)
#define  NonRepudiationId  (3)
#define  Public  (1)
#define  AccessId  (2)
#define  PrimaryGroupId  (3)
#define  GroupId  (4)
#define  Role  (5)
#define  AttributeSet  (6)
#define  Clearance  (7)
#define  Capability  (8)

typedef  struct  AttributeType {
    ExtensibleFamily  attribute_family ;
    SecurityAttributeType  attribute_type ;
}  AttributeType ;

typedef  SEQUENCE (AttributeType, AttributeTypeList) ;

typedef  struct  SecAttribute {
    AttributeType  attribute_type ;
    Opaque  defining_authority ;
    Opaque  value ;
}  SecAttribute ;

typedef  SEQUENCE (SecAttribute, AttributeList) ;

typedef  enum  AuthenticationStatus {
    SecAuthSuccess,
    SecAuthFailure,
    SecAuthContinue,
    SecAuthExpired
}  AuthenticationStatus ;

typedef  enum  AssociationStatus {
    SecAssocSuccess,
    SecAssocFailure,
    SecAssocContinue
}  AssociationStatus ;

typedef  unsigned  long  AuthenticationMethod ;

typedef  enum  CredentialType {
    SecInvocationCredentials,
    SecNRCredentials
}  CredentialType ;

typedef  struct  Right {
    ExtensibleFamily  rights_family ;
    char  *right ;
}  Right ;

typedef  SEQUENCE (Right, RightsList) ;

typedef  enum  RightsCombinator {
    SecAllRights,
    SecAnyRight
}  RightsCombinator ;

typedef  enum  DelegationState {
    SecInitiator,
    SecDelegate
}  DelegationState ;

/*===== "UtcT" already defined =====*/

/*===== "IntervalT" already defined =====*/

/*===== "TimeT" already defined =====*/

typedef  enum  SecurityFeature {
    SecNoDelegation,
    SecSimpleDelegation,
    SecCompositeDelegation,
    SecNoProtection,
    SecIntegrity,
    SecConfidentiality,
    SecIntegrityAndConfidentiality,
    SecDetectReplay,
    SecDetectMisordering,
    SecEstablishTrustInTarget
}  SecurityFeature ;

typedef  struct  SecurityFeatureValue {
    SecurityFeature  feature ;
    bool  value ;
}  SecurityFeatureValue ;

typedef  SEQUENCE (SecurityFeatureValue, SecurityFeatureValueList) ;

typedef  enum  QOP {
    SecQOPNoProtection,
    SecQOPIntegrity,
    SecQOPConfidentiality,
    SecQOPIntegrityAndConfidentiality
}  QOP ;

/*===== "AssociationOptions" already defined =====*/

/*===== "NoProtection" previously defined in CSIIOP =====*/

/*===== "Integrity" previously defined in CSIIOP =====*/

/*===== "Confidentiality" previously defined in CSIIOP =====*/

/*===== "DetectReplay" previously defined in CSIIOP =====*/

/*===== "DetectMisordering" previously defined in CSIIOP =====*/

/*===== "EstablishTrustInTarget" previously defined in CSIIOP =====*/

/*===== "EstablishTrustInClient" previously defined in CSIIOP =====*/

typedef  enum  RequiresSupports {
    SecRequires,
    SecSupports
}  RequiresSupports ;

typedef  enum  CommunicationDirection {
    SecDirectionBoth,
    SecDirectionRequest,
    SecDirectionReply
}  CommunicationDirection ;

typedef  struct  OptionsDirectionPair {
    AssociationOptions  options ;
    CommunicationDirection  direction ;
}  OptionsDirectionPair ;

typedef  SEQUENCE (OptionsDirectionPair, OptionsDirectionPairList) ;

typedef  enum  DelegationMode {
    SecDelModeNoDelegation,
    SecDelModeSimpleDelegation,
    SecDelModeCompositeDelegation
}  DelegationMode ;

typedef  struct  MechandOptions {
    MechanismType  mechanism_type ;
    AssociationOptions  options_supported ;
}  MechandOptions ;

typedef  SEQUENCE (MechandOptions, MechandOptionsList) ;

typedef  unsigned  long  AuditChannelId ;

typedef  unsigned  short  COSSEventType ;

#define  AuditAll  (0)
#define  AuditPrincipalAuth  (1)
#define  AuditSessionAuth  (2)
#define  AuditAuthorization  (3)
#define  AuditInvocation  (4)
#define  AuditSecEnvChange  (5)
#define  AuditPolicyChange  (6)
#define  AuditObjectCreation  (7)
#define  AuditObjectDestruction  (8)
#define  AuditNonRepudiation  (9)

typedef  struct  AuditEventType {
    ExtensibleFamily  event_family ;
    COSSEventType  event_type ;
}  AuditEventType ;

typedef  SEQUENCE (AuditEventType, AuditEventTypeList) ;

typedef  unsigned  long  SelectorType ;

#define  InterfaceRef  (1)
#define  ObjectRef  (2)
#define  Operation  (3)
#define  Initiator  (4)
#define  SuccessFailure  (5)
#define  Time  (6)

typedef  struct  SelectorValue {
    SelectorType  selector ;
    Any  value ;
}  SelectorValue ;

typedef  SEQUENCE (SelectorValue, SelectorValueList) ;

#define  SecClientInvocationAccess  (1)
#define  SecTargetInvocationAccess  (2)
#define  SecApplicationAccess  (3)
#define  SecClientInvocationAudit  (4)
#define  SecTargetInvocationAudit  (5)
#define  SecApplicationAudit  (6)
#define  SecDelegation  (7)
#define  SecClientSecureInvocation  (8)
#define  SecTargetSecureInvocation  (9)
#define  SecNonRepudiation  (10)
#define  SecMechanismsPolicy  (12)
#define  SecCredentialsPolicy  (13)
#define  SecFeaturesPolicy  (14)
#define  SecQOPPolicy  (15)

/* Module: SecurityLevel1 */

/* Module: SecurityLevel2 */

typedef  IOR  PrincipalAuthenticator ;
typedef  IOR  Credentials ;

typedef  SEQUENCE (Credentials, CredentialsList) ;

typedef  IOR  RequiredRights ;
typedef  IOR  AuditChannel ;
typedef  IOR  AuditDecision ;
typedef  IOR  AccessDecision ;
typedef  IOR  QOPPolicy ;
typedef  IOR  MechanismPolicy ;
typedef  IOR  SecurityFeaturesPolicy ;
typedef  IOR  InvocationCredentialsPolicy ;

/* Module: NRService */

typedef  MechanismType  NRMech ;

typedef  ExtensibleFamily  NRPolicyId ;

typedef  enum  EvidenceType {
    SecProofofCreation,
    SecProofofReceipt,
    SecProofofApproval,
    SecProofofRetrieval,
    SecProofofOrigin,
    SecProofofDelivery,
    SecNoEvidence
}  EvidenceType ;

typedef  enum  NRVerificationResult {
    SecNRInvalid,
    SecNRValid,
    SecNRConditionallyValid
}  NRVerificationResult ;

typedef  unsigned  long  DurationInMinutes ;

#define  DurationHour  (60)
#define  DurationDay  (1440)
#define  DurationWeek  (10080)
#define  DurationMonth  (43200)
#define  DurationYear  (525600)

typedef  long  TimeOffsetInMinutes ;

typedef  struct  NRPolicyFeatures {
    NRPolicyId  policy_id ;
    unsigned  long  policy_version ;
    NRMech  mechanism ;
}  NRPolicyFeatures ;

typedef  SEQUENCE (NRPolicyFeatures, NRPolicyFeaturesList) ;

typedef  struct  RequestFeatures {
    NRPolicyFeatures  requested_policy ;
    EvidenceType  requested_evidence ;
    char  *requested_evidence_generators ;
    char  *requested_evidence_recipients ;
    bool  include_this_token_in_evidence ;
}  RequestFeatures ;

typedef  struct  EvidenceDescriptor {
    EvidenceType  evidence_type ;
    DurationInMinutes  evidence_validity_duration ;
    bool  must_use_trusted_time ;
}  EvidenceDescriptor ;

typedef  SEQUENCE (EvidenceDescriptor, EvidenceDescriptorList) ;

typedef  struct  AuthorityDescriptor {
    char  *authority_name ;
    char  *authority_role ;
    TimeOffsetInMinutes  last_revocation_check_offset ;
}  AuthorityDescriptor ;

typedef  SEQUENCE (AuthorityDescriptor, AuthorityDescriptorList) ;

typedef  struct  MechanismDescriptor {
    NRMech  mech_type ;
    AuthorityDescriptorList  authority_list ;
    TimeOffsetInMinutes  max_time_skew ;
}  MechanismDescriptor ;

typedef  SEQUENCE (MechanismDescriptor, MechanismDescriptorList) ;

typedef  IOR  NRCredentials ;
typedef  IOR  NRPolicy ;

/* Module: SECIOP */

/*===== "TAG_GENERIC_SEC_MECH" previously defined in IOP =====*/

/*===== "TAG_ASSOCIATION_OPTIONS" previously defined in IOP =====*/

/*===== "TAG_SEC_NAME" previously defined in IOP =====*/

typedef  struct  TargetAssociationOptions {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
}  TargetAssociationOptions ;

typedef  struct  GenericMechanismInfo {
    OctetSeq  security_mechanism_type ;
    OctetSeq  mech_specific_data ;
    SEQUENCE (TaggedComponent, components) ;
}  GenericMechanismInfo ;

/*===== "ContextId" already defined =====*/

typedef  enum  ContextIdDefn {
    CIDClient,
    CIDPeer,
    CIDSender
}  ContextIdDefn ;

/*===== "EstablishContext" already defined =====*/

/*===== "CompleteEstablishContext" already defined =====*/

typedef  struct  ContinueEstablishContext {
    ContextId  client_context_id ;
    OctetSeq  continuation_context_token ;
}  ContinueEstablishContext ;

typedef  struct  DiscardContext {
    ContextIdDefn  message_context_id_defn ;
    ContextId  message_context_id ;
    OctetSeq  discard_context_token ;
}  DiscardContext ;

typedef  enum  ContextTokenType {
    SecTokenTypeWrap,
    SecTokenTypeMIC
}  ContextTokenType ;

/*===== "MessageInContext" already defined =====*/

typedef  struct  SequencingHeader {
    octet  control_state ;
    unsigned  long  direct_sequence_number ;
    unsigned  long  reverse_sequence_number ;
    unsigned  long  reverse_window ;
}  SequencingHeader ;

/*===== "SecurityName" already defined =====*/

typedef  unsigned  short  CryptographicProfile ;

typedef  SEQUENCE (CryptographicProfile, CryptographicProfileList) ;

#define  MD5_RSA  (20)
#define  MD5_DES_CBC  (21)
#define  DES_CBC  (22)
#define  MD5_DES_CBC_SOURCE  (23)
#define  DES_CBC_SOURCE  (24)

/*===== "TAG_SPKM_1_SEC_MECH" previously defined in IOP =====*/

typedef  struct  SPKM_1 {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    CryptographicProfileList  crypto_profile ;
    SecurityName  security_name ;
}  SPKM_1 ;

/*===== "TAG_SPKM_2_SEC_MECH" previously defined in IOP =====*/

typedef  struct  SPKM_2 {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    CryptographicProfileList  crypto_profile ;
    SecurityName  security_name ;
}  SPKM_2 ;

#define  DES_CBC_DES_MAC  (10)
#define  DES_CBC_MD5  (11)
#define  DES_MAC  (12)
#define  MD5  (13)

/*===== "TAG_KerberosV5_SEC_MECH" previously defined in IOP =====*/

typedef  struct  KerberosV5 {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    CryptographicProfileList  crypto_profile ;
    SecurityName  security_name ;
}  KerberosV5 ;

#define  FullSecurity  (1)
#define  NoDataConfidentiality  (2)
#define  LowGradeConfidentiality  (3)
#define  AgreedDefault  (5)

/*===== "TAG_CSI_ECMA_Secret_SEC_MECH" previously defined in IOP =====*/

typedef  struct  CSI_ECMA_Secret {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    CryptographicProfileList  crypto_profile ;
    SecurityName  security_name ;
}  CSI_ECMA_Secret ;

/*===== "TAG_CSI_ECMA_Hybrid_SEC_MECH" previously defined in IOP =====*/

typedef  struct  CSI_ECMA_Hybrid {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    CryptographicProfileList  crypto_profile ;
    SecurityName  security_name ;
}  CSI_ECMA_Hybrid ;

/*===== "TAG_CSI_ECMA_Public_SEC_MECH" previously defined in IOP =====*/

typedef  struct  CSI_ECMA_Public {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    CryptographicProfileList  crypto_profile ;
    SecurityName  security_name ;
}  CSI_ECMA_Public ;

/* Module: SecurityAdmin */

typedef  IOR  AccessPolicy ;
typedef  IOR  DomainAccessPolicy ;
typedef  IOR  AuditPolicy ;
typedef  IOR  SecureInvocationPolicy ;
typedef  IOR  DelegationPolicy ;

/* Module: SecurityReplaceable */

typedef  IOR  SecurityContext ;
typedef  IOR  Vault ;

/* Module: SSLIOP */

#define  TAG_SSL_SEC_TRANS  (20)

typedef  struct  SSL {
    AssociationOptions  target_supports ;
    AssociationOptions  target_requires ;
    unsigned  short  port ;
}  SSL ;

/* Module: Dynamic */

typedef  struct  Parameter {
    Any  argument ;
    ParameterMode  mode ;
}  Parameter ;

/*===== "ParameterList" already defined =====*/

typedef  StringSeq  ContextList ;

typedef  SEQUENCE (TypeCode, ExceptionList) ;

typedef  StringSeq  RequestContext ;

/* Module: Messaging */

typedef  short  RebindMode ;

#define  MESSAGING_TRANSPARENT_REBIND  (0)
#define  MESSAGING_NO_REBIND  (1)
#define  MESSAGING_NO_RECONNECT  (2)

typedef  short  SyncScope ;

#define  MESSAGING_SYNC_NONE  (0)
#define  MESSAGING_SYNC_WITH_TRANSPORT  (1)
#define  MESSAGING_SYNC_WITH_SERVER  (2)
#define  MESSAGING_SYNC_WITH_TARGET  (3)

typedef  short  RoutingType ;

#define  MESSAGING_ROUTE_NONE  (0)
#define  MESSAGING_ROUTE_FORWARD  (1)
#define  MESSAGING_ROUTE_STORE_AND_FORWARD  (2)

typedef  short  Priority ;

typedef  unsigned  short  Ordering ;

#define  MESSAGING_ORDER_ANY  (0x01)
#define  MESSAGING_ORDER_TEMPORAL  (0x02)
#define  MESSAGING_ORDER_PRIORITY  (0x04)
#define  MESSAGING_ORDER_DEADLINE  (0x08)
#define  MESSAGING_REBIND_POLICY_TYPE  (23)
#define  MESSAGING_SYNC_SCOPE_POLICY_TYPE  (24)
#define  MESSAGING_REQUEST_PRIORITY_POLICY_TYPE  (25)

typedef  struct  PriorityRange {
    Priority  min ;
    Priority  max ;
}  PriorityRange ;

#define  MESSAGING_REPLY_PRIORITY_POLICY_TYPE  (26)

typedef  IOR  ReplyPriorityPolicy ;

#define  MESSAGING_REQUEST_START_TIME_POLICY_TYPE  (27)
#define  MESSAGING_REQUEST_END_TIME_POLICY_TYPE  (28)
#define  MESSAGING_REPLY_START_TIME_POLICY_TYPE  (29)
#define  MESSAGING_REPLY_END_TIME_POLICY_TYPE  (30)
#define  MESSAGING_RELATIVE_REQ_TIMEOUT_POLICY_TYPE  (31)
#define  MESSAGING_RELATIVE_RT_TIMEOUT_POLICY_TYPE  (32)
#define  MESSAGING_ROUTING_POLICY_TYPE  (33)

typedef  struct  RoutingTypeRange {
    RoutingType  min ;
    RoutingType  max ;
}  RoutingTypeRange ;

#define  MESSAGING_MAX_HOPS_POLICY_TYPE  (34)
#define  MESSAGING_QUEUE_ORDER_POLICY_TYPE  (35)

typedef  SEQUENCE (PolicyValue, PolicyValueSeq) ;

typedef  IOR  ReplyHandler ;

/*******************************************************************************
    Tables for mapping enumerated values to names and vice-versa;
    see the coliToName() and coliToNumber() functions.
*******************************************************************************/

extern  const  ColiMap  ActionRequiredLUT[]  OCD ("CosLicen") ;
extern  const  ColiMap  AnswerLUT[]  OCD ("CosLicen") ;
extern  const  ColiMap  AssociationStatusLUT[]  OCD ("Security") ;
extern  const  ColiMap  AttributeModeLUT[]  OCD ("CORBA") ;
extern  const  ColiMap  AuthenticationStatusLUT[]  OCD ("Security") ;
extern  const  ColiMap  BindingTypeLUT[]  OCD ("CosNamin") ;
extern  const  ColiMap  ChallengeProtocolLUT[]  OCD ("CosLicen") ;
extern  const  ColiMap  CommunicationDirectionLUT[]  OCD ("Security") ;
extern  const  ColiMap  ComparisonTypeLUT[]  OCD ("CosTime") ;
extern  const  ColiMap  ContextIdDefnLUT[]  OCD ("SECIOP") ;
extern  const  ColiMap  ContextTokenTypeLUT[]  OCD ("SECIOP") ;
extern  const  ColiMap  CredentialTypeLUT[]  OCD ("Security") ;
extern  const  ColiMap  DefinitionKindLUT[]  OCD ("CORBA") ;
extern  const  ColiMap  DelegationModeLUT[]  OCD ("Security") ;
extern  const  ColiMap  DelegationStateLUT[]  OCD ("Security") ;
extern  const  ColiMap  ElementInvalidReasonLUT[]  OCD ("CosColle") ;
extern  const  ColiMap  EventStatusLUT[]  OCD ("CosTimer") ;
extern  const  ColiMap  EvidenceTypeLUT[]  OCD ("NRServic") ;
extern  const  ColiMap  ExceptionReasonLUT[]  OCD ("CosPrope") ;
extern  const  ColiMap  FollowOptionLUT[]  OCD ("CosTradi") ;
extern  const  ColiMap  HowManyPropsLUT[]  OCD ("CosTradi") ;
extern  const  ColiMap  IteratorInvalidReasonLUT[]  OCD ("CosColle") ;
extern  const  ColiMap  ListOptionLUT[]  OCD ("CosTradi") ;
extern  const  ColiMap  lock_modeLUT[]  OCD ("CosConcu") ;
extern  const  ColiMap  LowerBoundStyleLUT[]  OCD ("CosColle") ;
extern  const  ColiMap  ModeLUT[]  OCD ("CosGraph") ;
extern  const  ColiMap  NotFoundReasonLUT[]  OCD ("CosNamin") ;
extern  const  ColiMap  NRVerificationResultLUT[]  OCD ("NRServic") ;
extern  const  ColiMap  OperationLUT[]  OCD ("CosCompo") ;
extern  const  ColiMap  OperationModeLUT[]  OCD ("CORBA") ;
extern  const  ColiMap  OverlapTypeLUT[]  OCD ("CosTime") ;
extern  const  ColiMap  ParameterModeLUT[]  OCD ("CORBA") ;
extern  const  ColiMap  PrimitiveKindLUT[]  OCD ("CORBA") ;
extern  const  ColiMap  PropagationValueLUT[]  OCD ("CosGraph") ;
extern  const  ColiMap  PropertyModeTypeLUT[]  OCD ("CosPrope") ;
extern  const  ColiMap  QOPLUT[]  OCD ("Security") ;
extern  const  ColiMap  QueryStatusLUT[]  OCD ("CosQuery") ;
extern  const  ColiMap  RequiresSupportsLUT[]  OCD ("Security") ;
extern  const  ColiMap  RightsCombinatorLUT[]  OCD ("Security") ;
extern  const  ColiMap  SecurityFeatureLUT[]  OCD ("Security") ;
extern  const  ColiMap  StatusLUT[]  OCD ("CosTrans") ;
extern  const  ColiMap  TCKindLUT[]  OCD ("CORBA") ;
extern  const  ColiMap  TimeComparisonLUT[]  OCD ("CosTime") ;
extern  const  ColiMap  TimeTypeLUT[]  OCD ("CosTimer") ;
extern  const  ColiMap  UpperBoundStyleLUT[]  OCD ("CosColle") ;
extern  const  ColiMap  ValueTypeLUT[]  OCD ("CosQuery") ;
extern  const  ColiMap  VoteLUT[]  OCD ("CosTrans") ;

/*******************************************************************************
    Public functions.
*******************************************************************************/

/* Marshaling functions for the defined data types. */

extern  errno_t  gimxAbstractInterfaceDefSeq P_((ComxChannel channel,
                                                 AbstractInterfaceDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxAction P_((ComxChannel channel,
                                Action *value))
    OCD ("CosLicen") ;

extern  errno_t  gimxAnySequence P_((ComxChannel channel,
                                     AnySequence *value))
    OCD ("CosColle") ;

extern  errno_t  gimxAS_ContextSec P_((ComxChannel channel,
                                       AS_ContextSec *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxAttrDescriptionSeq P_((ComxChannel channel,
                                            AttrDescriptionSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxAttributeDescription P_((ComxChannel channel,
                                              AttributeDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxAttributeList P_((ComxChannel channel,
                                       AttributeList *value))
    OCD ("Security") ;

extern  errno_t  gimxAttributeType P_((ComxChannel channel,
                                       AttributeType *value))
    OCD ("Security") ;

extern  errno_t  gimxAttributeTypeList P_((ComxChannel channel,
                                           AttributeTypeList *value))
    OCD ("Security") ;

extern  errno_t  gimxAuditEventType P_((ComxChannel channel,
                                        AuditEventType *value))
    OCD ("Security") ;

extern  errno_t  gimxAuditEventTypeList P_((ComxChannel channel,
                                            AuditEventTypeList *value))
    OCD ("Security") ;

extern  errno_t  gimxAuthorityDescriptor P_((ComxChannel channel,
                                             AuthorityDescriptor *value))
    OCD ("NRServic") ;

extern  errno_t  gimxAuthorityDescriptorList P_((ComxChannel channel,
                                                 AuthorityDescriptorList *value))
    OCD ("NRServic") ;

extern  errno_t  gimxAuthorizationElement P_((ComxChannel channel,
                                              AuthorizationElement *value))
    OCD ("CSI") ;

extern  errno_t  gimxAuthorizationToken P_((ComxChannel channel,
                                            AuthorizationToken *value))
    OCD ("CSI") ;

extern  errno_t  gimxBiDirIIOPServiceContext P_((ComxChannel channel,
                                                 BiDirIIOPServiceContext *value))
    OCD ("IIOP") ;

extern  errno_t  gimxBinding P_((ComxChannel channel,
                                 Binding *value))
    OCD ("CosNamin") ;

extern  errno_t  gimxBindingList P_((ComxChannel channel,
                                     BindingList *value))
    OCD ("CosNamin") ;

extern  errno_t  gimxCancelRequestHeader P_((ComxChannel channel,
                                             CancelRequestHeader *value))
    OCD ("GIOP") ;

extern  errno_t  gimxChallenge P_((ComxChannel channel,
                                   Challenge *value))
    OCD ("CosLicen") ;

extern  errno_t  gimxChallengeData P_((ComxChannel channel,
                                       ChallengeData *value))
    OCD ("CosLicen") ;

extern  errno_t  gimxCodeSetComponent P_((ComxChannel channel,
                                          CodeSetComponent *value))
    OCD ("CONV_FRA") ;

extern  errno_t  gimxCodeSetComponentInfo P_((ComxChannel channel,
                                              CodeSetComponentInfo *value))
    OCD ("CONV_FRA") ;

extern  errno_t  gimxCodeSetContext P_((ComxChannel channel,
                                        CodeSetContext *value))
    OCD ("CONV_FRA") ;

extern  errno_t  gimxCompleteEstablishContext P_((ComxChannel channel,
                                                  CompleteEstablishContext *value))
    OCD ("CSI") ;

extern  errno_t  gimxComponentDescription P_((ComxChannel channel,
                                              ComponentDescription *value))
    OCD ("Componen") ;

extern  errno_t  gimxComponentId P_((ComxChannel channel,
                                     ComponentId *value))
    OCD ("IOP") ;

extern  errno_t  gimxCompoundSecMech P_((ComxChannel channel,
                                         CompoundSecMech *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxCompoundSecMechanisms P_((ComxChannel channel,
                                               CompoundSecMechanisms *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxCompoundSecMechList P_((ComxChannel channel,
                                             CompoundSecMechList *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxConstantDescription P_((ComxChannel channel,
                                             ConstantDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxContainedSeq P_((ComxChannel channel,
                                      ContainedSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxContextError P_((ComxChannel channel,
                                      ContextError *value))
    OCD ("CSI") ;

extern  errno_t  gimxContextIdSeq P_((ComxChannel channel,
                                      ContextIdSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxContinueEstablishContext P_((ComxChannel channel,
                                                  ContinueEstablishContext *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxCOSLCSKeys P_((ComxChannel channel,
                                    COSLCSKeys *value))
    OCD ("LifeCycl") ;

extern  errno_t  gimxCOSTPolicy P_((ComxChannel channel,
                                    COSTPolicy *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxCOSTPolicySeq P_((ComxChannel channel,
                                       COSTPolicySeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxCredentialsList P_((ComxChannel channel,
                                         CredentialsList *value))
    OCD ("Security") ;

extern  errno_t  gimxCriteria P_((ComxChannel channel,
                                  Criteria *value))
    OCD ("CosLifeC") ;

extern  errno_t  gimxCryptographicProfileList P_((ComxChannel channel,
                                                  CryptographicProfileList *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxCSI_ECMA_Hybrid P_((ComxChannel channel,
                                         CSI_ECMA_Hybrid *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxCSI_ECMA_Public P_((ComxChannel channel,
                                         CSI_ECMA_Public *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxCSI_ECMA_Secret P_((ComxChannel channel,
                                         CSI_ECMA_Secret *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxDCESecurityMechanismInfo P_((ComxChannel channel,
                                                  DCESecurityMechanismInfo *value))
    OCD ("DCE_CIOP") ;

extern  errno_t  gimxDecimal P_((ComxChannel channel,
                                 Decimal *value))
    OCD ("CosQuery") ;

extern  errno_t  gimxDescription P_((ComxChannel channel,
                                     Description *value))
    OCD ("CORBA") ;

extern  errno_t  gimxDescriptionSeq P_((ComxChannel channel,
                                        DescriptionSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxDiscardContext P_((ComxChannel channel,
                                        DiscardContext *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxDomainManagersList P_((ComxChannel channel,
                                            DomainManagersList *value))
    OCD ("CORBA") ;

extern  errno_t  gimxDynamicProp P_((ComxChannel channel,
                                     DynamicProp *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxEdge P_((ComxChannel channel,
                              Edge *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxEdges P_((ComxChannel channel,
                               Edges *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxEncoding P_((ComxChannel channel,
                                  Encoding *value))
    OCD ("IOP") ;

extern  errno_t  gimxEndPoint P_((ComxChannel channel,
                                  EndPoint *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxEndpointIdPositionComponent P_((ComxChannel channel,
                                                     EndpointIdPositionComponent *value))
    OCD ("IOP") ;

extern  errno_t  gimxEndPoints P_((ComxChannel channel,
                                   EndPoints *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxEnumMemberSeq P_((ComxChannel channel,
                                       EnumMemberSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxErrorToken P_((ComxChannel channel,
                                    ErrorToken *value))
    OCD ("GSSUP") ;

extern  errno_t  gimxEstablishContext P_((ComxChannel channel,
                                          EstablishContext *value))
    OCD ("CSI") ;

extern  errno_t  gimxEventPortDescription P_((ComxChannel channel,
                                              EventPortDescription *value))
    OCD ("Componen") ;

extern  errno_t  gimxEventPortDescriptionSeq P_((ComxChannel channel,
                                                 EventPortDescriptionSeq *value))
    OCD ("Componen") ;

extern  errno_t  gimxEvidenceDescriptor P_((ComxChannel channel,
                                            EvidenceDescriptor *value))
    OCD ("NRServic") ;

extern  errno_t  gimxEvidenceDescriptorList P_((ComxChannel channel,
                                                EvidenceDescriptorList *value))
    OCD ("NRServic") ;

extern  errno_t  gimxExcDescriptionSeq P_((ComxChannel channel,
                                           ExcDescriptionSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExceptionDefSeq P_((ComxChannel channel,
                                         ExceptionDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExceptionDescription P_((ComxChannel channel,
                                              ExceptionDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExceptionList P_((ComxChannel channel,
                                       ExceptionList *value))
    OCD ("Dynamic") ;

extern  errno_t  gimxExtAbstractInterfaceDefSeq P_((ComxChannel channel,
                                                    ExtAbstractInterfaceDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtAttrDescriptionSeq P_((ComxChannel channel,
                                               ExtAttrDescriptionSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtAttributeDescription P_((ComxChannel channel,
                                                 ExtAttributeDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtensibleFamily P_((ComxChannel channel,
                                          ExtensibleFamily *value))
    OCD ("Security") ;

extern  errno_t  gimxExtFullInterfaceDescription P_((ComxChannel channel,
                                                     ExtFullInterfaceDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtFullValueDescription P_((ComxChannel channel,
                                                 ExtFullValueDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtInitializer P_((ComxChannel channel,
                                        ExtInitializer *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtInitializerSeq P_((ComxChannel channel,
                                           ExtInitializerSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtInterfaceDefSeq P_((ComxChannel channel,
                                            ExtInterfaceDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtLocalInterfaceDefSeq P_((ComxChannel channel,
                                                 ExtLocalInterfaceDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxExtValueDefSeq P_((ComxChannel channel,
                                        ExtValueDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxFactories P_((ComxChannel channel,
                                   Factories *value))
    OCD ("CosLifeC") ;

extern  errno_t  gimxFieldValue P_((ComxChannel channel,
                                    FieldValue *value))
    OCD ("CosQuery") ;

extern  errno_t  gimxFragmentHeader P_((ComxChannel channel,
                                        FragmentHeader *value))
    OCD ("GIOP") ;

extern  errno_t  gimxFullInterfaceDescription P_((ComxChannel channel,
                                                  FullInterfaceDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxFullValueDescription P_((ComxChannel channel,
                                              FullValueDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxGenericMechanismInfo P_((ComxChannel channel,
                                              GenericMechanismInfo *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxGSS_NT_ExportedNameList P_((ComxChannel channel,
                                                 GSS_NT_ExportedNameList *value))
    OCD ("CSI") ;

extern  errno_t  gimxHomeDescription P_((ComxChannel channel,
                                         HomeDescription *value))
    OCD ("Componen") ;

extern  errno_t  gimxIdentityToken P_((ComxChannel channel,
                                       IdentityToken *value))
    OCD ("CSI") ;

extern  errno_t  gimxIncarnationNumber P_((ComxChannel channel,
                                           IncarnationNumber *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxInitialContextToken P_((ComxChannel channel,
                                             InitialContextToken *value))
    OCD ("GSSUP") ;

extern  errno_t  gimxInitializer P_((ComxChannel channel,
                                     Initializer *value))
    OCD ("CORBA") ;

extern  errno_t  gimxInitializerSeq P_((ComxChannel channel,
                                        InitializerSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxInterfaceDefs P_((ComxChannel channel,
                                       InterfaceDefs *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxInterfaceDefSeq P_((ComxChannel channel,
                                         InterfaceDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxInterfaceDescription P_((ComxChannel channel,
                                              InterfaceDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxIntervalT P_((ComxChannel channel,
                                   IntervalT *value))
    OCD ("TimeBase") ;

extern  errno_t  gimxIOR P_((ComxChannel channel,
                             IOR *value))
    OCD ("IOP") ;

extern  errno_t  gimxIORAddressingInfo P_((ComxChannel channel,
                                           IORAddressingInfo *value))
    OCD ("GIOP") ;

extern  errno_t  gimxKerberosV5 P_((ComxChannel channel,
                                    KerberosV5 *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxLinkInfo P_((ComxChannel channel,
                                  LinkInfo *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxLinkNameSeq P_((ComxChannel channel,
                                     LinkNameSeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxListenPoint P_((ComxChannel channel,
                                     ListenPoint *value))
    OCD ("IIOP") ;

extern  errno_t  gimxListenPointList P_((ComxChannel channel,
                                         ListenPointList *value))
    OCD ("IIOP") ;

extern  errno_t  gimxLocalInterfaceDefSeq P_((ComxChannel channel,
                                              LocalInterfaceDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxLocateReplyHeader P_((ComxChannel channel,
                                           LocateReplyHeader *value))
    OCD ("GIOP") ;

extern  errno_t  gimxLocateReplyHeader_1_0 P_((ComxChannel channel,
                                               LocateReplyHeader_1_0 *value))
    OCD ("GIOP") ;

extern  errno_t  gimxLocateRequestHeader P_((ComxChannel channel,
                                             LocateRequestHeader *value))
    OCD ("GIOP") ;

extern  errno_t  gimxLocateRequestHeader_1_0 P_((ComxChannel channel,
                                                 LocateRequestHeader_1_0 *value))
    OCD ("GIOP") ;

extern  errno_t  gimxMechandOptions P_((ComxChannel channel,
                                        MechandOptions *value))
    OCD ("Security") ;

extern  errno_t  gimxMechandOptionsList P_((ComxChannel channel,
                                            MechandOptionsList *value))
    OCD ("Security") ;

extern  errno_t  gimxMechanismDescriptor P_((ComxChannel channel,
                                             MechanismDescriptor *value))
    OCD ("NRServic") ;

extern  errno_t  gimxMechanismDescriptorList P_((ComxChannel channel,
                                                 MechanismDescriptorList *value))
    OCD ("NRServic") ;

extern  errno_t  gimxMechanismTypeList P_((ComxChannel channel,
                                           MechanismTypeList *value))
    OCD ("Security") ;

extern  errno_t  gimxMessageHeader P_((ComxChannel channel,
                                       MessageHeader *value))
    OCD ("GIOP") ;

extern  errno_t  gimxMessageHeader_1_0 P_((ComxChannel channel,
                                           MessageHeader_1_0 *value))
    OCD ("GIOP") ;

extern  errno_t  gimxMessageInContext P_((ComxChannel channel,
                                          MessageInContext *value))
    OCD ("CSI") ;

extern  errno_t  gimxModuleDescription P_((ComxChannel channel,
                                           ModuleDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxMultipleComponentProfile P_((ComxChannel channel,
                                                  MultipleComponentProfile *value))
    OCD ("IOP") ;

extern  errno_t  gimxName P_((ComxChannel channel,
                              Name *value))
    OCD ("CosNamin") ;

extern  errno_t  gimxNameComponent P_((ComxChannel channel,
                                       NameComponent *value))
    OCD ("CosNamin") ;

extern  errno_t  gimxNameComponents P_((ComxChannel channel,
                                        NameComponents *value))
    OCD ("LifeCycl") ;

extern  errno_t  gimxNamedRole P_((ComxChannel channel,
                                   NamedRole *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxNamedRoles P_((ComxChannel channel,
                                    NamedRoles *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxNamedRoleType P_((ComxChannel channel,
                                       NamedRoleType *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxNamedRoleTypes P_((ComxChannel channel,
                                        NamedRoleTypes *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxNameValuePair P_((ComxChannel channel,
                                       NameValuePair *value))
    OCD ("CosLifeC") ;

extern  errno_t  gimxNodeHandle P_((ComxChannel channel,
                                    NodeHandle *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxNodeHandles P_((ComxChannel channel,
                                     NodeHandles *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxNRPolicyFeatures P_((ComxChannel channel,
                                          NRPolicyFeatures *value))
    OCD ("NRServic") ;

extern  errno_t  gimxNRPolicyFeaturesList P_((ComxChannel channel,
                                              NRPolicyFeaturesList *value))
    OCD ("NRServic") ;

extern  errno_t  gimxNVPair P_((ComxChannel channel,
                                NVPair *value))
    OCD ("CosColle") ;

extern  errno_t  gimxObject P_((ComxChannel channel,
                                Object *value))
    OCD ("IOP") ;

extern  errno_t  gimxOffer P_((ComxChannel channel,
                               Offer *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxOfferIdSeq P_((ComxChannel channel,
                                    OfferIdSeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxOfferInfo P_((ComxChannel channel,
                                   OfferInfo *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxOfferSeq P_((ComxChannel channel,
                                  OfferSeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxOIDList P_((ComxChannel channel,
                                 OIDList *value))
    OCD ("CSI") ;

extern  errno_t  gimxOpDescriptionSeq P_((ComxChannel channel,
                                          OpDescriptionSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxOperationDescription P_((ComxChannel channel,
                                              OperationDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxOptionsDirectionPair P_((ComxChannel channel,
                                              OptionsDirectionPair *value))
    OCD ("Security") ;

extern  errno_t  gimxOptionsDirectionPairList P_((ComxChannel channel,
                                                  OptionsDirectionPairList *value))
    OCD ("Security") ;

extern  errno_t  gimxOtid_t P_((ComxChannel channel,
                                otid_t *value))
    OCD ("CosTrans") ;

extern  errno_t  gimxParameter P_((ComxChannel channel,
                                   Parameter *value))
    OCD ("Dynamic") ;

extern  errno_t  gimxParameterDescription P_((ComxChannel channel,
                                              ParameterDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxParameterList P_((ComxChannel channel,
                                       ParameterList *value))
    OCD ("CosColle") ;

extern  errno_t  gimxParDescriptionSeq P_((ComxChannel channel,
                                           ParDescriptionSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxPolicyList P_((ComxChannel channel,
                                    PolicyList *value))
    OCD ("CORBA") ;

extern  errno_t  gimxPolicyNameSeq P_((ComxChannel channel,
                                       PolicyNameSeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxPolicyTypeSeq P_((ComxChannel channel,
                                       PolicyTypeSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxPolicyValueSeq P_((ComxChannel channel,
                                        PolicyValueSeq *value))
    OCD ("Messagin") ;

extern  errno_t  gimxPriorityRange P_((ComxChannel channel,
                                       PriorityRange *value))
    OCD ("Messagin") ;

extern  errno_t  gimxProfileId P_((ComxChannel channel,
                                   ProfileId *value))
    OCD ("IOP") ;

extern  errno_t  gimxPropagationContext P_((ComxChannel channel,
                                            PropagationContext *value))
    OCD ("CosTrans") ;

extern  errno_t  gimxProperties P_((ComxChannel channel,
                                    Properties *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxProperty P_((ComxChannel channel,
                                  Property *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyDef P_((ComxChannel channel,
                                     PropertyDef *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyDefs P_((ComxChannel channel,
                                      PropertyDefs *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyException P_((ComxChannel channel,
                                           PropertyException *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyExceptions P_((ComxChannel channel,
                                            PropertyExceptions *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyList P_((ComxChannel channel,
                                      PropertyList *value))
    OCD ("LifeCycl") ;

extern  errno_t  gimxPropertyMode P_((ComxChannel channel,
                                      PropertyMode *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyModes P_((ComxChannel channel,
                                       PropertyModes *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyNames P_((ComxChannel channel,
                                       PropertyNames *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropertyNameSeq P_((ComxChannel channel,
                                         PropertyNameSeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxPropertySeq P_((ComxChannel channel,
                                     PropertySeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxPropertyTypes P_((ComxChannel channel,
                                       PropertyTypes *value))
    OCD ("CosPrope") ;

extern  errno_t  gimxPropStruct P_((ComxChannel channel,
                                    PropStruct *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxPropStructSeq P_((ComxChannel channel,
                                       PropStructSeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxProvidesDescription P_((ComxChannel channel,
                                             ProvidesDescription *value))
    OCD ("Componen") ;

extern  errno_t  gimxProvidesDescriptionSeq P_((ComxChannel channel,
                                                ProvidesDescriptionSeq *value))
    OCD ("Componen") ;

extern  errno_t  gimxProxyInfo P_((ComxChannel channel,
                                   ProxyInfo *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxQLTypes P_((ComxChannel channel,
                                 QLTypes *value))
    OCD ("CosQuery") ;

extern  errno_t  gimxRecord P_((ComxChannel channel,
                                Record *value))
    OCD ("CosQuery") ;

extern  errno_t  gimxRelationshipHandle P_((ComxChannel channel,
                                            RelationshipHandle *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxRelationshipHandles P_((ComxChannel channel,
                                             RelationshipHandles *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxReplyHeader P_((ComxChannel channel,
                                     ReplyHeader *value))
    OCD ("GIOP") ;

extern  errno_t  gimxReplyHeader_1_0 P_((ComxChannel channel,
                                         ReplyHeader_1_0 *value))
    OCD ("GIOP") ;

extern  errno_t  gimxRepositoryIdSeq P_((ComxChannel channel,
                                         RepositoryIdSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxRequestFeatures P_((ComxChannel channel,
                                         RequestFeatures *value))
    OCD ("NRServic") ;

extern  errno_t  gimxRequestHeader P_((ComxChannel channel,
                                       RequestHeader *value))
    OCD ("GIOP") ;

extern  errno_t  gimxRequestHeader_1_0 P_((ComxChannel channel,
                                           RequestHeader_1_0 *value))
    OCD ("GIOP") ;

extern  errno_t  gimxRequestHeader_1_1 P_((ComxChannel channel,
                                           RequestHeader_1_1 *value))
    OCD ("GIOP") ;

extern  errno_t  gimxRight P_((ComxChannel channel,
                               Right *value))
    OCD ("Security") ;

extern  errno_t  gimxRightsList P_((ComxChannel channel,
                                    RightsList *value))
    OCD ("Security") ;

extern  errno_t  gimxRoleNames P_((ComxChannel channel,
                                   RoleNames *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxRoles P_((ComxChannel channel,
                               Roles *value))
    OCD ("CosRelat") ;

extern  errno_t  gimxRoutingTypeRange P_((ComxChannel channel,
                                          RoutingTypeRange *value))
    OCD ("Messagin") ;

extern  errno_t  gimxSASContextBody P_((ComxChannel channel,
                                        SASContextBody *value))
    OCD ("CSI") ;

extern  errno_t  gimxSAS_ContextSec P_((ComxChannel channel,
                                        SAS_ContextSec *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxScopedEdge P_((ComxChannel channel,
                                    ScopedEdge *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxScopedEdges P_((ComxChannel channel,
                                     ScopedEdges *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxScopedEndPoint P_((ComxChannel channel,
                                        ScopedEndPoint *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxScopedEndPoints P_((ComxChannel channel,
                                         ScopedEndPoints *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxScopedRelationship P_((ComxChannel channel,
                                            ScopedRelationship *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxSecAttribute P_((ComxChannel channel,
                                      SecAttribute *value))
    OCD ("Security") ;

extern  errno_t  gimxSECIOP_SEC_TRANS P_((ComxChannel channel,
                                          SECIOP_SEC_TRANS *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxSecurityFeatureValue P_((ComxChannel channel,
                                              SecurityFeatureValue *value))
    OCD ("Security") ;

extern  errno_t  gimxSecurityFeatureValueList P_((ComxChannel channel,
                                                  SecurityFeatureValueList *value))
    OCD ("Security") ;

extern  errno_t  gimxSecurityMechandName P_((ComxChannel channel,
                                             SecurityMechandName *value))
    OCD ("Security") ;

extern  errno_t  gimxSecurityMechandNameList P_((ComxChannel channel,
                                                 SecurityMechandNameList *value))
    OCD ("Security") ;

extern  errno_t  gimxSelectorValue P_((ComxChannel channel,
                                       SelectorValue *value))
    OCD ("Security") ;

extern  errno_t  gimxSelectorValueList P_((ComxChannel channel,
                                           SelectorValueList *value))
    OCD ("Security") ;

extern  errno_t  gimxSequencingHeader P_((ComxChannel channel,
                                          SequencingHeader *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxServiceConfiguration P_((ComxChannel channel,
                                              ServiceConfiguration *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxServiceConfigurationList P_((ComxChannel channel,
                                                  ServiceConfigurationList *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxServiceContext P_((ComxChannel channel,
                                        ServiceContext *value))
    OCD ("IOP") ;

extern  errno_t  gimxServiceContextList P_((ComxChannel channel,
                                            ServiceContextList *value))
    OCD ("IOP") ;

extern  errno_t  gimxServiceTypeNameSeq P_((ComxChannel channel,
                                            ServiceTypeNameSeq *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxSpecifiedProps P_((ComxChannel channel,
                                        SpecifiedProps *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxSpecifiedServiceTypes P_((ComxChannel channel,
                                               SpecifiedServiceTypes *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxSPKM_1 P_((ComxChannel channel,
                                SPKM_1 *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxSPKM_2 P_((ComxChannel channel,
                                SPKM_2 *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxSSL P_((ComxChannel channel,
                             SSL *value))
    OCD ("SSLIOP") ;

extern  errno_t  gimxStructMember P_((ComxChannel channel,
                                      StructMember *value))
    OCD ("CORBA") ;

extern  errno_t  gimxStructMemberSeq P_((ComxChannel channel,
                                         StructMemberSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxSystemExceptionReplyBody P_((ComxChannel channel,
                                                  SystemExceptionReplyBody *value))
    OCD ("GIOP") ;

extern  errno_t  gimxTaggedComponent P_((ComxChannel channel,
                                         TaggedComponent *value))
    OCD ("IOP") ;

extern  errno_t  gimxTargetAddress P_((ComxChannel channel,
                                       TargetAddress *value))
    OCD ("GIOP") ;

extern  errno_t  gimxTargetAssociationOptions P_((ComxChannel channel,
                                                  TargetAssociationOptions *value))
    OCD ("SECIOP") ;

extern  errno_t  gimxTimerEventT P_((ComxChannel channel,
                                     TimerEventT *value))
    OCD ("CosTimer") ;

extern  errno_t  gimxTLS_SEC_TRANS P_((ComxChannel channel,
                                       TLS_SEC_TRANS *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxTransIdentity P_((ComxChannel channel,
                                       TransIdentity *value))
    OCD ("CosTrans") ;

extern  errno_t  gimxTransportAddress P_((ComxChannel channel,
                                          TransportAddress *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxTransportAddressList P_((ComxChannel channel,
                                              TransportAddressList *value))
    OCD ("CSIIOP") ;

extern  errno_t  gimxTypeDescription P_((ComxChannel channel,
                                         TypeDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxTypeStruct P_((ComxChannel channel,
                                    TypeStruct *value))
    OCD ("CosTradi") ;

extern  errno_t  gimxUnionMember P_((ComxChannel channel,
                                     UnionMember *value))
    OCD ("CORBA") ;

extern  errno_t  gimxUnionMemberSeq P_((ComxChannel channel,
                                        UnionMemberSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxURLSeq P_((ComxChannel channel,
                                URLSeq *value))
    OCD ("SendingC") ;

extern  errno_t  gimxUsesDescription P_((ComxChannel channel,
                                         UsesDescription *value))
    OCD ("Componen") ;

extern  errno_t  gimxUsesDescriptionSeq P_((ComxChannel channel,
                                            UsesDescriptionSeq *value))
    OCD ("Componen") ;

extern  errno_t  gimxUtcT P_((ComxChannel channel,
                              UtcT *value))
    OCD ("TimeBase") ;

extern  errno_t  gimxValue P_((ComxChannel channel,
                               Value *value))
    OCD ("CosQuery") ;

extern  errno_t  gimxValueDefSeq P_((ComxChannel channel,
                                     ValueDefSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxValueDescription P_((ComxChannel channel,
                                          ValueDescription *value))
    OCD ("CORBA") ;

extern  errno_t  gimxValueDescSeq P_((ComxChannel channel,
                                      ValueDescSeq *value))
    OCD ("SendingC") ;

extern  errno_t  gimxValueMember P_((ComxChannel channel,
                                     ValueMember *value))
    OCD ("CORBA") ;

extern  errno_t  gimxValueMemberSeq P_((ComxChannel channel,
                                        ValueMemberSeq *value))
    OCD ("CORBA") ;

extern  errno_t  gimxWeightedEdge P_((ComxChannel channel,
                                      WeightedEdge *value))
    OCD ("CosGraph") ;

extern  errno_t  gimxWeightedEdges P_((ComxChannel channel,
                                       WeightedEdges *value))
    OCD ("CosGraph") ;
