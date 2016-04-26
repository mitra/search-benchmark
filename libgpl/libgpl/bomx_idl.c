/* $Id: bomx_idl.c,v 1.1 2009/09/23 13:37:41 alex Exp $ */
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

    bomxActivationEnvValue() - decode/encode/erase a CORBA ActivationEnvValue structure.

*******************************************************************************/


errno_t  bomxActivationEnvValue (

#    if PROTOTYPES
        ComxChannel  channel,
        ActivationEnvValue  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ActivationEnvValue  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, name))) ;
    CHECK (comxString (channel, NULL_OR (value, value))) ;
    CHECK (comxLong (channel, NULL_OR (value, flags))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxActivationEnvironment() - decode/encode/erase a sequence of CORBA ActivationEnvValue structures.

*******************************************************************************/


errno_t  bomxActivationEnvironment (

#    if PROTOTYPES
        ComxChannel  channel,
        ActivationEnvironment  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ActivationEnvironment  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) bomxActivationEnvValue, sizeof (ActivationEnvValue))) ;

}

/*!*****************************************************************************

    bomxActivationResultData() - decode/encode/erase a CORBA ActivationResultData structure.

*******************************************************************************/


errno_t  bomxActivationResultData (

#    if PROTOTYPES
        ComxChannel  channel,
        ActivationResultData  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ActivationResultData  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->which ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->which = (ActivationResultType) enumeration ;
  }

    switch (value->which) {
    case ACTIVATION_RESULT_OBJECT:
        CHECK (gimxIOR (channel, NULL_OR (value, data.res_object))) ;
        break ;
    case ACTIVATION_RESULT_SHLIB:
        CHECK (comxStringSeq (channel, NULL_OR (value, data.res_shlib))) ;
        break ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(bomxActivationResultData) Invalid ActivationResultType: %lu\n", (unsigned long) value->which) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

    bomxActivationResult() - decode/encode/erase a CORBA ActivationResult structure.

*******************************************************************************/


errno_t  bomxActivationResult (

#    if PROTOTYPES
        ComxChannel  channel,
        ActivationResult  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ActivationResult  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, aid))) ;
    CHECK (bomxActivationResultData (channel, NULL_OR (value, res))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxActivationPropertyValue() - decode/encode/erase a CORBA ActivationPropertyValue structure.

*******************************************************************************/


errno_t  bomxActivationPropertyValue (

#    if PROTOTYPES
        ComxChannel  channel,
        ActivationPropertyValue  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ActivationPropertyValue  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->which ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->which = (ActivationPropertyType) enumeration ;
  }

    switch (value->which) {
    case ACTIVATION_P_STRING:
        CHECK (comxString (channel, NULL_OR (value, data.value_string))) ;
        break ;
    case ACTIVATION_P_NUMBER:
        CHECK (comxDouble (channel, NULL_OR (value, data.value_number))) ;
        break ;
    case ACTIVATION_P_BOOLEAN:
        CHECK (comxBoolean (channel, NULL_OR (value, data.value_boolean))) ;
        break ;
    case ACTIVATION_P_STRINGV:
        CHECK (comxStringSeq (channel, NULL_OR (value, data.value_stringv))) ;
        break ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(bomxActivationPropertyValue) Invalid ActivationPropertyType: %lu\n", (unsigned long) value->which) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

    bomxActivationProperty() - decode/encode/erase a CORBA ActivationProperty structure.

*******************************************************************************/


errno_t  bomxActivationProperty (

#    if PROTOTYPES
        ComxChannel  channel,
        ActivationProperty  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ActivationProperty  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, name))) ;
    CHECK (bomxActivationPropertyValue (channel, NULL_OR (value, v))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxServerInfo() - decode/encode/erase a CORBA ServerInfo structure.

*******************************************************************************/


errno_t  bomxServerInfo (

#    if PROTOTYPES
        ComxChannel  channel,
        ServerInfo  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ServerInfo  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, iid))) ;
    CHECK (comxString (channel, NULL_OR (value, server_type))) ;
    CHECK (comxString (channel, NULL_OR (value, location_info))) ;
    CHECK (comxString (channel, NULL_OR (value, username))) ;
    CHECK (comxString (channel, NULL_OR (value, hostname))) ;
    CHECK (comxString (channel, NULL_OR (value, domain))) ;
    CHECK (comxSequence (channel, NULL_OR (value, props), (ComxFunc) bomxActivationProperty, sizeof (ActivationProperty))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxServerInfoList() - decode/encode/erase a sequence of CORBA ServerInfo structures.

*******************************************************************************/


errno_t  bomxServerInfoList (

#    if PROTOTYPES
        ComxChannel  channel,
        ServerInfoList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ServerInfoList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) bomxServerInfo, sizeof (ServerInfo))) ;

}

/*!*****************************************************************************

    bomxResolveOptions() - decode/encode/erase a CORBA ResolveOptions structure.

*******************************************************************************/


errno_t  bomxResolveOptions (

#    if PROTOTYPES
        ComxChannel  channel,
        ResolveOptions  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ResolveOptions  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, flags))) ;
    CHECK (comxLong (channel, NULL_OR (value, timeout))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxStorageInfo() - decode/encode/erase a CORBA StorageInfo structure.

*******************************************************************************/


errno_t  bomxStorageInfo (

#    if PROTOTYPES
        ComxChannel  channel,
        StorageInfo  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        StorageInfo  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, name))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->type = (StorageType) enumeration ;
  }
    CHECK (comxString (channel, NULL_OR (value, content_type))) ;
    CHECK (comxLong (channel, NULL_OR (value, size))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxDirectoryList() - decode/encode/erase a sequence of CORBA StorageInfo structures.

*******************************************************************************/


errno_t  bomxDirectoryList (

#    if PROTOTYPES
        ComxChannel  channel,
        DirectoryList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DirectoryList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) bomxStorageInfo, sizeof (StorageInfo))) ;

}

/*!*****************************************************************************

    bomxContentTypeList() - decode/encode/erase a sequence of CORBA ContentType structures.

*******************************************************************************/


errno_t  bomxContentTypeList (

#    if PROTOTYPES
        ComxChannel  channel,
        ContentTypeList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ContentTypeList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxString, sizeof (char *))) ;

}

/*!*****************************************************************************

    bomxMotionEvent() - decode/encode/erase a CORBA MotionEvent structure.

*******************************************************************************/


errno_t  bomxMotionEvent (

#    if PROTOTYPES
        ComxChannel  channel,
        MotionEvent  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        MotionEvent  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, time))) ;
    CHECK (comxDouble (channel, NULL_OR (value, x))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y))) ;
    CHECK (comxDouble (channel, NULL_OR (value, x_root))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y_root))) ;
    CHECK (comxDouble (channel, NULL_OR (value, pressure))) ;
    CHECK (comxDouble (channel, NULL_OR (value, xtilt))) ;
    CHECK (comxDouble (channel, NULL_OR (value, ytilt))) ;
    CHECK (comxLong (channel, NULL_OR (value, state))) ;
    CHECK (comxBoolean (channel, NULL_OR (value, is_hint))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxButtonEvent() - decode/encode/erase a CORBA ButtonEvent structure.

*******************************************************************************/


errno_t  bomxButtonEvent (

#    if PROTOTYPES
        ComxChannel  channel,
        ButtonEvent  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ButtonEvent  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->type = (ButtonType) enumeration ;
  }
    CHECK (comxLong (channel, NULL_OR (value, time))) ;
    CHECK (comxDouble (channel, NULL_OR (value, x))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y))) ;
    CHECK (comxDouble (channel, NULL_OR (value, x_root))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y_root))) ;
    CHECK (comxShort (channel, NULL_OR (value, button))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxKeyEvent() - decode/encode/erase a CORBA KeyEvent structure.

*******************************************************************************/


errno_t  bomxKeyEvent (

#    if PROTOTYPES
        ComxChannel  channel,
        KeyEvent  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        KeyEvent  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->type = (KeyType) enumeration ;
  }
    CHECK (comxLong (channel, NULL_OR (value, time))) ;
    CHECK (comxShort (channel, NULL_OR (value, state))) ;
    CHECK (comxShort (channel, NULL_OR (value, keyval))) ;
    CHECK (comxShort (channel, NULL_OR (value, length))) ;
    CHECK (comxString (channel, NULL_OR (value, str))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxCrossingEvent() - decode/encode/erase a CORBA CrossingEvent structure.

*******************************************************************************/


errno_t  bomxCrossingEvent (

#    if PROTOTYPES
        ComxChannel  channel,
        CrossingEvent  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        CrossingEvent  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->type ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->type = (CrossType) enumeration ;
  }
    CHECK (comxLong (channel, NULL_OR (value, time))) ;
    CHECK (comxDouble (channel, NULL_OR (value, x))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y))) ;
    CHECK (comxDouble (channel, NULL_OR (value, x_root))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y_root))) ;
  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->mode ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->mode = (CrossMode) enumeration ;
  }
    CHECK (comxBoolean (channel, NULL_OR (value, focus))) ;
    CHECK (comxShort (channel, NULL_OR (value, state))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxFocusEvent() - decode/encode/erase a CORBA FocusEvent structure.

*******************************************************************************/


errno_t  bomxFocusEvent (

#    if PROTOTYPES
        ComxChannel  channel,
        FocusEvent  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        FocusEvent  *value ;
#    endif

{

    CHECK (comxBoolean (channel, NULL_OR (value, inside))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxEvent() - decode/encode/erase a CORBA Event structure.

*******************************************************************************/


errno_t  bomxEvent (

#    if PROTOTYPES
        ComxChannel  channel,
        Event  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Event  *value ;
#    endif

{

  { unsigned  long  enumeration ;
    if (value != NULL)  enumeration = value->which ;
    CHECK (comxEnum (channel, &enumeration)) ;
    if (value != NULL)  value->which = (EventType) enumeration ;
  }

    switch (value->which) {
    case FOCUS:
        CHECK (bomxFocusEvent (channel, NULL_OR (value, data.focus))) ;
        break ;
    case KEY:
        CHECK (bomxKeyEvent (channel, NULL_OR (value, data.key))) ;
        break ;
    case MOTION:
        CHECK (bomxMotionEvent (channel, NULL_OR (value, data.motion))) ;
        break ;
    case BUTTON:
        CHECK (bomxButtonEvent (channel, NULL_OR (value, data.button))) ;
        break ;
    case CROSSING:
        CHECK (bomxCrossingEvent (channel, NULL_OR (value, data.crossing))) ;
        break ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(bomxEvent) Invalid EventType: %lu\n", (unsigned long) value->which) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

    bomxRequisition() - decode/encode/erase a CORBA Requisition structure.

*******************************************************************************/


errno_t  bomxRequisition (

#    if PROTOTYPES
        ComxChannel  channel,
        Requisition  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Requisition  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, width))) ;
    CHECK (comxLong (channel, NULL_OR (value, height))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxPair() - decode/encode/erase a CORBA Pair structure.

*******************************************************************************/


errno_t  bomxPair (

#    if PROTOTYPES
        ComxChannel  channel,
        Pair  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Pair  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, name))) ;
    CHECK (gimxAny (channel, NULL_OR (value, value))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxBonoboPropertySet() - decode/encode/erase a sequence of CORBA Pair structures.

*******************************************************************************/


errno_t  bomxBonoboPropertySet (

#    if PROTOTYPES
        ComxChannel  channel,
        BonoboPropertySet  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        BonoboPropertySet  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) bomxPair, sizeof (Pair))) ;

}

/*!*****************************************************************************

    bomxZoomLevelList() - decode/encode/erase a sequence of CORBA ZoomLevel structures.

*******************************************************************************/


errno_t  bomxZoomLevelList (

#    if PROTOTYPES
        ComxChannel  channel,
        ZoomLevelList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ZoomLevelList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxFloat, sizeof (float))) ;

}

/*!*****************************************************************************

    bomxZoomLevelNameList() - decode/encode/erase a sequence of CORBA ZoomLevelName structures.

*******************************************************************************/


errno_t  bomxZoomLevelNameList (

#    if PROTOTYPES
        ComxChannel  channel,
        ZoomLevelNameList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ZoomLevelNameList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxString, sizeof (char *))) ;

}

/*!*****************************************************************************

    bomxIRect() - decode/encode/erase a CORBA IRect structure.

*******************************************************************************/


errno_t  bomxIRect (

#    if PROTOTYPES
        ComxChannel  channel,
        IRect  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        IRect  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, x0))) ;
    CHECK (comxLong (channel, NULL_OR (value, y0))) ;
    CHECK (comxLong (channel, NULL_OR (value, x1))) ;
    CHECK (comxLong (channel, NULL_OR (value, y1))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxDRect() - decode/encode/erase a CORBA DRect structure.

*******************************************************************************/


errno_t  bomxDRect (

#    if PROTOTYPES
        ComxChannel  channel,
        DRect  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        DRect  *value ;
#    endif

{

    CHECK (comxDouble (channel, NULL_OR (value, x0))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y0))) ;
    CHECK (comxDouble (channel, NULL_OR (value, x1))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y1))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxBuf() - decode/encode/erase a CORBA Buf structure.

*******************************************************************************/


errno_t  bomxBuf (

#    if PROTOTYPES
        ComxChannel  channel,
        Buf  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Buf  *value ;
#    endif

{

    CHECK (comxOctetSeq (channel, NULL_OR (value, rgb_buf))) ;
    CHECK (comxLong (channel, NULL_OR (value, row_stride))) ;
    CHECK (bomxIRect (channel, NULL_OR (value, rect))) ;
    CHECK (comxLong (channel, NULL_OR (value, bg_color))) ;
    CHECK (comxShort (channel, NULL_OR (value, flags))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxPoint() - decode/encode/erase a CORBA Point structure.

*******************************************************************************/


errno_t  bomxPoint (

#    if PROTOTYPES
        ComxChannel  channel,
        Point  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Point  *value ;
#    endif

{

    CHECK (comxDouble (channel, NULL_OR (value, x))) ;
    CHECK (comxDouble (channel, NULL_OR (value, y))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxPoints() - decode/encode/erase a sequence of CORBA Point structures.

*******************************************************************************/


errno_t  bomxPoints (

#    if PROTOTYPES
        ComxChannel  channel,
        Points  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Points  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) bomxPoint, sizeof (Point))) ;

}

/*!*****************************************************************************

    bomxSVPSegment() - decode/encode/erase a CORBA SVPSegment structure.

*******************************************************************************/


errno_t  bomxSVPSegment (

#    if PROTOTYPES
        ComxChannel  channel,
        SVPSegment  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SVPSegment  *value ;
#    endif

{

    CHECK (comxBoolean (channel, NULL_OR (value, up))) ;
    CHECK (bomxDRect (channel, NULL_OR (value, bbox))) ;
    CHECK (bomxPoints (channel, NULL_OR (value, points))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxSVP() - decode/encode/erase a sequence of CORBA SVPSegment structures.

*******************************************************************************/


errno_t  bomxSVP (

#    if PROTOTYPES
        ComxChannel  channel,
        SVP  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SVP  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) bomxSVPSegment, sizeof (SVPSegment))) ;

}

/*!*****************************************************************************

    bomxArtUTA() - decode/encode/erase a CORBA ArtUTA structure.

*******************************************************************************/


errno_t  bomxArtUTA (

#    if PROTOTYPES
        ComxChannel  channel,
        ArtUTA  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ArtUTA  *value ;
#    endif

{

    CHECK (comxShort (channel, NULL_OR (value, x0))) ;
    CHECK (comxShort (channel, NULL_OR (value, y0))) ;
    CHECK (comxShort (channel, NULL_OR (value, width))) ;
    CHECK (comxShort (channel, NULL_OR (value, height))) ;
    CHECK (comxSequence (channel, NULL_OR (value, utiles), (ComxFunc) comxLong, sizeof (long))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxPrintScissor() - decode/encode/erase a CORBA PrintScissor structure.

*******************************************************************************/


errno_t  bomxPrintScissor (

#    if PROTOTYPES
        ComxChannel  channel,
        PrintScissor  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        PrintScissor  *value ;
#    endif

{

    CHECK (comxDouble (channel, NULL_OR (value, width_first_page))) ;
    CHECK (comxDouble (channel, NULL_OR (value, width_per_page))) ;
    CHECK (comxDouble (channel, NULL_OR (value, height_first_page))) ;
    CHECK (comxDouble (channel, NULL_OR (value, height_per_page))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxPrintDimensions() - decode/encode/erase a CORBA PrintDimensions structure.

*******************************************************************************/


errno_t  bomxPrintDimensions (

#    if PROTOTYPES
        ComxChannel  channel,
        PrintDimensions  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        PrintDimensions  *value ;
#    endif

{

    CHECK (comxDouble (channel, NULL_OR (value, width))) ;
    CHECK (comxDouble (channel, NULL_OR (value, height))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxArgList() - decode/encode/erase a sequence of CORBA Any structures.

*******************************************************************************/


errno_t  bomxArgList (

#    if PROTOTYPES
        ComxChannel  channel,
        ArgList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ArgList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxAny, sizeof (Any))) ;

}

/*!*****************************************************************************

    bomxMessageDesc() - decode/encode/erase a CORBA MessageDesc structure.

*******************************************************************************/


errno_t  bomxMessageDesc (

#    if PROTOTYPES
        ComxChannel  channel,
        MessageDesc  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        MessageDesc  *value ;
#    endif

{

    CHECK (comxString (channel, NULL_OR (value, name))) ;
    CHECK (comxSequence (channel, NULL_OR (value, types), (ComxFunc) gimxIOR, sizeof (IOR))) ;
    CHECK (gimxIOR (channel, NULL_OR (value, return_type))) ;
    CHECK (comxString (channel, NULL_OR (value, description))) ;

    return (0) ;

}

/*!*****************************************************************************

    bomxMessageList() - decode/encode/erase a sequence of CORBA MessageDesc structures.

*******************************************************************************/


errno_t  bomxMessageList (

#    if PROTOTYPES
        ComxChannel  channel,
        MessageList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        MessageList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) bomxMessageDesc, sizeof (MessageDesc))) ;

}

/*!*****************************************************************************

    bomxObjectDirectoryList() - decode/encode/erase a sequence of CORBA ObjectDirectory structures.

*******************************************************************************/


errno_t  bomxObjectDirectoryList (

#    if PROTOTYPES
        ComxChannel  channel,
        ObjectDirectoryList  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ObjectDirectoryList  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxIOR, sizeof (IOR))) ;

}

/*!*****************************************************************************

    bomxServerInfoListCache() - decode/encode/erase a CORBA ServerInfoListCache structure.

*******************************************************************************/


errno_t  bomxServerInfoListCache (

#    if PROTOTYPES
        ComxChannel  channel,
        ServerInfoListCache  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ServerInfoListCache  *value ;
#    endif

{

    CHECK (comxBoolean (channel, NULL_OR (value, which))) ;

    switch (value->which) {
    case TRUE:
        CHECK (bomxServerInfoList (channel, NULL_OR (value, data.server_list))) ;
        break ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(bomxServerInfoListCache) Invalid switch: %lu\n", (unsigned long) value->which) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

    bomxServerStateCache() - decode/encode/erase a CORBA ServerStateCache structure.

*******************************************************************************/


errno_t  bomxServerStateCache (

#    if PROTOTYPES
        ComxChannel  channel,
        ServerStateCache  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ServerStateCache  *value ;
#    endif

{

    CHECK (comxBoolean (channel, NULL_OR (value, which))) ;

    switch (value->which) {
    case TRUE:
        CHECK (comxSequence (channel, NULL_OR (value, data.active_servers), (ComxFunc) comxString, sizeof (char *))) ;
        break ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(bomxServerStateCache) Invalid switch: %lu\n", (unsigned long) value->which) ;
        return (errno) ;
    }

    return (0) ;

}

/*******************************************************************************
    Lookup Tables - for converting named constants to numbers and vice-versa;
        see the coliToName() and coliToNumber() functions.
*******************************************************************************/


const  ColiMap  ActivationPropertyTypeLUT[] = {
  { (long) ACTIVATION_P_STRING, "ACTIVATION_P_STRING" },
  { (long) ACTIVATION_P_NUMBER, "ACTIVATION_P_NUMBER" },
  { (long) ACTIVATION_P_BOOLEAN, "ACTIVATION_P_BOOLEAN" },
  { (long) ACTIVATION_P_STRINGV, "ACTIVATION_P_STRINGV" },
  { 0L, NULL }
} ;

const  ColiMap  ActivationResultTypeLUT[] = {
  { (long) ACTIVATION_RESULT_OBJECT, "ACTIVATION_RESULT_OBJECT" },
  { (long) ACTIVATION_RESULT_SHLIB, "ACTIVATION_RESULT_SHLIB" },
  { (long) ACTIVATION_RESULT_NONE, "ACTIVATION_RESULT_NONE" },
  { 0L, NULL }
} ;

const  ColiMap  ButtonTypeLUT[] = {
  { (long) BUTTON_PRESS, "BUTTON_PRESS" },
  { (long) BUTTON_2_PRESS, "BUTTON_2_PRESS" },
  { (long) BUTTON_3_PRESS, "BUTTON_3_PRESS" },
  { (long) BUTTON_RELEASE, "BUTTON_RELEASE" },
  { 0L, NULL }
} ;

const  ColiMap  CrossModeLUT[] = {
  { (long) GDK_NORMAL, "GDK_NORMAL" },
  { (long) GDK_GRAB, "GDK_GRAB" },
  { (long) GDK_UNGRAB, "GDK_UNGRAB" },
  { 0L, NULL }
} ;

const  ColiMap  CrossTypeLUT[] = {
  { (long) ENTER, "ENTER" },
  { (long) LEAVE, "LEAVE" },
  { 0L, NULL }
} ;

const  ColiMap  DBFlagsLUT[] = {
  { (long) DBF_DEFAULT, "DBF_DEFAULT" },
  { (long) DBF_WRITE, "DBF_WRITE" },
  { (long) DBF_MANDATORY, "DBF_MANDATORY" },
  { 0L, NULL }
} ;

const  ColiMap  DirectionLUT[] = {
  { (long) DirectionTabForward, "DirectionTabForward" },
  { (long) DirectionTabBackward, "DirectionTabBackward" },
  { (long) DirectionUp, "DirectionUp" },
  { (long) DirectionDown, "DirectionDown" },
  { (long) DirectionLeft, "DirectionLeft" },
  { (long) DirectionRight, "DirectionRight" },
  { 0L, NULL }
} ;

const  ColiMap  DynamicPathLoadResultLUT[] = {
  { (long) DYNAMIC_LOAD_SUCCESS, "DYNAMIC_LOAD_SUCCESS" },
  { (long) DYNAMIC_LOAD_ERROR, "DYNAMIC_LOAD_ERROR" },
  { (long) DYNAMIC_LOAD_NOT_LISTED, "DYNAMIC_LOAD_NOT_LISTED" },
  { (long) DYNAMIC_LOAD_ALREADY_LISTED, "DYNAMIC_LOAD_ALREADY_LISTED" },
  { 0L, NULL }
} ;

const  ColiMap  EventTypeLUT[] = {
  { (long) FOCUS, "FOCUS" },
  { (long) KEY, "KEY" },
  { (long) MOTION, "MOTION" },
  { (long) BUTTON, "BUTTON" },
  { (long) CROSSING, "CROSSING" },
  { 0L, NULL }
} ;

const  ColiMap  KeyTypeLUT[] = {
  { (long) KEY_PRESS, "KEY_PRESS" },
  { (long) KEY_RELEASE, "KEY_RELEASE" },
  { 0L, NULL }
} ;

const  ColiMap  RegistrationResultLUT[] = {
  { (long) ACTIVATION_REG_SUCCESS, "ACTIVATION_REG_SUCCESS" },
  { (long) ACTIVATION_REG_NOT_LISTED, "ACTIVATION_REG_NOT_LISTED" },
  { (long) ACTIVATION_REG_ALREADY_ACTIVE, "ACTIVATION_REG_ALREADY_ACTIVE" },
  { (long) ACTIVATION_REG_ERROR, "ACTIVATION_REG_ERROR" },
  { 0L, NULL }
} ;

const  ColiMap  SeekTypeLUT[] = {
  { (long) SeekSet, "SeekSet" },
  { (long) SeekCur, "SeekCur" },
  { (long) SeekEnd, "SeekEnd" },
  { 0L, NULL }
} ;

const  ColiMap  StateLUT[] = {
  { (long) StateNormal, "StateNormal" },
  { (long) StateActive, "StateActive" },
  { (long) StatePrelight, "StatePrelight" },
  { (long) StateSelected, "StateSelected" },
  { (long) StateInsensitive, "StateInsensitive" },
  { 0L, NULL }
} ;

const  ColiMap  StorageTypeLUT[] = {
  { (long) STORAGE_TYPE_REGULAR, "STORAGE_TYPE_REGULAR" },
  { (long) STORAGE_TYPE_DIRECTORY, "STORAGE_TYPE_DIRECTORY" },
  { 0L, NULL }
} ;
