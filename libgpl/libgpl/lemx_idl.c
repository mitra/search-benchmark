/* $Id: lemx_idl.c,v 1.8 2006/04/28 20:05:23 alex Exp $ */
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

    lemxSAdministrative() - decode/encode/erase a CORBA SAdministrative structure.

*******************************************************************************/


errno_t  lemxSAdministrative (

#    if PROTOTYPES
        ComxChannel  channel,
        SAdministrative  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SAdministrative  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, model_number))) ;
    CHECK (comxString (channel, NULL_OR (value, serial_number))) ;
    CHECK (comxString (channel, NULL_OR (value, software_version))) ;
    CHECK (comxString (channel, NULL_OR (value, support_address))) ;
    CHECK (comxString (channel, NULL_OR (value, manufacturer_id))) ;
    CHECK (comxString (channel, NULL_OR (value, manufacturer_name))) ;
    CHECK (comxString (channel, NULL_OR (value, description))) ;
    CHECK (comxString (channel, NULL_OR (value, update_address))) ;
    CHECK (comxString (channel, NULL_OR (value, dcd_version))) ;
    CHECK (comxString (channel, NULL_OR (value, protocol))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSItemData() - decode/encode/erase a CORBA SItemData structure.

*******************************************************************************/


errno_t  lemxSItemData (

#    if PROTOTYPES
        ComxChannel  channel,
        SItemData  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SItemData  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, item))) ;
    CHECK (comxString (channel, NULL_OR (value, value))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSRange() - decode/encode/erase a CORBA SRange structure.

*******************************************************************************/


errno_t  lemxSRange (

#    if PROTOTYPES
        ComxChannel  channel,
        SRange  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SRange  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->range_type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->range_type = (ENumberType) enumeration ;
  }
    CHECK (comxString (channel, NULL_OR (value, low_limit))) ;
    CHECK (comxString (channel, NULL_OR (value, high_limit))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSArgument() - decode/encode/erase a CORBA SArgument structure.

*******************************************************************************/


errno_t  lemxSArgument (

#    if PROTOTYPES
        ComxChannel  channel,
        SArgument  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SArgument  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, name))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->argument_type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->argument_type = (EVariableType) enumeration ;
  }
    CHECK (gimxAny (channel, NULL_OR (value, default_value))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->transfer_type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->transfer_type = (ETransferType) enumeration ;
  }
    CHECK (comxString (channel, NULL_OR (value, description))) ;
    CHECK (comxSequence (channel, NULL_OR (value, properties), (ComxFunc) lemxSItemData, sizeof (SItemData))) ;
    CHECK (lemxSRange (channel, NULL_OR (value, range))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSComponentID() - decode/encode/erase a CORBA SComponentID structure.

*******************************************************************************/


errno_t  lemxSComponentID (

#    if PROTOTYPES
        ComxChannel  channel,
        SComponentID  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SComponentID  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, workcell_id))) ;
    CHECK (comxString (channel, NULL_OR (value, slm_id))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->component_category ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->component_category = (EComponentCategory) enumeration ;
  }
    CHECK (comxString (channel, NULL_OR (value, subunit_id))) ;
    CHECK (comxString (channel, NULL_OR (value, resource_id))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSOwnership() - decode/encode/erase a CORBA SOwnership structure.

*******************************************************************************/


errno_t  lemxSOwnership (

#    if PROTOTYPES
        ComxChannel  channel,
        SOwnership  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SOwnership  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->owner_status ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->owner_status = (EOwnerstatus) enumeration ;
  }
    CHECK (lemxSComponentID (channel, NULL_OR (value, owner))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSValue() - decode/encode/erase a CORBA SValue structure.

*******************************************************************************/


errno_t  lemxSValue (

#    if PROTOTYPES
        ComxChannel  channel,
        SValue  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SValue  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, value))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->type = (ENumberType) enumeration ;
  }
    CHECK (comxLong (channel, NULL_OR (value, exponent))) ;
    CHECK (comxString (channel, NULL_OR (value, unit))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSTranslation() - decode/encode/erase a CORBA STranslation structure.

*******************************************************************************/


errno_t  lemxSTranslation (

#    if PROTOTYPES
        ComxChannel  channel,
        STranslation  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        STranslation  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, x))) ;
    CHECK (comxLong (channel, NULL_OR (value, y))) ;
    CHECK (comxLong (channel, NULL_OR (value, z))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSRotation() - decode/encode/erase a CORBA SRotation structure.

*******************************************************************************/


errno_t  lemxSRotation (

#    if PROTOTYPES
        ComxChannel  channel,
        SRotation  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SRotation  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, x))) ;
    CHECK (comxLong (channel, NULL_OR (value, y))) ;
    CHECK (comxLong (channel, NULL_OR (value, z))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSSystemVariable() - decode/encode/erase a CORBA SSystemVariable structure.

*******************************************************************************/


errno_t  lemxSSystemVariable (

#    if PROTOTYPES
        ComxChannel  channel,
        SSystemVariable  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SSystemVariable  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, variable_id))) ;
    CHECK (comxString (channel, NULL_OR (value, description))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->data_type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->data_type = (EVariableType) enumeration ;
  }
    CHECK (gimxAny (channel, NULL_OR (value, current_value))) ;
    CHECK (comxString (channel, NULL_OR (value, category))) ;
    CHECK (lemxSRange (channel, NULL_OR (value, value_range))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSCapacity() - decode/encode/erase a CORBA SCapacity structure.

*******************************************************************************/


errno_t  lemxSCapacity (

#    if PROTOTYPES
        ComxChannel  channel,
        SCapacity  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SCapacity  *value ;
#    endif

{

    CHECK (lemxSValue (channel, NULL_OR (value, min_capacity))) ;
    CHECK (lemxSValue (channel, NULL_OR (value, max_capacity))) ;
    CHECK (lemxSValue (channel, NULL_OR (value, fill_steps))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSLocation() - decode/encode/erase a CORBA SLocation structure.

*******************************************************************************/


errno_t  lemxSLocation (

#    if PROTOTYPES
        ComxChannel  channel,
        SLocation  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SLocation  *value ;
#    endif

{

    CHECK (lemxSRotation (channel, NULL_OR (value, rotation))) ;
    CHECK (lemxSTranslation (channel, NULL_OR (value, translation))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSGeometricModel() - decode/encode/erase a CORBA SGeometricModel structure.

*******************************************************************************/


errno_t  lemxSGeometricModel (

#    if PROTOTYPES
        ComxChannel  channel,
        SGeometricModel  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SGeometricModel  *value ;
#    endif

{

    CHECK (gimxAny (channel, NULL_OR (value, model))) ;
    CHECK (gimxAny (channel, NULL_OR (value, access_curve))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSDimension() - decode/encode/erase a CORBA SDimension structure.

*******************************************************************************/


errno_t  lemxSDimension (

#    if PROTOTYPES
        ComxChannel  channel,
        SDimension  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SDimension  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, height))) ;
    CHECK (comxString (channel, NULL_OR (value, width))) ;
    CHECK (lemxSGeometricModel (channel, NULL_OR (value, geometric_model))) ;
    CHECK (comxString (channel, NULL_OR (value, length))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSPhysicalCharacteristics() - decode/encode/erase a CORBA SPhysicalCharacteristics structure.

*******************************************************************************/


errno_t  lemxSPhysicalCharacteristics (

#    if PROTOTYPES
        ComxChannel  channel,
        SPhysicalCharacteristics  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SPhysicalCharacteristics  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, weight))) ;
    CHECK (lemxSLocation (channel, NULL_OR (value, location))) ;
    CHECK (lemxSDimension (channel, NULL_OR (value, dimension))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSeqAny() - decode/encode/erase a sequence of CORBA Any structures.

*******************************************************************************/


errno_t  lemxSeqAny (

#    if PROTOTYPES
        ComxChannel  channel,
        SeqAny  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SeqAny  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxAny, sizeof (Any))) ;

}

/*!*****************************************************************************

    lemxCommands_def() - decode/encode/erase a sequence of CORBA ICommand structures.

*******************************************************************************/


errno_t  lemxCommands_def (

#    if PROTOTYPES
        ComxChannel  channel,
        commands_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        commands_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxAdministrative_def() - decode/encode/erase a sequence of CORBA SAdministrative structures.

*******************************************************************************/


errno_t  lemxAdministrative_def (

#    if PROTOTYPES
        ComxChannel  channel,
        administrative_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        administrative_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSAdministrative, sizeof (SAdministrative))) ;

}

/*!*****************************************************************************

    lemxEvents_def() - decode/encode/erase a sequence of CORBA IEvent structures.

*******************************************************************************/


errno_t  lemxEvents_def (

#    if PROTOTYPES
        ComxChannel  channel,
        events_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        events_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxPorts_def() - decode/encode/erase a sequence of CORBA IPort structures.

*******************************************************************************/


errno_t  lemxPorts_def (

#    if PROTOTYPES
        ComxChannel  channel,
        ports_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ports_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxResources_def() - decode/encode/erase a sequence of CORBA IResource structures.

*******************************************************************************/


errno_t  lemxResources_def (

#    if PROTOTYPES
        ComxChannel  channel,
        resources_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        resources_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxProperties_def() - decode/encode/erase a sequence of CORBA SItemData structures.

*******************************************************************************/


errno_t  lemxProperties_def (

#    if PROTOTYPES
        ComxChannel  channel,
        properties_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        properties_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSItemData, sizeof (SItemData))) ;

}

/*!*****************************************************************************

    lemxSubunit_external_macros_def() - decode/encode/erase a sequence of CORBA IExtMacroCommandList structures.

*******************************************************************************/


errno_t  lemxSubunit_external_macros_def (

#    if PROTOTYPES
        ComxChannel  channel,
        subunit_external_macros_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        subunit_external_macros_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxAccess_ports_def() - decode/encode/erase a sequence of CORBA IPort structures.

*******************************************************************************/


errno_t  lemxAccess_ports_def (

#    if PROTOTYPES
        ComxChannel  channel,
        access_ports_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        access_ports_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxContent_def() - decode/encode/erase a sequence of CORBA IResource structures.

*******************************************************************************/


errno_t  lemxContent_def (

#    if PROTOTYPES
        ComxChannel  channel,
        content_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        content_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxRequired_configurations_def() - decode/encode/erase a sequence of CORBA ICommand structures.

*******************************************************************************/


errno_t  lemxRequired_configurations_def (

#    if PROTOTYPES
        ComxChannel  channel,
        required_configurations_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        required_configurations_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxRequired_resources_def() - decode/encode/erase a sequence of CORBA IResource structures.

*******************************************************************************/


errno_t  lemxRequired_resources_def (

#    if PROTOTYPES
        ComxChannel  channel,
        required_resources_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        required_resources_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxProduced_resources_def() - decode/encode/erase a sequence of CORBA IResource structures.

*******************************************************************************/


errno_t  lemxProduced_resources_def (

#    if PROTOTYPES
        ComxChannel  channel,
        produced_resources_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        produced_resources_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxPort_inputs_def() - decode/encode/erase a sequence of CORBA IPort structures.

*******************************************************************************/


errno_t  lemxPort_inputs_def (

#    if PROTOTYPES
        ComxChannel  channel,
        port_inputs_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        port_inputs_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxPort_outputs_def() - decode/encode/erase a sequence of CORBA IPort structures.

*******************************************************************************/


errno_t  lemxPort_outputs_def (

#    if PROTOTYPES
        ComxChannel  channel,
        port_outputs_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        port_outputs_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxMeasurement_bounds_def() - decode/encode/erase a sequence of CORBA SRange structures.

*******************************************************************************/


errno_t  lemxMeasurement_bounds_def (

#    if PROTOTYPES
        ComxChannel  channel,
        measurement_bounds_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        measurement_bounds_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSRange, sizeof (SRange))) ;

}

/*!*****************************************************************************

    lemxFormal_arguments_def() - decode/encode/erase a sequence of CORBA SArgument structures.

*******************************************************************************/


errno_t  lemxFormal_arguments_def (

#    if PROTOTYPES
        ComxChannel  channel,
        formal_arguments_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        formal_arguments_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSArgument, sizeof (SArgument))) ;

}

/*!*****************************************************************************

    lemxSync_response_data_def() - decode/encode/erase a sequence of CORBA SArgument structures.

*******************************************************************************/


errno_t  lemxSync_response_data_def (

#    if PROTOTYPES
        ComxChannel  channel,
        sync_response_data_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        sync_response_data_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSArgument, sizeof (SArgument))) ;

}

/*!*****************************************************************************

    lemxExclusion_list_def() - decode/encode/erase a sequence of CORBA SItemData structures.

*******************************************************************************/


errno_t  lemxExclusion_list_def (

#    if PROTOTYPES
        ComxChannel  channel,
        exclusion_list_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        exclusion_list_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSItemData, sizeof (SItemData))) ;

}

/*!*****************************************************************************

    lemxSub_units_def() - decode/encode/erase a sequence of CORBA ISubUnit structures.

*******************************************************************************/


errno_t  lemxSub_units_def (

#    if PROTOTYPES
        ComxChannel  channel,
        sub_units_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        sub_units_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxDowntime_def() - decode/encode/erase a sequence of CORBA IDownTime structures.

*******************************************************************************/


errno_t  lemxDowntime_def (

#    if PROTOTYPES
        ComxChannel  channel,
        downtime_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        downtime_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxSystem_variables_def() - decode/encode/erase a sequence of CORBA SSystemVariable structures.

*******************************************************************************/


errno_t  lemxSystem_variables_def (

#    if PROTOTYPES
        ComxChannel  channel,
        system_variables_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        system_variables_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSSystemVariable, sizeof (SSystemVariable))) ;

}

/*!*****************************************************************************

    lemxEvent_data_types_def() - decode/encode/erase a sequence of CORBA SArgument structures.

*******************************************************************************/


errno_t  lemxEvent_data_types_def (

#    if PROTOTYPES
        ComxChannel  channel,
        event_data_types_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        event_data_types_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSArgument, sizeof (SArgument))) ;

}

/*!*****************************************************************************

    lemxPossible_event_reaction_def() - decode/encode/erase a sequence of CORBA ICommand structures.

*******************************************************************************/


errno_t  lemxPossible_event_reaction_def (

#    if PROTOTYPES
        ComxChannel  channel,
        possible_event_reaction_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        possible_event_reaction_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxSExtMacroCommand() - decode/encode/erase a CORBA SExtMacroCommand structure.

*******************************************************************************/


errno_t  lemxSExtMacroCommand (

#    if PROTOTYPES
        ComxChannel  channel,
        SExtMacroCommand  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SExtMacroCommand  *value ;
#    endif

{

    CHECK (lemxSeqAny (channel, NULL_OR (value, argument_values))) ;
    CHECK (gimxIOR (channel, NULL_OR (value, command_ref))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxExt_macros_def() - decode/encode/erase a sequence of CORBA IExtMacroCommandList structures.

*******************************************************************************/


errno_t  lemxExt_macros_def (

#    if PROTOTYPES
        ComxChannel  channel,
        ext_macros_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ext_macros_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxSlms_def() - decode/encode/erase a sequence of CORBA ISLM structures.

*******************************************************************************/


errno_t  lemxSlms_def (

#    if PROTOTYPES
        ComxChannel  channel,
        slms_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        slms_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxWorkcells_def() - decode/encode/erase a sequence of CORBA IWorkCell structures.

*******************************************************************************/


errno_t  lemxWorkcells_def (

#    if PROTOTYPES
        ComxChannel  channel,
        workcells_def  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        workcells_def  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    lemxSSysVar() - decode/encode/erase a CORBA SSysVar structure.

*******************************************************************************/


errno_t  lemxSSysVar (

#    if PROTOTYPES
        ComxChannel  channel,
        SSysVar  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SSysVar  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, variable_id))) ;
    CHECK (comxString (channel, NULL_OR (value, description))) ;
    CHECK (comxString (channel, NULL_OR (value, category))) ;
    CHECK (gimxAny (channel, NULL_OR (value, value))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSSubState() - decode/encode/erase a CORBA SSubState structure.

*******************************************************************************/


errno_t  lemxSSubState (

#    if PROTOTYPES
        ComxChannel  channel,
        SSubState  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SSubState  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, sub_unit_id))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->sub_unit_state ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->sub_unit_state = (ESubCtrlState) enumeration ;
  }

    return (0) ;

}

/*!*****************************************************************************

    lemxSeqSubStates() - decode/encode/erase a sequence of CORBA SSubState structures.

*******************************************************************************/


errno_t  lemxSeqSubStates (

#    if PROTOTYPES
        ComxChannel  channel,
        SeqSubStates  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SeqSubStates  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSSubState, sizeof (SSubState))) ;

}

/*!*****************************************************************************

    lemxSLM_RESULT() - decode/encode/erase a CORBA SLM_RESULT structure.

*******************************************************************************/


errno_t  lemxSLM_RESULT (

#    if PROTOTYPES
        ComxChannel  channel,
        SLM_RESULT  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SLM_RESULT  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->result_code ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->result_code = (EResultCode) enumeration ;
  }
    CHECK (comxString (channel, NULL_OR (value, minor_code))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->main_state ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->main_state = (EMainCtrlState) enumeration ;
  }
    CHECK (lemxSeqSubStates (channel, NULL_OR (value, sub_states))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->lr_mode ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->lr_mode = (ELocalRemote) enumeration ;
  }
    CHECK (comxString (channel, NULL_OR (value, message))) ;

    return (0) ;

}

/*!*****************************************************************************

    lemxSeqSysVar() - decode/encode/erase a sequence of CORBA SSysVar structures.

*******************************************************************************/


errno_t  lemxSeqSysVar (

#    if PROTOTYPES
        ComxChannel  channel,
        SeqSysVar  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SeqSysVar  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) lemxSSysVar, sizeof (SSysVar))) ;

}

/*******************************************************************************
    Lookup Tables - for converting named constants to numbers and vice-versa;
        see the coliToName() and coliToNumber() functions.
*******************************************************************************/


const  ColiMap  EAccessTypeLUT[] = {
  { (long) INLET, "INLET" },
  { (long) OUTLET, "OUTLET" },
  { (long) INOUTLET, "INOUTLET" },
  { (long) TRANSFER, "TRANSFER" },
  { 0L, NULL }
} ;

const  ColiMap  ECapacityTypeLUT[] = {
  { (long) FINITE, "FINITE" },
  { (long) ECT_INFINITE, "ECT_INFINITE" },
  { 0L, NULL }
} ;

const  ColiMap  ECommandCategoryLUT[] = {
  { (long) _INIT, "_INIT" },
  { (long) CONTROL, "CONTROL" },
  { (long) FUNCTION, "FUNCTION" },
  { (long) CONFIGURE, "CONFIGURE" },
  { (long) RECOVERY, "RECOVERY" },
  { (long) STATUSREQ, "STATUSREQ" },
  { (long) MAINTAIN, "MAINTAIN" },
  { (long) CALIBRATE, "CALIBRATE" },
  { (long) ADMIN, "ADMIN" },
  { (long) RESULT, "RESULT" },
  { 0L, NULL }
} ;

const  ColiMap  ECommandTypeLUT[] = {
  { (long) ATOMIC, "ATOMIC" },
  { (long) MACRO, "MACRO" },
  { 0L, NULL }
} ;

const  ColiMap  EComponentCategoryLUT[] = {
  { (long) SYSTEM, "SYSTEM" },
  { (long) WORKCELL, "WORKCELL" },
  { (long) SLM, "SLM" },
  { (long) RESOURCE, "RESOURCE" },
  { (long) SUBUNIT, "SUBUNIT" },
  { 0L, NULL }
} ;

const  ColiMap  EDataLinkTypeLUT[] = {
  { (long) EDLT_FILE, "EDLT_FILE" },
  { (long) DB, "DB" },
  { (long) OPERATION, "OPERATION" },
  { 0L, NULL }
} ;

const  ColiMap  EDownTimeCategoryLUT[] = {
  { (long) CLEANING, "CLEANING" },
  { (long) CALIBRATION, "CALIBRATION" },
  { (long) SOFTWARE_UPDATE, "SOFTWARE_UPDATE" },
  { (long) HARDWARE_UPDATE, "HARDWARE_UPDATE" },
  { 0L, NULL }
} ;

const  ColiMap  EDownTimeTypeLUT[] = {
  { (long) ESTIMATED, "ESTIMATED" },
  { (long) ACTUAL, "ACTUAL" },
  { 0L, NULL }
} ;

const  ColiMap  EEventCategoryLUT[] = {
  { (long) ALARM, "ALARM" },
  { (long) MESSAGE, "MESSAGE" },
  { (long) DATA_DIRECT, "DATA_DIRECT" },
  { (long) DATA_LINK, "DATA_LINK" },
  { (long) SYSVAR_CHANGED, "SYSVAR_CHANGED" },
  { (long) CONTROL_STATE_CHANGED, "CONTROL_STATE_CHANGED" },
  { (long) SLM_STATE_CHANGED, "SLM_STATE_CHANGED" },
  { 0L, NULL }
} ;

const  ColiMap  EEventTypeLUT[] = {
  { (long) EET_ALARM, "EET_ALARM" },
  { (long) EET_MESSAGE, "EET_MESSAGE" },
  { (long) EET_DATA_DIRECT, "EET_DATA_DIRECT" },
  { (long) EET_DATA_LINK, "EET_DATA_LINK" },
  { (long) EET_SYSVAR_CHANGED, "EET_SYSVAR_CHANGED" },
  { (long) CONTROL_STATE_CHANGE, "CONTROL_STATE_CHANGE" },
  { (long) DEVICE_STATE_CHANGED, "DEVICE_STATE_CHANGED" },
  { 0L, NULL }
} ;

const  ColiMap  ELocalRemoteLUT[] = {
  { (long) _LOCAL, "_LOCAL" },
  { (long) REMOTE, "REMOTE" },
  { (long) AVAILABLE, "AVAILABLE" },
  { 0L, NULL }
} ;

const  ColiMap  ELocalRemote_ArgTypeLUT[] = {
  { (long) LOCAL_CTRL_REQ, "LOCAL_CTRL_REQ" },
  { (long) REMOTE_CTRL_REQ, "REMOTE_CTRL_REQ" },
  { (long) FORCE_LOCAL_CTRL, "FORCE_LOCAL_CTRL" },
  { (long) RELEASE_CTRL, "RELEASE_CTRL" },
  { 0L, NULL }
} ;

const  ColiMap  EMainCtrlStateLUT[] = {
  { (long) POWERED_UP, "POWERED_UP" },
  { (long) INITIALIZING, "INITIALIZING" },
  { (long) NORMAL_OP, "NORMAL_OP" },
  { (long) EMCS_ERROR, "EMCS_ERROR" },
  { (long) ESTOPPED, "ESTOPPED" },
  { (long) CLEARING, "CLEARING" },
  { (long) CLEARED, "CLEARED" },
  { (long) SHUTDOWN, "SHUTDOWN" },
  { (long) DOWN, "DOWN" },
  { 0L, NULL }
} ;

const  ColiMap  ENumberTypeLUT[] = {
  { (long) LONG_NTYPE, "LONG_NTYPE" },
  { (long) FLOAT_NTYPE, "FLOAT_NTYPE" },
  { 0L, NULL }
} ;

const  ColiMap  EOwnerstatusLUT[] = {
  { (long) PRIVATE_OWNER, "PRIVATE_OWNER" },
  { (long) LOCKED, "LOCKED" },
  { (long) UNLOCKED, "UNLOCKED" },
  { 0L, NULL }
} ;

const  ColiMap  EPortTypeLUT[] = {
  { (long) DATA, "DATA" },
  { (long) MATERIAL, "MATERIAL" },
  { 0L, NULL }
} ;

const  ColiMap  EResourceCategoryLUT[] = {
  { (long) HARDWARE, "HARDWARE" },
  { (long) SAMPLE, "SAMPLE" },
  { (long) REAGENT, "REAGENT" },
  { (long) WASTE, "WASTE" },
  { (long) SPACE, "SPACE" },
  { (long) BUFFER, "BUFFER" },
  { (long) UNDEFINED, "UNDEFINED" },
  { 0L, NULL }
} ;

const  ColiMap  EResultCodeLUT[] = {
  { (long) ERC_SUCCESS, "ERC_SUCCESS" },
  { (long) REMOTE_CTRL_REQ_DENIED, "REMOTE_CTRL_REQ_DENIED" },
  { (long) LOCAL_CTRL_REQ_DENIED, "LOCAL_CTRL_REQ_DENIED" },
  { (long) FORCE_LOCAL_CTRL_FAILED, "FORCE_LOCAL_CTRL_FAILED" },
  { (long) RELEASE_REMOTE_CTRL_FAILED, "RELEASE_REMOTE_CTRL_FAILED" },
  { (long) READ_DCD_FAILED, "READ_DCD_FAILED" },
  { (long) WRITE_DCD_FAILED, "WRITE_DCD_FAILED" },
  { (long) DCD_NOT_AVAILABLE, "DCD_NOT_AVAILABLE" },
  { (long) SUBUNIT_UNKNOWN, "SUBUNIT_UNKNOWN" },
  { (long) DEVICE_HARDWARE_ERROR, "DEVICE_HARDWARE_ERROR" },
  { (long) COMMUNICATION_ERROR, "COMMUNICATION_ERROR" },
  { (long) TIMEOUT, "TIMEOUT" },
  { (long) UNSPECIFIED_ERROR, "UNSPECIFIED_ERROR" },
  { (long) SUB_STATE_INCORRECT, "SUB_STATE_INCORRECT" },
  { (long) MAIN_STATE_INCORRECT, "MAIN_STATE_INCORRECT" },
  { (long) PAUSE_REQUEST_DENIED, "PAUSE_REQUEST_DENIED" },
  { (long) TIME_SYNCHRONIZATION_FAILED, "TIME_SYNCHRONIZATION_FAILED" },
  { (long) UNKNOWN_COMMAND, "UNKNOWN_COMMAND" },
  { (long) TIME_SYNCHRONIZATION_NOT_AVAILABLE, "TIME_SYNCHRONIZATION_NOT_AVAILABLE" },
  { (long) WRONG_ARGUMENT_LIST, "WRONG_ARGUMENT_LIST" },
  { (long) DATA_ID_UNKNOWN, "DATA_ID_UNKNOWN" },
  { (long) INVALID_DATA, "INVALID_DATA" },
  { (long) ACCESS_DENIED, "ACCESS_DENIED" },
  { (long) EXECUTING_MACRO, "EXECUTING_MACRO" },
  { (long) EXECUTION_STOPPED, "EXECUTION_STOPPED" },
  { 0L, NULL }
} ;

const  ColiMap  ESubCtrlStateLUT[] = {
  { (long) SUB_POWERED_UP, "SUB_POWERED_UP" },
  { (long) SUB_INITIALIZING, "SUB_INITIALIZING" },
  { (long) SUB_SHUTDOWN, "SUB_SHUTDOWN" },
  { (long) SUB_DOWN, "SUB_DOWN" },
  { (long) SUB_ERROR, "SUB_ERROR" },
  { (long) SUB_CLEARING, "SUB_CLEARING" },
  { (long) SUB_CLEARED, "SUB_CLEARED" },
  { (long) SUB_ABORTED, "SUB_ABORTED" },
  { (long) SUB_ESTOPPED, "SUB_ESTOPPED" },
  { (long) SUB_IDLE, "SUB_IDLE" },
  { (long) SUB_PROCESSING, "SUB_PROCESSING" },
  { (long) SUB_PAUSING, "SUB_PAUSING" },
  { (long) SUB_PAUSED, "SUB_PAUSED" },
  { (long) SUB_RESUMING, "SUB_RESUMING" },
  { 0L, NULL }
} ;

const  ColiMap  ESystemDomainLUT[] = {
  { (long) COUNTRY, "COUNTRY" },
  { (long) DEPARTMENT, "DEPARTMENT" },
  { (long) SUBDIVISION, "SUBDIVISION" },
  { (long) LABORATORY, "LABORATORY" },
  { (long) ROOM, "ROOM" },
  { 0L, NULL }
} ;

const  ColiMap  ETransferTypeLUT[] = {
  { (long) INTRANSFER, "INTRANSFER" },
  { (long) OUTTRANSFER, "OUTTRANSFER" },
  { (long) INOUTTRANSFER, "INOUTTRANSFER" },
  { 0L, NULL }
} ;

const  ColiMap  EVariableTypeLUT[] = {
  { (long) LONG_TYPE, "LONG_TYPE" },
  { (long) FLOAT_TYPE, "FLOAT_TYPE" },
  { (long) BOOLEAN_TYPE, "BOOLEAN_TYPE" },
  { (long) STRING_TYPE, "STRING_TYPE" },
  { (long) OCTET_TYPE, "OCTET_TYPE" },
  { (long) SEQ_LONG_TYPE, "SEQ_LONG_TYPE" },
  { (long) SEQ_FLOAT_TYPE, "SEQ_FLOAT_TYPE" },
  { (long) SEQ_OCTET_TYPE, "SEQ_OCTET_TYPE" },
  { 0L, NULL }
} ;
