/* $Id: damx_idl.h,v 1.8 2009/09/09 23:23:11 alex Exp $ */
/*******************************************************************************
    damx_all.idl
*******************************************************************************/

/* Module: DAFIdentifiers */

typedef  char  *URI ;

typedef  SEQUENCE (URI, URISequence) ;

typedef  struct  ResourceID {
    ULONGLONG  container ;
    ULONGLONG  fragment ;
}  ResourceID ;

typedef  SEQUENCE (ResourceID, ResourceIDSequence) ;

typedef  IOR  ResourceIDService ;

/* Module: CosEventComm */

/* Module: CosEventChannelAdmin */

/* Module: DAFEvents */

typedef  struct  ResourceChangeEvent {
    ResourceIDSequence  affected ;
}  ResourceChangeEvent ;

typedef  IOR  ResourceEventSource ;

/* Module: TimeBase */

/*===== "TimeT" already defined =====*/

/*===== "InaccuracyT" already defined =====*/

/*===== "TdfT" already defined =====*/

/* Module: DAFDescriptions */

typedef  TimeT  DateTime ;

typedef  struct  Complex {
    double  real ;
    double  imaginary ;
}  Complex ;

typedef  short  SimpleValueType ;

#define  RESOURCE_TYPE  (1)
#define  URI_TYPE  (2)
#define  STRING_TYPE  (3)
#define  BOOLEAN_TYPE  (4)
#define  INT_TYPE  (5)
#define  UNSIGNED_TYPE  (6)
#define  DOUBLE_TYPE  (7)
#define  COMPLEX_TYPE  (8)
#define  DATE_TIME_TYPE  (9)
#define  ULONG_LONG_TYPE  (10)

typedef  struct  SimpleValue {
    SimpleValueType  which ;
    union {
			/* RESOURCE_TYPE */
        ResourceID  resource_value ;
			/* URI_TYPE */
        URI  uri_value ;
			/* STRING_TYPE */
        char  *string_value ;
			/* BOOLEAN_TYPE */
        bool  boolean_value ;
			/* INT_TYPE */
        long  int_value ;
			/* UNSIGNED_TYPE */
        unsigned  long  unsigned_value ;
			/* DOUBLE_TYPE */
        double  double_value ;
			/* COMPLEX_TYPE */
        Complex  complex_value ;
			/* DATE_TIME_TYPE */
        DateTime  date_time_value ;
			/* ULONG_LONG_TYPE */
        ULONGLONG  ulong_long_value ;
    }  data ;
}  SimpleValue ;

typedef  ResourceID  PropertyID ;

typedef  SEQUENCE (PropertyValue, PropertyValueSequence) ;

typedef  struct  ResourceDescription {
    ResourceID  id ;
    PropertyValueSequence  values ;
}  ResourceDescription ;

typedef  SEQUENCE (ResourceDescription, ResourceDescriptionSequence) ;

typedef  IOR  ResourceDescriptionIterator ;

/* Module: DAFQuery */

typedef  ResourceID  ClassID ;

/*===== "PropertyID" already defined =====*/

typedef  ResourceIDSequence  PropertySequence ;

typedef  struct  Association {
    PropertyID  property ;
    ClassID  type ;
    bool  inverse ;
}  Association ;

typedef  SEQUENCE (Association, AssociationSequence) ;

typedef  IOR  ResourceQueryService ;

/*******************************************************************************
    Public functions.
*******************************************************************************/

/* Marshaling functions for the defined data types. */

extern  errno_t  damxAssociation P_((ComxChannel channel,
                                     Association *value))
    OCD ("DAFQuery") ;

extern  errno_t  damxAssociationSequence P_((ComxChannel channel,
                                             AssociationSequence *value))
    OCD ("DAFQuery") ;

extern  errno_t  damxComplex P_((ComxChannel channel,
                                 Complex *value))
    OCD ("DAFDescr") ;

extern  errno_t  damxPropertyValueSequence P_((ComxChannel channel,
                                               PropertyValueSequence *value))
    OCD ("DAFDescr") ;

extern  errno_t  damxResourceChangeEvent P_((ComxChannel channel,
                                             ResourceChangeEvent *value))
    OCD ("DAFEvent") ;

extern  errno_t  damxResourceDescription P_((ComxChannel channel,
                                             ResourceDescription *value))
    OCD ("DAFDescr") ;

extern  errno_t  damxResourceDescriptionSequence P_((ComxChannel channel,
                                                     ResourceDescriptionSequence *value))
    OCD ("DAFDescr") ;

extern  errno_t  damxResourceID P_((ComxChannel channel,
                                    ResourceID *value))
    OCD ("DAFIdent") ;

extern  errno_t  damxResourceIDSequence P_((ComxChannel channel,
                                            ResourceIDSequence *value))
    OCD ("DAFIdent") ;

extern  errno_t  damxSimpleValue P_((ComxChannel channel,
                                     SimpleValue *value))
    OCD ("DAFDescr") ;

extern  errno_t  damxURISequence P_((ComxChannel channel,
                                     URISequence *value))
    OCD ("DAFIdent") ;
