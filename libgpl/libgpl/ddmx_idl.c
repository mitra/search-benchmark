/* $Id: ddmx_idl.c,v 1.4 2006/04/28 20:05:23 alex Exp $ */
/*******************************************************************************
    Convenience Macros (from comx_util.c).
*******************************************************************************/

/* CHECK(status) - return ERRNO on non-zero status. */

#ifndef CHECK
#    define  CHECK(status)			\
        if ((status))  return (errno) ;
#endif

/* NULL_OR(pointer,field) - pass NULL if structure pointer is NULL; else
   pass address of field in structure. */

#ifndef NULL_OR
#    define  NULL_OR(pointer, field)		\
        (((pointer) == NULL) ? NULL : &(pointer)->field)
#endif

/* RETURN_IF_NULL(pointer) - return EINVAL if pointer is NULL. */

#ifndef RETURN_IF_NULL
#    define  RETURN_IF_NULL(pointer)		\
        if ((pointer) == NULL) {		\
            SET_ERRNO (EINVAL) ;		\
            return (errno) ;			\
        }
#endif

/*!*****************************************************************************

    ddmxDuration_t() - decode/encode/erase a CORBA Duration_t structure.

*******************************************************************************/


errno_t  ddmxDuration_t (

#    if PROTOTYPES
        ComxChannel  channel,
        Duration_t  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Duration_t  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, sec))) ;
    CHECK (comxULong (channel, NULL_OR (value, nanosec))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxTime_t() - decode/encode/erase a CORBA Time_t structure.

*******************************************************************************/


errno_t  ddmxTime_t (

#    if PROTOTYPES
        ComxChannel  channel,
        Time_t  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Time_t  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, sec))) ;
    CHECK (comxULong (channel, NULL_OR (value, nanosec))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxInconsistentTopicStatus() - decode/encode/erase a CORBA InconsistentTopicStatus structure.

*******************************************************************************/


errno_t  ddmxInconsistentTopicStatus (

#    if PROTOTYPES
        ComxChannel  channel,
        InconsistentTopicStatus  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        InconsistentTopicStatus  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_change))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxSampleLostStatus() - decode/encode/erase a CORBA SampleLostStatus structure.

*******************************************************************************/


errno_t  ddmxSampleLostStatus (

#    if PROTOTYPES
        ComxChannel  channel,
        SampleLostStatus  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SampleLostStatus  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_change))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxSampleRejectedStatus() - decode/encode/erase a CORBA SampleRejectedStatus structure.

*******************************************************************************/


errno_t  ddmxSampleRejectedStatus (

#    if PROTOTYPES
        ComxChannel  channel,
        SampleRejectedStatus  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SampleRejectedStatus  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_change))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->last_reason ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->last_reason = (SampleRejectedStatusKind) enumeration ;
  }
    CHECK (comxLong (channel, NULL_OR (value, last_instance_handle))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxLivelinessLostStatus() - decode/encode/erase a CORBA LivelinessLostStatus structure.

*******************************************************************************/


errno_t  ddmxLivelinessLostStatus (

#    if PROTOTYPES
        ComxChannel  channel,
        LivelinessLostStatus  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LivelinessLostStatus  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_chnage))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxLivelinessChangedStatus() - decode/encode/erase a CORBA LivelinessChangedStatus structure.

*******************************************************************************/


errno_t  ddmxLivelinessChangedStatus (

#    if PROTOTYPES
        ComxChannel  channel,
        LivelinessChangedStatus  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LivelinessChangedStatus  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, active_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, inactive_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, active_count_change))) ;
    CHECK (comxLong (channel, NULL_OR (value, inactive_count_change))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxOfferedDeadlineMissedStatus() - decode/encode/erase a CORBA OfferedDeadlineMissedStatus structure.

*******************************************************************************/


errno_t  ddmxOfferedDeadlineMissedStatus (

#    if PROTOTYPES
        ComxChannel  channel,
        OfferedDeadlineMissedStatus  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        OfferedDeadlineMissedStatus  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_change))) ;
    CHECK (comxLong (channel, NULL_OR (value, last_instance_handle))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxRequestedDeadlineMissedSt() - decode/encode/erase a CORBA RequestedDeadlineMissedSt structure.

*******************************************************************************/


errno_t  ddmxRequestedDeadlineMissedSt (

#    if PROTOTYPES
        ComxChannel  channel,
        RequestedDeadlineMissedSt  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        RequestedDeadlineMissedSt  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_change))) ;
    CHECK (comxLong (channel, NULL_OR (value, last_instance_handle))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxQosPolicyCount() - decode/encode/erase a CORBA QosPolicyCount structure.

*******************************************************************************/


errno_t  ddmxQosPolicyCount (

#    if PROTOTYPES
        ComxChannel  channel,
        QosPolicyCount  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        QosPolicyCount  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, policy_id))) ;
    CHECK (comxLong (channel, NULL_OR (value, count))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxOfferedIncompatibleQosSt() - decode/encode/erase a CORBA OfferedIncompatibleQosSt structure.

*******************************************************************************/


errno_t  ddmxOfferedIncompatibleQosSt (

#    if PROTOTYPES
        ComxChannel  channel,
        OfferedIncompatibleQosSt  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        OfferedIncompatibleQosSt  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_change))) ;
    CHECK (comxLong (channel, NULL_OR (value, last_policy_id))) ;
    CHECK (comxSequence (channel, NULL_OR (value, policies), (ComxFunc) ddmxQosPolicyCount, sizeof (QosPolicyCount))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxRequestedIncompatibleQosSt() - decode/encode/erase a CORBA RequestedIncompatibleQosSt structure.

*******************************************************************************/


errno_t  ddmxRequestedIncompatibleQosSt (

#    if PROTOTYPES
        ComxChannel  channel,
        RequestedIncompatibleQosSt  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        RequestedIncompatibleQosSt  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, total_count))) ;
    CHECK (comxLong (channel, NULL_OR (value, total_count_change))) ;
    CHECK (comxLong (channel, NULL_OR (value, last_policy_id))) ;
    CHECK (comxSequence (channel, NULL_OR (value, policies), (ComxFunc) ddmxQosPolicyCount, sizeof (QosPolicyCount))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxTopicSeq() - decode/encode/erase a sequence of CORBA Topic structures.

*******************************************************************************/


errno_t  ddmxTopicSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        TopicSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        TopicSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    ddmxDataReaderSeq() - decode/encode/erase a sequence of CORBA DataReader structures.

*******************************************************************************/


errno_t  ddmxDataReaderSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        DataReaderSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DataReaderSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    ddmxConditionSeq() - decode/encode/erase a sequence of CORBA Condition structures.

*******************************************************************************/


errno_t  ddmxConditionSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        ConditionSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ConditionSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    ddmxSampleStateSeq() - decode/encode/erase a sequence of CORBA SampleStateKind structures.

*******************************************************************************/


errno_t  ddmxSampleStateSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        SampleStateSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SampleStateSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxULong, sizeof (unsigned long))) ;

}

/*!*****************************************************************************

    ddmxLifecycleStateSeq() - decode/encode/erase a sequence of CORBA LifecycleStateKind structures.

*******************************************************************************/


errno_t  ddmxLifecycleStateSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        LifecycleStateSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LifecycleStateSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxULong, sizeof (unsigned long))) ;

}

/*!*****************************************************************************

    ddmxUserDataQosPolicy() - decode/encode/erase a CORBA UserDataQosPolicy structure.

*******************************************************************************/


errno_t  ddmxUserDataQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        UserDataQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        UserDataQosPolicy  *value ;
#    endif

{

    CHECK (comxOctetSeq (channel, NULL_OR (value, data))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxDurabilityQosPolicy() - decode/encode/erase a CORBA DurabilityQosPolicy structure.

*******************************************************************************/


errno_t  ddmxDurabilityQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        DurabilityQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DurabilityQosPolicy  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->kind ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->kind = (DurabilityQosPolicyKind) enumeration ;
  }

    return (0) ;

}

/*!*****************************************************************************

    ddmxPresentationQosPolicy() - decode/encode/erase a CORBA PresentationQosPolicy structure.

*******************************************************************************/


errno_t  ddmxPresentationQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        PresentationQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        PresentationQosPolicy  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->access_scope ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->access_scope = (PresentationQosPolicyKind) enumeration ;
  }
    CHECK (comxBoolean (channel, NULL_OR (value, coherent_access))) ;
    CHECK (comxBoolean (channel, NULL_OR (value, ordered_access))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxDeadlineQosPolicy() - decode/encode/erase a CORBA DeadlineQosPolicy structure.

*******************************************************************************/


errno_t  ddmxDeadlineQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        DeadlineQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DeadlineQosPolicy  *value ;
#    endif

{

    CHECK (ddmxDuration_t (channel, NULL_OR (value, period))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxLatencyBudgetQosPolicy() - decode/encode/erase a CORBA LatencyBudgetQosPolicy structure.

*******************************************************************************/


errno_t  ddmxLatencyBudgetQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        LatencyBudgetQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LatencyBudgetQosPolicy  *value ;
#    endif

{

    CHECK (ddmxDuration_t (channel, NULL_OR (value, duration))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxOwnershipQosPolicy() - decode/encode/erase a CORBA OwnershipQosPolicy structure.

*******************************************************************************/


errno_t  ddmxOwnershipQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        OwnershipQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        OwnershipQosPolicy  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->kind ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->kind = (OwnershipQosPolicyKind) enumeration ;
  }

    return (0) ;

}

/*!*****************************************************************************

    ddmxOwnershipStrengthQosPolicy() - decode/encode/erase a CORBA OwnershipStrengthQosPolicy structure.

*******************************************************************************/


errno_t  ddmxOwnershipStrengthQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        OwnershipStrengthQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        OwnershipStrengthQosPolicy  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, value))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxLivelinessQosPolicy() - decode/encode/erase a CORBA LivelinessQosPolicy structure.

*******************************************************************************/


errno_t  ddmxLivelinessQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        LivelinessQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        LivelinessQosPolicy  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->kind ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->kind = (LivelinessQosPolicyKind) enumeration ;
  }
    CHECK (ddmxDuration_t (channel, NULL_OR (value, lease_duration))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxTimeBasedFilterQosPolicy() - decode/encode/erase a CORBA TimeBasedFilterQosPolicy structure.

*******************************************************************************/


errno_t  ddmxTimeBasedFilterQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        TimeBasedFilterQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        TimeBasedFilterQosPolicy  *value ;
#    endif

{

    CHECK (ddmxDuration_t (channel, NULL_OR (value, minimum_separation))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxPartitionQosPolicy() - decode/encode/erase a CORBA PartitionQosPolicy structure.

*******************************************************************************/


errno_t  ddmxPartitionQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        PartitionQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        PartitionQosPolicy  *value ;
#    endif

{

    CHECK (comxStringSeq (channel, NULL_OR (value, name))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxReliabilityQosPolicy() - decode/encode/erase a CORBA ReliabilityQosPolicy structure.

*******************************************************************************/


errno_t  ddmxReliabilityQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        ReliabilityQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ReliabilityQosPolicy  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->kind ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->kind = (ReliabilityQosPolicyKind) enumeration ;
  }

    return (0) ;

}

/*!*****************************************************************************

    ddmxDestinationOrderQosPolicy() - decode/encode/erase a CORBA DestinationOrderQosPolicy structure.

*******************************************************************************/


errno_t  ddmxDestinationOrderQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        DestinationOrderQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DestinationOrderQosPolicy  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->kind ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->kind = (DestinationOrderQosPolKind) enumeration ;
  }

    return (0) ;

}

/*!*****************************************************************************

    ddmxHistoryQosPolicy() - decode/encode/erase a CORBA HistoryQosPolicy structure.

*******************************************************************************/


errno_t  ddmxHistoryQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        HistoryQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        HistoryQosPolicy  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->kind ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->kind = (HistoryQosPolicyKind) enumeration ;
  }
    CHECK (comxLong (channel, NULL_OR (value, depth))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxResourceLimitsQosPolicy() - decode/encode/erase a CORBA ResourceLimitsQosPolicy structure.

*******************************************************************************/


errno_t  ddmxResourceLimitsQosPolicy (

#    if PROTOTYPES
        ComxChannel  channel,
        ResourceLimitsQosPolicy  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ResourceLimitsQosPolicy  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, max_samples))) ;
    CHECK (comxLong (channel, NULL_OR (value, max_instances))) ;
    CHECK (comxLong (channel, NULL_OR (value, max_samples_per_instance))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxDomainParticipantQos() - decode/encode/erase a CORBA DomainParticipantQos structure.

*******************************************************************************/


errno_t  ddmxDomainParticipantQos (

#    if PROTOTYPES
        ComxChannel  channel,
        DomainParticipantQos  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DomainParticipantQos  *value ;
#    endif

{

    CHECK (ddmxUserDataQosPolicy (channel, NULL_OR (value, user_data))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxTopicQos() - decode/encode/erase a CORBA TopicQos structure.

*******************************************************************************/


errno_t  ddmxTopicQos (

#    if PROTOTYPES
        ComxChannel  channel,
        TopicQos  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        TopicQos  *value ;
#    endif

{

    CHECK (ddmxDurabilityQosPolicy (channel, NULL_OR (value, durability))) ;
    CHECK (ddmxDeadlineQosPolicy (channel, NULL_OR (value, deadline))) ;
    CHECK (ddmxLatencyBudgetQosPolicy (channel, NULL_OR (value, delay_laxity))) ;
    CHECK (ddmxLivelinessQosPolicy (channel, NULL_OR (value, liveliness))) ;
    CHECK (ddmxReliabilityQosPolicy (channel, NULL_OR (value, reliability))) ;
    CHECK (ddmxDestinationOrderQosPolicy (channel, NULL_OR (value, destination_order))) ;
    CHECK (ddmxHistoryQosPolicy (channel, NULL_OR (value, history))) ;
    CHECK (ddmxResourceLimitsQosPolicy (channel, NULL_OR (value, resource_limits))) ;
    CHECK (ddmxOwnershipQosPolicy (channel, NULL_OR (value, ownership))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxDataWriterQos() - decode/encode/erase a CORBA DataWriterQos structure.

*******************************************************************************/


errno_t  ddmxDataWriterQos (

#    if PROTOTYPES
        ComxChannel  channel,
        DataWriterQos  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DataWriterQos  *value ;
#    endif

{

    CHECK (ddmxDurabilityQosPolicy (channel, NULL_OR (value, durability))) ;
    CHECK (ddmxDeadlineQosPolicy (channel, NULL_OR (value, deadline))) ;
    CHECK (ddmxLatencyBudgetQosPolicy (channel, NULL_OR (value, delay_laxity))) ;
    CHECK (ddmxLivelinessQosPolicy (channel, NULL_OR (value, liveliness))) ;
    CHECK (ddmxReliabilityQosPolicy (channel, NULL_OR (value, reliability))) ;
    CHECK (ddmxDestinationOrderQosPolicy (channel, NULL_OR (value, destination_order))) ;
    CHECK (ddmxHistoryQosPolicy (channel, NULL_OR (value, history))) ;
    CHECK (ddmxResourceLimitsQosPolicy (channel, NULL_OR (value, resource_limits))) ;
    CHECK (ddmxUserDataQosPolicy (channel, NULL_OR (value, user_data))) ;
    CHECK (ddmxOwnershipStrengthQosPolicy (channel, NULL_OR (value, ownership_strength))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxPublisherQos() - decode/encode/erase a CORBA PublisherQos structure.

*******************************************************************************/


errno_t  ddmxPublisherQos (

#    if PROTOTYPES
        ComxChannel  channel,
        PublisherQos  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        PublisherQos  *value ;
#    endif

{

    CHECK (ddmxUserDataQosPolicy (channel, NULL_OR (value, user_data))) ;
    CHECK (ddmxPresentationQosPolicy (channel, NULL_OR (value, presentation))) ;
    CHECK (ddmxPartitionQosPolicy (channel, NULL_OR (value, partition))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxDataReaderQos() - decode/encode/erase a CORBA DataReaderQos structure.

*******************************************************************************/


errno_t  ddmxDataReaderQos (

#    if PROTOTYPES
        ComxChannel  channel,
        DataReaderQos  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DataReaderQos  *value ;
#    endif

{

    CHECK (ddmxDurabilityQosPolicy (channel, NULL_OR (value, durability))) ;
    CHECK (ddmxDeadlineQosPolicy (channel, NULL_OR (value, deadline))) ;
    CHECK (ddmxLatencyBudgetQosPolicy (channel, NULL_OR (value, delay_laxity))) ;
    CHECK (ddmxLivelinessQosPolicy (channel, NULL_OR (value, liveliness))) ;
    CHECK (ddmxReliabilityQosPolicy (channel, NULL_OR (value, reliability))) ;
    CHECK (ddmxDestinationOrderQosPolicy (channel, NULL_OR (value, destination_order))) ;
    CHECK (ddmxHistoryQosPolicy (channel, NULL_OR (value, history))) ;
    CHECK (ddmxResourceLimitsQosPolicy (channel, NULL_OR (value, resource_limits))) ;
    CHECK (ddmxUserDataQosPolicy (channel, NULL_OR (value, user_data))) ;
    CHECK (ddmxTimeBasedFilterQosPolicy (channel, NULL_OR (value, time_based_filter))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxSubscriberQos() - decode/encode/erase a CORBA SubscriberQos structure.

*******************************************************************************/


errno_t  ddmxSubscriberQos (

#    if PROTOTYPES
        ComxChannel  channel,
        SubscriberQos  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SubscriberQos  *value ;
#    endif

{

    CHECK (ddmxUserDataQosPolicy (channel, NULL_OR (value, user_data))) ;
    CHECK (ddmxPresentationQosPolicy (channel, NULL_OR (value, presentation))) ;
    CHECK (ddmxPartitionQosPolicy (channel, NULL_OR (value, partition))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxSampleInfo() - decode/encode/erase a CORBA SampleInfo structure.

*******************************************************************************/


errno_t  ddmxSampleInfo (

#    if PROTOTYPES
        ComxChannel  channel,
        SampleInfo  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SampleInfo  *value ;
#    endif

{

    CHECK (comxULong (channel, NULL_OR (value, sample_state))) ;
    CHECK (comxULong (channel, NULL_OR (value, lifecycle_state))) ;
    CHECK (ddmxTime_t (channel, NULL_OR (value, source_timestamp))) ;
    CHECK (comxLong (channel, NULL_OR (value, instance_handle))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxSampleInfoSeq() - decode/encode/erase a sequence of CORBA SampleInfo structures.

*******************************************************************************/


errno_t  ddmxSampleInfoSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        SampleInfoSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SampleInfoSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) ddmxSampleInfo, sizeof (SampleInfo))) ;

}

/*!*****************************************************************************

    ddmxObjectRootSeq() - decode/encode/erase a sequence of CORBA ObjectRoot structures.

*******************************************************************************/


errno_t  ddmxObjectRootSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        ObjectRootSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ObjectRootSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    ddmxObjectHomeSeq() - decode/encode/erase a sequence of CORBA ObjectHome structures.

*******************************************************************************/


errno_t  ddmxObjectHomeSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        ObjectHomeSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ObjectHomeSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    ddmxSelectionSeq() - decode/encode/erase a sequence of CORBA Selection structures.

*******************************************************************************/


errno_t  ddmxSelectionSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        SelectionSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SelectionSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    ddmxCacheAccessSeq() - decode/encode/erase a sequence of CORBA CacheAccess structures.

*******************************************************************************/


errno_t  ddmxCacheAccessSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        CacheAccessSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        CacheAccessSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    ddmxObjectLink() - decode/encode/erase a CORBA ObjectLink structure.

*******************************************************************************/


errno_t  ddmxObjectLink (

#    if PROTOTYPES
        ComxChannel  channel,
        ObjectLink  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ObjectLink  *value ;
#    endif

{

    CHECK (comxULong (channel, NULL_OR (value, oid))) ;
    CHECK (comxULong (channel, NULL_OR (value, home_index))) ;

    return (0) ;

}

/*!*****************************************************************************

    ddmxObjectLinkSeq() - decode/encode/erase a sequence of CORBA ObjectLink structures.

*******************************************************************************/


errno_t  ddmxObjectLinkSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        ObjectLinkSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ObjectLinkSeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) ddmxObjectLink, sizeof (ObjectLink))) ;

}

/*******************************************************************************
    Lookup Tables - for converting named constants to numbers and vice-versa;
        see the coliToName() and coliToNumber() functions.
*******************************************************************************/


const  ColiMap  CacheUsageLUT[] = {
  { (long) READ_ONLY, "READ_ONLY" },
  { (long) WRITE_ONLY, "WRITE_ONLY" },
  { (long) READ_WRITE, "READ_WRITE" },
  { 0L, NULL }
} ;

const  ColiMap  DCPSStateLUT[] = {
  { (long) DCPS_INITIAL, "DCPS_INITIAL" },
  { (long) DCPS_REGISTERED, "DCPS_REGISTERED" },
  { (long) DCPS_ENABLED, "DCPS_ENABLED" },
  { 0L, NULL }
} ;

const  ColiMap  DestinationOrderQosPolKindLUT[] = {
  { (long) BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS, "BY_RECEPTION_TIMESTAMP_DESTINATIONORDER_QOS" },
  { (long) BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS, "BY_SOURCE_TIMESTAMP_DESTINATIONORDER_QOS" },
  { 0L, NULL }
} ;

const  ColiMap  DurabilityQosPolicyKindLUT[] = {
  { (long) VOLATILE_DURABILITY_QOS, "VOLATILE_DURABILITY_QOS" },
  { (long) TRANSIENT_DURABILITY_QOS, "TRANSIENT_DURABILITY_QOS" },
  { (long) PERSISTENT_DURABILITY_QOS, "PERSISTENT_DURABILITY_QOS" },
  { 0L, NULL }
} ;

const  ColiMap  HistoryQosPolicyKindLUT[] = {
  { (long) KEEP_LAST_HISTORY_QOS, "KEEP_LAST_HISTORY_QOS" },
  { (long) KEEP_ALL_HISTORY_QOS, "KEEP_ALL_HISTORY_QOS" },
  { 0L, NULL }
} ;

const  ColiMap  LivelinessQosPolicyKindLUT[] = {
  { (long) AUTOMATIC_LIVELINESS_QOS, "AUTOMATIC_LIVELINESS_QOS" },
  { (long) MANUAL_BY_PARTICIPANT_LIVELINESS_QOS, "MANUAL_BY_PARTICIPANT_LIVELINESS_QOS" },
  { (long) MANUAL_BY_TOPIC_LIVELINESS_QOS, "MANUAL_BY_TOPIC_LIVELINESS_QOS" },
  { 0L, NULL }
} ;

const  ColiMap  ObjectScopeLUT[] = {
  { (long) SIMPLE_OBJECT_SCOPE, "SIMPLE_OBJECT_SCOPE" },
  { (long) CONTAINED_OBJECTS_SCOPE, "CONTAINED_OBJECTS_SCOPE" },
  { (long) RELATED_OBJECTS_SCOPE, "RELATED_OBJECTS_SCOPE" },
  { 0L, NULL }
} ;

const  ColiMap  OwnershipQosPolicyKindLUT[] = {
  { (long) SHARED_OWNERSHIP_QOS, "SHARED_OWNERSHIP_QOS" },
  { (long) EXCLUSIVE_OWNERSHIP_QOS, "EXCLUSIVE_OWNERSHIP_QOS" },
  { 0L, NULL }
} ;

const  ColiMap  PresentationQosPolicyKindLUT[] = {
  { (long) INSTANCE_PRESENTATION_QOS, "INSTANCE_PRESENTATION_QOS" },
  { (long) TOPIC_PRESENTATION_QOS, "TOPIC_PRESENTATION_QOS" },
  { (long) GROUP_PRESENTATION_QOS, "GROUP_PRESENTATION_QOS" },
  { 0L, NULL }
} ;

const  ColiMap  ReferenceScopeLUT[] = {
  { (long) SIMPLE_CONTENT_SCOPE, "SIMPLE_CONTENT_SCOPE" },
  { (long) REFERENCED_CONTENTS_SCOPE, "REFERENCED_CONTENTS_SCOPE" },
  { 0L, NULL }
} ;

const  ColiMap  ReliabilityQosPolicyKindLUT[] = {
  { (long) BEST_EFFORT_RELIABILITY_QOS, "BEST_EFFORT_RELIABILITY_QOS" },
  { (long) RELIABLE_RELIABILITY_QOS, "RELIABLE_RELIABILITY_QOS" },
  { 0L, NULL }
} ;

const  ColiMap  SampleRejectedStatusKindLUT[] = {
  { (long) REJECTED_BY_INSTANCE_LIMIT, "REJECTED_BY_INSTANCE_LIMIT" },
  { (long) REJECTED_BY_TOPIC_LIMIT, "REJECTED_BY_TOPIC_LIMIT" },
  { 0L, NULL }
} ;
