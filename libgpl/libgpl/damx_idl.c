/* $Id: damx_idl.c,v 1.3 2004/12/30 21:50:19 alex Exp $ */
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

    damxURISequence() - decode/encode/erase a sequence of CORBA URI structures.

*******************************************************************************/


errno_t  damxURISequence (

#    if PROTOTYPES
        ComxChannel  channel,
        URISequence  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        URISequence  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) comxString, sizeof (char *))) ;

}

/*!*****************************************************************************

    damxResourceID() - decode/encode/erase a CORBA ResourceID structure.

*******************************************************************************/


errno_t  damxResourceID (

#    if PROTOTYPES
        ComxChannel  channel,
        ResourceID  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ResourceID  *value ;
#    endif

{

    CHECK (comxULongLong (channel, NULL_OR (value, container))) ;
    CHECK (comxULongLong (channel, NULL_OR (value, fragment))) ;

    return (0) ;

}

/*!*****************************************************************************

    damxResourceIDSequence() - decode/encode/erase a sequence of CORBA ResourceID structures.

*******************************************************************************/


errno_t  damxResourceIDSequence (

#    if PROTOTYPES
        ComxChannel  channel,
        ResourceIDSequence  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ResourceIDSequence  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) damxResourceID, sizeof (ResourceID))) ;

}

/*!*****************************************************************************

    damxResourceChangeEvent() - decode/encode/erase a CORBA ResourceChangeEvent structure.

*******************************************************************************/


errno_t  damxResourceChangeEvent (

#    if PROTOTYPES
        ComxChannel  channel,
        ResourceChangeEvent  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ResourceChangeEvent  *value ;
#    endif

{

    CHECK (damxResourceIDSequence (channel, NULL_OR (value, affected))) ;

    return (0) ;

}

/*!*****************************************************************************

    damxComplex() - decode/encode/erase a CORBA Complex structure.

*******************************************************************************/


errno_t  damxComplex (

#    if PROTOTYPES
        ComxChannel  channel,
        Complex  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Complex  *value ;
#    endif

{

    CHECK (comxDouble (channel, NULL_OR (value, real))) ;
    CHECK (comxDouble (channel, NULL_OR (value, imaginary))) ;

    return (0) ;

}

/*!*****************************************************************************

    damxSimpleValue() - decode/encode/erase a CORBA SimpleValue structure.

*******************************************************************************/


errno_t  damxSimpleValue (

#    if PROTOTYPES
        ComxChannel  channel,
        SimpleValue  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        SimpleValue  *value ;
#    endif

{

    CHECK (comxShort (channel, NULL_OR (value, which))) ;

    switch (value->which) {
    case RESOURCE_TYPE:
        CHECK (damxResourceID (channel, NULL_OR (value, data.resource_value))) ;
        break ;
    case URI_TYPE:
        CHECK (comxString (channel, NULL_OR (value, data.uri_value))) ;
        break ;
    case STRING_TYPE:
        CHECK (comxString (channel, NULL_OR (value, data.string_value))) ;
        break ;
    case BOOLEAN_TYPE:
        CHECK (comxBoolean (channel, NULL_OR (value, data.boolean_value))) ;
        break ;
    case INT_TYPE:
        CHECK (comxLong (channel, NULL_OR (value, data.int_value))) ;
        break ;
    case UNSIGNED_TYPE:
        CHECK (comxULong (channel, NULL_OR (value, data.unsigned_value))) ;
        break ;
    case DOUBLE_TYPE:
        CHECK (comxDouble (channel, NULL_OR (value, data.double_value))) ;
        break ;
    case COMPLEX_TYPE:
        CHECK (damxComplex (channel, NULL_OR (value, data.complex_value))) ;
        break ;
    case DATE_TIME_TYPE:
        CHECK (comxULongLong (channel, NULL_OR (value, data.date_time_value))) ;
        break ;
    case ULONG_LONG_TYPE:
        CHECK (comxULongLong (channel, NULL_OR (value, data.ulong_long_value))) ;
        break ;
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(damxSimpleValue) Invalid switch: %lu\n", (unsigned long) value->which) ;
        return (errno) ;
    }

    return (0) ;

}

/*!*****************************************************************************

    damxPropertyValueSequence() - decode/encode/erase a sequence of CORBA PropertyValue structures.

*******************************************************************************/


errno_t  damxPropertyValueSequence (

#    if PROTOTYPES
        ComxChannel  channel,
        PropertyValueSequence  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        PropertyValueSequence  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxAny, sizeof (Any))) ;

}

/*!*****************************************************************************

    damxResourceDescription() - decode/encode/erase a CORBA ResourceDescription structure.

*******************************************************************************/


errno_t  damxResourceDescription (

#    if PROTOTYPES
        ComxChannel  channel,
        ResourceDescription  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ResourceDescription  *value ;
#    endif

{

    CHECK (damxResourceID (channel, NULL_OR (value, id))) ;
    CHECK (damxPropertyValueSequence (channel, NULL_OR (value, values))) ;

    return (0) ;

}

/*!*****************************************************************************

    damxResourceDescriptionSequence() - decode/encode/erase a sequence of CORBA ResourceDescription structures.

*******************************************************************************/


errno_t  damxResourceDescriptionSequence (

#    if PROTOTYPES
        ComxChannel  channel,
        ResourceDescriptionSequence  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ResourceDescriptionSequence  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) damxResourceDescription, sizeof (ResourceDescription))) ;

}

/*!*****************************************************************************

    damxAssociation() - decode/encode/erase a CORBA Association structure.

*******************************************************************************/


errno_t  damxAssociation (

#    if PROTOTYPES
        ComxChannel  channel,
        Association  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Association  *value ;
#    endif

{

    CHECK (damxResourceID (channel, NULL_OR (value, property))) ;
    CHECK (damxResourceID (channel, NULL_OR (value, type))) ;
    CHECK (comxBoolean (channel, NULL_OR (value, inverse))) ;

    return (0) ;

}

/*!*****************************************************************************

    damxAssociationSequence() - decode/encode/erase a sequence of CORBA Association structures.

*******************************************************************************/


errno_t  damxAssociationSequence (

#    if PROTOTYPES
        ComxChannel  channel,
        AssociationSequence  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        AssociationSequence  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) damxAssociation, sizeof (Association))) ;

}
