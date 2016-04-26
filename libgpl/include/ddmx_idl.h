/* $Id: ddmx_idl.h,v 1.5 2009/09/09 23:23:11 alex Exp $ */
#ifndef KLUDGE_FOR_VALUETYPES
    typedef  IOR  ObjectRoot ;
    typedef  IOR  ObjectHome ;
    typedef  IOR  Selection ;
    typedef  IOR  CacheAccess ;
#endif
/*******************************************************************************
    ../idl/DDS/dds_dlrl.idl
*******************************************************************************/

/* Module: DCPS */

typedef  long  DomainId_t ;

typedef  long  InstanceHandle_t ;

typedef  long  ReturnCode_t ;

typedef  long  QosPolicyId_t ;

/*===== "StringSeq" already defined =====*/

typedef  struct  Duration_t {
    long  sec ;
    unsigned  long  nanosec ;
}  Duration_t ;

typedef  struct  Time_t {
    long  sec ;
    unsigned  long  nanosec ;
}  Time_t ;

#define  HANDLE_NIL  (0)
#define  RETCODE_OK  (0)
#define  RETCODE_ERROR  (1)
#define  RETCODE_UNSUPPORTED  (2)
#define  RETCODE_BAD_PARAMETER  (3)
#define  RETCODE_PRECONDITION_NOT_MET  (4)
#define  RETCODE_OUT_OF_RESOURCES  (5)
#define  RETCODE_NOT_ENABLED  (6)
#define  RETCODE_IMMUTABLE_POLICY  (7)
#define  RETCODE_INCONSISTENT_POLICY  (8)

typedef  unsigned  long  StatusKind ;

typedef  unsigned  long  StatusKindMask ;

#define  INCONSISTENT_TOPIC_STATUS  (0x0001 < < 0)
#define  OFFERED_DEADLINE_MISSED_STATUS  (0x0001 < < 1)
#define  REQUESTED_DEADLINE_MISSED_STATUS  (0x0001 < < 2)
#define  OFFERED_INSTANCE_DEADLINE_MISSED_STATUS  (0x0001 < < 3)
#define  REQUESTED_INSTANCE_DEADLINE_MISSED_STATUS  (0x0001 < < 4)
#define  OFFERED_INCOMPATIBLE_QOS_STATUS  (0x0001 < < 5)
#define  REQUESTED_INCOMPATIBLE_QOS_STATUS  (0x0001 < < 6)
#define  SAMPLE_LOST_STATUS  (0x0001 < < 7)
#define  SAMPLE_REJECTED_STATUS  (0x0001 < < 8)
#define  DATA_ON_READERS_STATUS  (0x0001 < < 9)
#define  DATA_AVAILABLE_STATUS  (0x0001 < < 10)

typedef  struct  InconsistentTopicStatus {
    long  total_count ;
    long  total_count_change ;
}  InconsistentTopicStatus ;

typedef  struct  SampleLostStatus {
    long  total_count ;
    long  total_count_change ;
}  SampleLostStatus ;

typedef  enum  SampleRejectedStatusKind {
    REJECTED_BY_INSTANCE_LIMIT,
    REJECTED_BY_TOPIC_LIMIT
}  SampleRejectedStatusKind ;

typedef  struct  SampleRejectedStatus {
    long  total_count ;
    long  total_count_change ;
    SampleRejectedStatusKind  last_reason ;
    InstanceHandle_t  last_instance_handle ;
}  SampleRejectedStatus ;

typedef  struct  LivelinessLostStatus {
    long  total_count ;
    long  total_count_chnage ;
}  LivelinessLostStatus ;

typedef  struct  LivelinessChangedStatus {
    long  active_count ;
    long  inactive_count ;
    long  active_count_change ;
    long  inactive_count_change ;
}  LivelinessChangedStatus ;

typedef  struct  OfferedDeadlineMissedStatus {
    long  total_count ;
    long  total_count_change ;
    InstanceHandle_t  last_instance_handle ;
}  OfferedDeadlineMissedStatus ;

typedef  struct  RequestedDeadlineMissedSt {
    long  total_count ;
    long  total_count_change ;
    InstanceHandle_t  last_instance_handle ;
}  RequestedDeadlineMissedSt ;

typedef  struct  QosPolicyCount {
    QosPolicyId_t  policy_id ;
    long  count ;
}  QosPolicyCount ;

typedef  struct  OfferedIncompatibleQosSt {
    long  total_count ;
    long  total_count_change ;
    QosPolicyId_t  last_policy_id ;
    SEQUENCE (QosPolicyCount, policies) ;
}  OfferedIncompatibleQosSt ;

typedef  struct  RequestedIncompatibleQosSt {
    long  total_count ;
    long  total_count_change ;
    QosPolicyId_t  last_policy_id ;
    SEQUENCE (QosPolicyCount, policies) ;
}  RequestedIncompatibleQosSt ;

typedef  IOR  Listener ;
typedef  IOR  Entity ;
typedef  IOR  Topic ;
typedef  IOR  ContentFilteredTopic ;
typedef  IOR  MultiTopic ;
typedef  IOR  DataWriter ;
typedef  IOR  DataReader ;
typedef  IOR  Subscriber ;
typedef  IOR  Publisher ;

typedef  SEQUENCE (Topic, TopicSeq) ;

typedef  SEQUENCE (DataReader, DataReaderSeq) ;

typedef  IOR  TopicListener ;
typedef  IOR  DataWriterListener ;
typedef  IOR  PublisherListener ;
typedef  IOR  DataReaderListener ;
typedef  IOR  SubscriberListener ;
typedef  IOR  DomainParticipantListener ;
typedef  IOR  Condition ;

typedef  SEQUENCE (Condition, ConditionSeq) ;

typedef  IOR  WaitSet ;
typedef  IOR  GuardCondition ;
typedef  IOR  StatusCondition ;

typedef  unsigned  long  SampleStateKind ;

typedef  SEQUENCE (SampleStateKind, SampleStateSeq) ;

#define  READ_SAMPLE_STATE  (0x0001 < < 0)
#define  NOT_READ_SAMPLE_STATE  (0x0001 < < 1)

typedef  unsigned  long  SampleStateMask ;

#define  ANY_SAMPLE_STATE  (0xffff)

typedef  unsigned  long  LifecycleStateKind ;

typedef  SEQUENCE (LifecycleStateKind, LifecycleStateSeq) ;

#define  NEW_LIFECYCLE_STATE  (0x0001 < < 0)
#define  MODIFIED_LIFECYCLE_STATE  (0x0001 < < 1)
#define  DISPOSED_LIFECYCLE_STATE  (0x0001 < < 2)
#define  NO_WRITERS_LIFECYCLE_STATE  (0x0001 < < 3)

typedef  unsigned  long  LifecycleStateMask ;

#define  ANY_LIFECYCLE_STATE  (0xffff)

typedef  IOR  ReadCondition ;
typedef  IOR  QueryCondition ;

#define  USERDATA_QOS_POLICY_NAME  ("UserData")
#define  DURABILITY_QOS_POLICY_NAME  ("Durability")
#define  PRESENTATION_QOS_POLICY_NAME  ("Presentation")
#define  DEADLINE_QOS_POLICY_NAME  ("Deadline")
#define  LATENCYBUDGET_QOS_POLICY_NAME  ("LatencyBudget")
#define  OWNERSHIP_QOS_POLICY_NAME  ("Ownership")
#define  OWNERSHIPSTRENGTH_QOS_POLICY_NAME  ("OwnershipStrength")
#define  LIVELINESS_QOS_POLICY_NAME  ("Liveliness")
#define  TIMEBASEDFILTER_QOS_POLICY_NAME  ("TimeBasedFilter")
#define  PARTITION_QOS_POLICY_NAME  ("Partition")
#define  RELIABILITY_QOS_POLICY_NAME  ("Reliability")
#define  DESTINATIONORDER_QOS_POLICY_NAME  ("DestinationOrder")
#define  HISTORY_QOS_POLICY_NAME  ("History")
#define  RESOURCELIMITS_QOS_POLICY_NAME  ("ResourceLimits")
#define  USERDATA_QOS_POLICY_ID  (1)
#define  DURABILITY_QOS_POLICY_ID  (2)
#define  PRESENTATION_QOS_POLICY_ID  (3)
#define  DEADLINE_QOS_POLICY_ID  (4)
#define  LATENCYBUDGET_QOS_POLICY_ID  (5)
#define  OWNERSHIP_QOS_POLICY_ID  (6)
#define  OWNERSHIPSTRENGTH_QOS_POLICY_ID  (7)
#define  LIVELINESS_QOS_POLICY_ID  (8)
#define  TIMEBASEDFILTER_QOS_POLICY_ID  (9)
#define  PARTITION_QOS_POLICY_ID  (10)
#define  RELIABILITY_QOS_POLICY_ID  (11)
#define  DESTINATIONORDER_QOS_POLICY_ID  (12)
#define  HISTORY_QOS_POLICY_ID  (13)
#define  RESOURCELIMITS_QOS_POLICY_ID  (14)

typedef  struct  UserDataQosPolicy {
    OctetSeq  data ;
}  UserDataQosPolicy ;

typedef  enum  DurabilityQosPolicyKind {
    VOLATILE_DURABILITY_QOS,
    TRANSIENT_DURABILITY_QOS,
    PERSISTENT_DURABILITY_QOS
}  DurabilityQosPolicyKind ;

typedef  struct  DurabilityQosPolicy {
    DurabilityQosPolicyKind  kind ;
}  DurabilityQosPolicy ;

typedef  enum  PresentationQosPolicyKind {
    INSTANCE_PRESENTATION_QOS,
    TOPIC_PRESENTATION_QOS,
    GROUP_PRESENTATION_QOS
}  PresentationQosPolicyKind ;

typedef  struct  PresentationQosPolicy {
    PresentationQosPolicyKind  access_scope ;
    bool  coherent_access ;
    bool  ordered_access ;
}  PresentationQosPolicy ;

typedef  struct  DeadlineQosPolicy {
    Duration_t  period ;
}  DeadlineQosPolicy ;

typedef  struct  LatencyBudgetQosPolicy {
    Duration_t  duration ;
}  LatencyBudgetQosPolicy ;

typedef  enum  OwnershipQosPolicyKind {
    SHARED_OWNERSHIP_QOS,
    EXCLUSIVE_OWNERSHIP_QOS
}  OwnershipQosPolicyKind ;

typedef  struct  OwnershipQosPolicy {
    OwnershipQosPolicyKind  kind ;
}  OwnershipQosPolicy ;

typedef  struct  OwnershipStrengthQosPolicy {
    long  value ;
}  OwnershipStrengthQosPolicy ;

typedef  enum  LivelinessQosPolicyKind {
    AUTOMATIC_LIVELINESS_QOS,
    MANUAL_BY_PARTICIPANT_LIVELINESS_QOS,
    MANUAL_BY_TOPIC_LIVELINESS_QOS
}  LivelinessQosPolicyKind ;

typedef  struct  LivelinessQosPolicy {
    LivelinessQosPolicyKind  kind ;
    Duration_t  lease_duration ;
}  LivelinessQosPolicy ;

typedef  struct  TimeBasedFilterQosPolicy {
    Duration_t  minimum_separation ;
}  TimeBasedFilterQosPolicy ;

typedef  struct  PartitionQosPolicy {
    StringSeq  name ;
}  PartitionQosPolicy ;

typedef  enum  ReliabilityQosPolicyKind {
    BEST_EFFORT_RELIABILITY_QOS,
    RELIABLE_RELIABILITY_QOS
}  ReliabilityQosPolicyKind ;

typedef  struct  ReliabilityQosPolicy {
    ReliabilityQosPolicyKind  kind ;
}  ReliabilityQosPolicy ;

typedef  enum  DestinationOrderQosPolKind {
    BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS,
    BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS
}  DestinationOrderQosPolKind ;

typedef  struct  DestinationOrderQosPolicy {
    DestinationOrderQosPolKind  kind ;
}  DestinationOrderQosPolicy ;

typedef  enum  HistoryQosPolicyKind {
    KEEP_LAST_HISTORY_QOS,
    KEEP_ALL_HISTORY_QOS
}  HistoryQosPolicyKind ;

typedef  struct  HistoryQosPolicy {
    HistoryQosPolicyKind  kind ;
    long  depth ;
}  HistoryQosPolicy ;

typedef  struct  ResourceLimitsQosPolicy {
    long  max_samples ;
    long  max_instances ;
    long  max_samples_per_instance ;
}  ResourceLimitsQosPolicy ;

typedef  struct  DomainParticipantQos {
    UserDataQosPolicy  user_data ;
}  DomainParticipantQos ;

typedef  struct  TopicQos {
    DurabilityQosPolicy  durability ;
    DeadlineQosPolicy  deadline ;
    LatencyBudgetQosPolicy  delay_laxity ;
    LivelinessQosPolicy  liveliness ;
    ReliabilityQosPolicy  reliability ;
    DestinationOrderQosPolicy  destination_order ;
    HistoryQosPolicy  history ;
    ResourceLimitsQosPolicy  resource_limits ;
    OwnershipQosPolicy  ownership ;
}  TopicQos ;

typedef  struct  DataWriterQos {
    DurabilityQosPolicy  durability ;
    DeadlineQosPolicy  deadline ;
    LatencyBudgetQosPolicy  delay_laxity ;
    LivelinessQosPolicy  liveliness ;
    ReliabilityQosPolicy  reliability ;
    DestinationOrderQosPolicy  destination_order ;
    HistoryQosPolicy  history ;
    ResourceLimitsQosPolicy  resource_limits ;
    UserDataQosPolicy  user_data ;
    OwnershipStrengthQosPolicy  ownership_strength ;
}  DataWriterQos ;

typedef  struct  PublisherQos {
    UserDataQosPolicy  user_data ;
    PresentationQosPolicy  presentation ;
    PartitionQosPolicy  partition ;
}  PublisherQos ;

typedef  struct  DataReaderQos {
    DurabilityQosPolicy  durability ;
    DeadlineQosPolicy  deadline ;
    LatencyBudgetQosPolicy  delay_laxity ;
    LivelinessQosPolicy  liveliness ;
    ReliabilityQosPolicy  reliability ;
    DestinationOrderQosPolicy  destination_order ;
    HistoryQosPolicy  history ;
    ResourceLimitsQosPolicy  resource_limits ;
    UserDataQosPolicy  user_data ;
    TimeBasedFilterQosPolicy  time_based_filter ;
}  DataReaderQos ;

typedef  struct  SubscriberQos {
    UserDataQosPolicy  user_data ;
    PresentationQosPolicy  presentation ;
    PartitionQosPolicy  partition ;
}  SubscriberQos ;

typedef  IOR  DomainParticipant ;
typedef  IOR  DomainParticipantFactory ;
typedef  IOR  DataType ;
typedef  IOR  TopicDescription ;

typedef  struct  SampleInfo {
    SampleStateKind  sample_state ;
    LifecycleStateKind  lifecycle_state ;
    Time_t  source_timestamp ;
    InstanceHandle_t  instance_handle ;
}  SampleInfo ;

typedef  SEQUENCE (SampleInfo, SampleInfoSeq) ;

/* Module: DLRL */

typedef  enum  ReferenceScope {
    SIMPLE_CONTENT_SCOPE,
    REFERENCED_CONTENTS_SCOPE
}  ReferenceScope ;

typedef  enum  ObjectScope {
    SIMPLE_OBJECT_SCOPE,
    CONTAINED_OBJECTS_SCOPE,
    RELATED_OBJECTS_SCOPE
}  ObjectScope ;

typedef  enum  DCPSState {
    DCPS_INITIAL,
    DCPS_REGISTERED,
    DCPS_ENABLED
}  DCPSState ;

typedef  enum  CacheUsage {
    READ_ONLY,
    WRITE_ONLY,
    READ_WRITE
}  CacheUsage ;

typedef  unsigned  short  ObjectState ;

#define  R_NEW  (0x0001 < < 0)
#define  R_MODIFIED  (0x0001 < < 1)
#define  R_READ  (0x0001 < < 2)
#define  R_DELETED  (0x0001 < < 3)
#define  W_CREATED  (0x0001 < < 8)
#define  W_CHANGED  (0x0001 < < 9)
#define  W_WRITTEN  (0x0001 < < 10)
#define  W_DESTROYED  (0x0001 < < 11)
#define  W_DELETED  (0x0001 < < 12)

typedef  unsigned  long  DLRLOid ;

typedef  StringSeq  stringSeq ;

typedef  LongSeq  longSeq ;

typedef  SEQUENCE (ObjectRoot, ObjectRootSeq) ;

typedef  SEQUENCE (ObjectHome, ObjectHomeSeq) ;

typedef  SEQUENCE (Selection, SelectionSeq) ;

typedef  SEQUENCE (CacheAccess, CacheAccessSeq) ;

typedef  struct  ObjectLink {
    DLRLOid  oid ;
    unsigned  long  home_index ;
}  ObjectLink ;

typedef  SEQUENCE (ObjectLink, ObjectLinkSeq) ;

/*******************************************************************************
    Tables for mapping enumerated values to names and vice-versa;
    see the coliToName() and coliToNumber() functions.
*******************************************************************************/

extern  const  ColiMap  CacheUsageLUT[]  OCD ("DLRL") ;
extern  const  ColiMap  DCPSStateLUT[]  OCD ("DLRL") ;
extern  const  ColiMap  DestinationOrderQosPolKindLUT[]  OCD ("DCPS") ;
extern  const  ColiMap  DurabilityQosPolicyKindLUT[]  OCD ("DCPS") ;
extern  const  ColiMap  HistoryQosPolicyKindLUT[]  OCD ("DCPS") ;
extern  const  ColiMap  LivelinessQosPolicyKindLUT[]  OCD ("DCPS") ;
extern  const  ColiMap  ObjectScopeLUT[]  OCD ("DLRL") ;
extern  const  ColiMap  OwnershipQosPolicyKindLUT[]  OCD ("DCPS") ;
extern  const  ColiMap  PresentationQosPolicyKindLUT[]  OCD ("DCPS") ;
extern  const  ColiMap  ReferenceScopeLUT[]  OCD ("DLRL") ;
extern  const  ColiMap  ReliabilityQosPolicyKindLUT[]  OCD ("DCPS") ;
extern  const  ColiMap  SampleRejectedStatusKindLUT[]  OCD ("DCPS") ;

/*******************************************************************************
    Public functions.
*******************************************************************************/

/* Marshaling functions for the defined data types. */

extern  errno_t  ddmxCacheAccessSeq P_((ComxChannel channel,
                                        CacheAccessSeq *value))
    OCD ("DLRL") ;

extern  errno_t  ddmxConditionSeq P_((ComxChannel channel,
                                      ConditionSeq *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDataReaderQos P_((ComxChannel channel,
                                       DataReaderQos *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDataReaderSeq P_((ComxChannel channel,
                                       DataReaderSeq *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDataWriterQos P_((ComxChannel channel,
                                       DataWriterQos *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDeadlineQosPolicy P_((ComxChannel channel,
                                           DeadlineQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDestinationOrderQosPolicy P_((ComxChannel channel,
                                                   DestinationOrderQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDomainParticipantQos P_((ComxChannel channel,
                                              DomainParticipantQos *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDurabilityQosPolicy P_((ComxChannel channel,
                                             DurabilityQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxDuration_t P_((ComxChannel channel,
                                    Duration_t *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxHistoryQosPolicy P_((ComxChannel channel,
                                          HistoryQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxInconsistentTopicStatus P_((ComxChannel channel,
                                                 InconsistentTopicStatus *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxLatencyBudgetQosPolicy P_((ComxChannel channel,
                                                LatencyBudgetQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxLifecycleStateSeq P_((ComxChannel channel,
                                           LifecycleStateSeq *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxLivelinessChangedStatus P_((ComxChannel channel,
                                                 LivelinessChangedStatus *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxLivelinessLostStatus P_((ComxChannel channel,
                                              LivelinessLostStatus *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxLivelinessQosPolicy P_((ComxChannel channel,
                                             LivelinessQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxObjectHomeSeq P_((ComxChannel channel,
                                       ObjectHomeSeq *value))
    OCD ("DLRL") ;

extern  errno_t  ddmxObjectLink P_((ComxChannel channel,
                                    ObjectLink *value))
    OCD ("DLRL") ;

extern  errno_t  ddmxObjectLinkSeq P_((ComxChannel channel,
                                       ObjectLinkSeq *value))
    OCD ("DLRL") ;

extern  errno_t  ddmxObjectRootSeq P_((ComxChannel channel,
                                       ObjectRootSeq *value))
    OCD ("DLRL") ;

extern  errno_t  ddmxOfferedDeadlineMissedStatus P_((ComxChannel channel,
                                                     OfferedDeadlineMissedStatus *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxOfferedIncompatibleQosSt P_((ComxChannel channel,
                                                  OfferedIncompatibleQosSt *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxOwnershipQosPolicy P_((ComxChannel channel,
                                            OwnershipQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxOwnershipStrengthQosPolicy P_((ComxChannel channel,
                                                    OwnershipStrengthQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxPartitionQosPolicy P_((ComxChannel channel,
                                            PartitionQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxPresentationQosPolicy P_((ComxChannel channel,
                                               PresentationQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxPublisherQos P_((ComxChannel channel,
                                      PublisherQos *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxQosPolicyCount P_((ComxChannel channel,
                                        QosPolicyCount *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxReliabilityQosPolicy P_((ComxChannel channel,
                                              ReliabilityQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxRequestedDeadlineMissedSt P_((ComxChannel channel,
                                                   RequestedDeadlineMissedSt *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxRequestedIncompatibleQosSt P_((ComxChannel channel,
                                                    RequestedIncompatibleQosSt *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxResourceLimitsQosPolicy P_((ComxChannel channel,
                                                 ResourceLimitsQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxSampleInfo P_((ComxChannel channel,
                                    SampleInfo *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxSampleInfoSeq P_((ComxChannel channel,
                                       SampleInfoSeq *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxSampleLostStatus P_((ComxChannel channel,
                                          SampleLostStatus *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxSampleRejectedStatus P_((ComxChannel channel,
                                              SampleRejectedStatus *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxSampleStateSeq P_((ComxChannel channel,
                                        SampleStateSeq *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxSelectionSeq P_((ComxChannel channel,
                                      SelectionSeq *value))
    OCD ("DLRL") ;

extern  errno_t  ddmxSubscriberQos P_((ComxChannel channel,
                                       SubscriberQos *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxTimeBasedFilterQosPolicy P_((ComxChannel channel,
                                                  TimeBasedFilterQosPolicy *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxTime_t P_((ComxChannel channel,
                                Time_t *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxTopicQos P_((ComxChannel channel,
                                  TopicQos *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxTopicSeq P_((ComxChannel channel,
                                  TopicSeq *value))
    OCD ("DCPS") ;

extern  errno_t  ddmxUserDataQosPolicy P_((ComxChannel channel,
                                           UserDataQosPolicy *value))
    OCD ("DCPS") ;
