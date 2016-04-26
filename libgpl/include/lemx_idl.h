/* $Id: lemx_idl.h,v 1.8 2006/04/28 19:58:28 alex Exp $ */
/*******************************************************************************
    ../idl/LECIS/LECIS.idl
*******************************************************************************/

/* Module: SCD */

typedef  IOR  ICommand ;
typedef  IOR  IEvent ;
typedef  IOR  IPort ;
typedef  IOR  IResource ;
typedef  IOR  IExtMacroCommandList ;
typedef  IOR  IWorkCell ;

typedef  struct  SAdministrative {
    char  *model_number ;
    char  *serial_number ;
    char  *software_version ;
    char  *support_address ;
    char  *manufacturer_id ;
    char  *manufacturer_name ;
    char  *description ;
    char  *update_address ;
    char  *dcd_version ;
    char  *protocol ;
}  SAdministrative ;

typedef  StringSeq  SeqString ;

typedef  enum  ECommandCategory {
    _INIT,
    CONTROL,
    FUNCTION,
    CONFIGURE,
    RECOVERY,
    STATUSREQ,
    MAINTAIN,
    CALIBRATE,
    ADMIN,
    RESULT
}  ECommandCategory ;

typedef  enum  EVariableType {
    LONG_TYPE,
    FLOAT_TYPE,
    BOOLEAN_TYPE,
    STRING_TYPE,
    OCTET_TYPE,
    SEQ_LONG_TYPE,
    SEQ_FLOAT_TYPE,
    SEQ_OCTET_TYPE
}  EVariableType ;

typedef  enum  ETransferType {
    INTRANSFER,
    OUTTRANSFER,
    INOUTTRANSFER
}  ETransferType ;

typedef  struct  SItemData {
    char  *item ;
    char  *value ;
}  SItemData ;

typedef  enum  ENumberType {
    LONG_NTYPE,
    FLOAT_NTYPE
}  ENumberType ;

typedef  struct  SRange {
    ENumberType  range_type ;
    char  *low_limit ;
    char  *high_limit ;
}  SRange ;

typedef  struct  SArgument {
    char  *name ;
    EVariableType  argument_type ;
    Any  default_value ;
    ETransferType  transfer_type ;
    char  *description ;
    SEQUENCE (SItemData, properties) ;
    SRange  range ;
}  SArgument ;

typedef  enum  EAccessType {
    INLET,
    OUTLET,
    INOUTLET,
    TRANSFER
}  EAccessType ;

typedef  enum  EOwnerstatus {
    PRIVATE_OWNER,
    LOCKED,
    UNLOCKED
}  EOwnerstatus ;

typedef  enum  EComponentCategory {
    SYSTEM,
    WORKCELL,
    SLM,
    RESOURCE,
    SUBUNIT
}  EComponentCategory ;

typedef  struct  SComponentID {
    char  *workcell_id ;
    char  *slm_id ;
    EComponentCategory  component_category ;
    char  *subunit_id ;
    char  *resource_id ;
}  SComponentID ;

typedef  struct  SOwnership {
    EOwnerstatus  owner_status ;
    SComponentID  owner ;
}  SOwnership ;

typedef  struct  SValue {
    char  *value ;
    ENumberType  type ;
    long  exponent ;
    char  *unit ;
}  SValue ;

typedef  enum  EResourceCategory {
    HARDWARE,
    SAMPLE,
    REAGENT,
    WASTE,
    SPACE,
    BUFFER,
    UNDEFINED
}  EResourceCategory ;

typedef  enum  ECapacityType {
    FINITE,
    ECT_INFINITE
}  ECapacityType ;

typedef  enum  EDownTimeCategory {
    CLEANING,
    CALIBRATION,
    SOFTWARE_UPDATE,
    HARDWARE_UPDATE
}  EDownTimeCategory ;

typedef  struct  STranslation {
    long  x ;
    long  y ;
    long  z ;
}  STranslation ;

typedef  struct  SRotation {
    long  x ;
    long  y ;
    long  z ;
}  SRotation ;

typedef  enum  ESystemDomain {
    COUNTRY,
    DEPARTMENT,
    SUBDIVISION,
    LABORATORY,
    ROOM
}  ESystemDomain ;

typedef  enum  EEventCategory {
    ALARM,
    MESSAGE,
    DATA_DIRECT,
    DATA_LINK,
    SYSVAR_CHANGED,
    CONTROL_STATE_CHANGED,
    SLM_STATE_CHANGED
}  EEventCategory ;

typedef  struct  SSystemVariable {
    char  *variable_id ;
    char  *description ;
    EVariableType  data_type ;
    Any  current_value ;
    char  *category ;
    SRange  value_range ;
}  SSystemVariable ;

typedef  struct  SCapacity {
    SValue  min_capacity ;
    SValue  max_capacity ;
    SValue  fill_steps ;
}  SCapacity ;

typedef  struct  SLocation {
    SRotation  rotation ;
    STranslation  translation ;
}  SLocation ;

typedef  struct  SGeometricModel {
    Any  model ;
    Any  access_curve ;
}  SGeometricModel ;

typedef  struct  SDimension {
    char  *height ;
    char  *width ;
    SGeometricModel  geometric_model ;
    char  *length ;
}  SDimension ;

typedef  struct  SPhysicalCharacteristics {
    char  *weight ;
    SLocation  location ;
    SDimension  dimension ;
}  SPhysicalCharacteristics ;

typedef  SEQUENCE (Any, SeqAny) ;

typedef  enum  ECommandType {
    ATOMIC,
    MACRO
}  ECommandType ;

typedef  enum  EDownTimeType {
    ESTIMATED,
    ACTUAL
}  EDownTimeType ;

typedef  IOR  IDownTime ;

typedef  enum  EPortType {
    DATA,
    MATERIAL
}  EPortType ;

typedef  IOR  ISubUnit ;

typedef  SEQUENCE (ICommand, commands_def) ;

typedef  SEQUENCE (SAdministrative, administrative_def) ;

typedef  SEQUENCE (IEvent, events_def) ;

typedef  SEQUENCE (IPort, ports_def) ;

typedef  SEQUENCE (IResource, resources_def) ;

typedef  SEQUENCE (SItemData, properties_def) ;

typedef  SEQUENCE (IExtMacroCommandList, subunit_external_macros_def) ;

/*===== "properties_def" already defined =====*/

typedef  SEQUENCE (IPort, access_ports_def) ;

/*===== "properties_def" already defined =====*/

typedef  SEQUENCE (IResource, content_def) ;

typedef  SEQUENCE (ICommand, required_configurations_def) ;

typedef  SEQUENCE (IResource, required_resources_def) ;

typedef  SEQUENCE (IResource, produced_resources_def) ;

typedef  SEQUENCE (IPort, port_inputs_def) ;

typedef  SEQUENCE (IPort, port_outputs_def) ;

typedef  SEQUENCE (SRange, measurement_bounds_def) ;

typedef  SEQUENCE (SArgument, formal_arguments_def) ;

typedef  SEQUENCE (SArgument, sync_response_data_def) ;

/*===== "properties_def" already defined =====*/

typedef  SEQUENCE (SItemData, exclusion_list_def) ;

typedef  IOR  ISLM ;

/*===== "resources_def" already defined =====*/

typedef  SEQUENCE (ISubUnit, sub_units_def) ;

/*===== "properties_def" already defined =====*/

typedef  SEQUENCE (IDownTime, downtime_def) ;

/*===== "ports_def" already defined =====*/

/*===== "events_def" already defined =====*/

typedef  SEQUENCE (SSystemVariable, system_variables_def) ;

/*===== "commands_def" already defined =====*/

/*===== "properties_def" already defined =====*/

typedef  SEQUENCE (SArgument, event_data_types_def) ;

typedef  SEQUENCE (ICommand, possible_event_reaction_def) ;

/*===== "system_variables_def" already defined =====*/

typedef  struct  SExtMacroCommand {
    SeqAny  argument_values ;
    ICommand  command_ref ;
}  SExtMacroCommand ;

/*===== "commands_def" already defined =====*/

/*===== "properties_def" already defined =====*/

typedef  SEQUENCE (IExtMacroCommandList, ext_macros_def) ;

typedef  SEQUENCE (ISLM, slms_def) ;

/*===== "resources_def" already defined =====*/

typedef  IOR  ISystem ;

typedef  SEQUENCE (IWorkCell, workcells_def) ;

/*===== "resources_def" already defined =====*/

typedef  IOR  ISCDRegistry ;

/* Module: SLM_INTERFACE */

typedef  enum  EMainCtrlState {
    POWERED_UP,
    INITIALIZING,
    NORMAL_OP,
    EMCS_ERROR,
    ESTOPPED,
    CLEARING,
    CLEARED,
    SHUTDOWN,
    DOWN
}  EMainCtrlState ;

typedef  enum  ESubCtrlState {
    SUB_POWERED_UP,
    SUB_INITIALIZING,
    SUB_SHUTDOWN,
    SUB_DOWN,
    SUB_ERROR,
    SUB_CLEARING,
    SUB_CLEARED,
    SUB_ABORTED,
    SUB_ESTOPPED,
    SUB_IDLE,
    SUB_PROCESSING,
    SUB_PAUSING,
    SUB_PAUSED,
    SUB_RESUMING
}  ESubCtrlState ;

typedef  enum  EResultCode {
    ERC_SUCCESS,
    REMOTE_CTRL_REQ_DENIED,
    LOCAL_CTRL_REQ_DENIED,
    FORCE_LOCAL_CTRL_FAILED,
    RELEASE_REMOTE_CTRL_FAILED,
    READ_DCD_FAILED,
    WRITE_DCD_FAILED,
    DCD_NOT_AVAILABLE,
    SUBUNIT_UNKNOWN,
    DEVICE_HARDWARE_ERROR,
    COMMUNICATION_ERROR,
    TIMEOUT,
    UNSPECIFIED_ERROR,
    SUB_STATE_INCORRECT,
    MAIN_STATE_INCORRECT,
    PAUSE_REQUEST_DENIED,
    TIME_SYNCHRONIZATION_FAILED,
    UNKNOWN_COMMAND,
    TIME_SYNCHRONIZATION_NOT_AVAILABLE,
    WRONG_ARGUMENT_LIST,
    DATA_ID_UNKNOWN,
    INVALID_DATA,
    ACCESS_DENIED,
    EXECUTING_MACRO,
    EXECUTION_STOPPED
}  EResultCode ;

typedef  OctetSeq  SeqOctet ;

typedef  enum  ELocalRemote {
    _LOCAL,
    REMOTE,
    AVAILABLE
}  ELocalRemote ;

/*===== "SeqString" already defined =====*/

typedef  struct  SSysVar {
    char  *variable_id ;
    char  *description ;
    char  *category ;
    Any  value ;
}  SSysVar ;

typedef  struct  SSubState {
    char  *sub_unit_id ;
    ESubCtrlState  sub_unit_state ;
}  SSubState ;

typedef  SEQUENCE (SSubState, SeqSubStates) ;

typedef  struct  SLM_RESULT {
    EResultCode  result_code ;
    char  *minor_code ;
    EMainCtrlState  main_state ;
    SeqSubStates  sub_states ;
    ELocalRemote  lr_mode ;
    char  *message ;
}  SLM_RESULT ;

typedef  SEQUENCE (SSysVar, SeqSysVar) ;

/*===== "SeqAny" already defined =====*/

typedef  enum  ELocalRemote_ArgType {
    LOCAL_CTRL_REQ,
    REMOTE_CTRL_REQ,
    FORCE_LOCAL_CTRL,
    RELEASE_CTRL
}  ELocalRemote_ArgType ;

typedef  enum  EEventType {
    EET_ALARM,
    EET_MESSAGE,
    EET_DATA_DIRECT,
    EET_DATA_LINK,
    EET_SYSVAR_CHANGED,
    CONTROL_STATE_CHANGE,
    DEVICE_STATE_CHANGED
}  EEventType ;

typedef  enum  EDataLinkType {
    EDLT_FILE,
    DB,
    OPERATION
}  EDataLinkType ;

typedef  IOR  ITSC_Callback ;
typedef  IOR  ILECI ;

/*******************************************************************************
    Tables for mapping enumerated values to names and vice-versa;
    see the coliToName() and coliToNumber() functions.
*******************************************************************************/

extern  const  ColiMap  EAccessTypeLUT[]  OCD ("SCD") ;
extern  const  ColiMap  ECapacityTypeLUT[]  OCD ("SCD") ;
extern  const  ColiMap  ECommandCategoryLUT[]  OCD ("SCD") ;
extern  const  ColiMap  ECommandTypeLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EComponentCategoryLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EDataLinkTypeLUT[]  OCD ("SLM_INTE") ;
extern  const  ColiMap  EDownTimeCategoryLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EDownTimeTypeLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EEventCategoryLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EEventTypeLUT[]  OCD ("SLM_INTE") ;
extern  const  ColiMap  ELocalRemoteLUT[]  OCD ("SLM_INTE") ;
extern  const  ColiMap  ELocalRemote_ArgTypeLUT[]  OCD ("SLM_INTE") ;
extern  const  ColiMap  EMainCtrlStateLUT[]  OCD ("SLM_INTE") ;
extern  const  ColiMap  ENumberTypeLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EOwnerstatusLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EPortTypeLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EResourceCategoryLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EResultCodeLUT[]  OCD ("SLM_INTE") ;
extern  const  ColiMap  ESubCtrlStateLUT[]  OCD ("SLM_INTE") ;
extern  const  ColiMap  ESystemDomainLUT[]  OCD ("SCD") ;
extern  const  ColiMap  ETransferTypeLUT[]  OCD ("SCD") ;
extern  const  ColiMap  EVariableTypeLUT[]  OCD ("SCD") ;

/*******************************************************************************
    Public functions.
*******************************************************************************/

/* Marshaling functions for the defined data types. */

extern  errno_t  lemxAccess_ports_def P_((ComxChannel channel,
                                          access_ports_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxAdministrative_def P_((ComxChannel channel,
                                            administrative_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxCommands_def P_((ComxChannel channel,
                                      commands_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxContent_def P_((ComxChannel channel,
                                     content_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxDowntime_def P_((ComxChannel channel,
                                      downtime_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxEvents_def P_((ComxChannel channel,
                                    events_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxEvent_data_types_def P_((ComxChannel channel,
                                              event_data_types_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxExclusion_list_def P_((ComxChannel channel,
                                            exclusion_list_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxExt_macros_def P_((ComxChannel channel,
                                        ext_macros_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxFormal_arguments_def P_((ComxChannel channel,
                                              formal_arguments_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxMeasurement_bounds_def P_((ComxChannel channel,
                                                measurement_bounds_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxPorts_def P_((ComxChannel channel,
                                   ports_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxPort_inputs_def P_((ComxChannel channel,
                                         port_inputs_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxPort_outputs_def P_((ComxChannel channel,
                                          port_outputs_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxPossible_event_reaction_def P_((ComxChannel channel,
                                                     possible_event_reaction_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxProduced_resources_def P_((ComxChannel channel,
                                                produced_resources_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxProperties_def P_((ComxChannel channel,
                                        properties_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxRequired_configurations_def P_((ComxChannel channel,
                                                     required_configurations_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxRequired_resources_def P_((ComxChannel channel,
                                                required_resources_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxResources_def P_((ComxChannel channel,
                                       resources_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxSAdministrative P_((ComxChannel channel,
                                         SAdministrative *value))
    OCD ("SCD") ;

extern  errno_t  lemxSArgument P_((ComxChannel channel,
                                   SArgument *value))
    OCD ("SCD") ;

extern  errno_t  lemxSCapacity P_((ComxChannel channel,
                                   SCapacity *value))
    OCD ("SCD") ;

extern  errno_t  lemxSComponentID P_((ComxChannel channel,
                                      SComponentID *value))
    OCD ("SCD") ;

extern  errno_t  lemxSDimension P_((ComxChannel channel,
                                    SDimension *value))
    OCD ("SCD") ;

extern  errno_t  lemxSeqAny P_((ComxChannel channel,
                                SeqAny *value))
    OCD ("SCD") ;

extern  errno_t  lemxSeqSubStates P_((ComxChannel channel,
                                      SeqSubStates *value))
    OCD ("SLM_INTE") ;

extern  errno_t  lemxSeqSysVar P_((ComxChannel channel,
                                   SeqSysVar *value))
    OCD ("SLM_INTE") ;

extern  errno_t  lemxSExtMacroCommand P_((ComxChannel channel,
                                          SExtMacroCommand *value))
    OCD ("SCD") ;

extern  errno_t  lemxSGeometricModel P_((ComxChannel channel,
                                         SGeometricModel *value))
    OCD ("SCD") ;

extern  errno_t  lemxSItemData P_((ComxChannel channel,
                                   SItemData *value))
    OCD ("SCD") ;

extern  errno_t  lemxSlms_def P_((ComxChannel channel,
                                  slms_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxSLM_RESULT P_((ComxChannel channel,
                                    SLM_RESULT *value))
    OCD ("SLM_INTE") ;

extern  errno_t  lemxSLocation P_((ComxChannel channel,
                                   SLocation *value))
    OCD ("SCD") ;

extern  errno_t  lemxSOwnership P_((ComxChannel channel,
                                    SOwnership *value))
    OCD ("SCD") ;

extern  errno_t  lemxSPhysicalCharacteristics P_((ComxChannel channel,
                                                  SPhysicalCharacteristics *value))
    OCD ("SCD") ;

extern  errno_t  lemxSRange P_((ComxChannel channel,
                                SRange *value))
    OCD ("SCD") ;

extern  errno_t  lemxSRotation P_((ComxChannel channel,
                                   SRotation *value))
    OCD ("SCD") ;

extern  errno_t  lemxSSubState P_((ComxChannel channel,
                                   SSubState *value))
    OCD ("SLM_INTE") ;

extern  errno_t  lemxSSystemVariable P_((ComxChannel channel,
                                         SSystemVariable *value))
    OCD ("SCD") ;

extern  errno_t  lemxSSysVar P_((ComxChannel channel,
                                 SSysVar *value))
    OCD ("SLM_INTE") ;

extern  errno_t  lemxSTranslation P_((ComxChannel channel,
                                      STranslation *value))
    OCD ("SCD") ;

extern  errno_t  lemxSubunit_external_macros_def P_((ComxChannel channel,
                                                     subunit_external_macros_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxSub_units_def P_((ComxChannel channel,
                                       sub_units_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxSValue P_((ComxChannel channel,
                                SValue *value))
    OCD ("SCD") ;

extern  errno_t  lemxSync_response_data_def P_((ComxChannel channel,
                                                sync_response_data_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxSystem_variables_def P_((ComxChannel channel,
                                              system_variables_def *value))
    OCD ("SCD") ;

extern  errno_t  lemxWorkcells_def P_((ComxChannel channel,
                                       workcells_def *value))
    OCD ("SCD") ;
