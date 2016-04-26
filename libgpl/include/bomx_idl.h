/* $Id: bomx_idl.h,v 1.2 2011/07/18 17:55:53 alex Exp $ */
/*******************************************************************************
    ../idl/GNOME/bonobo-2.0/Bonobo.idl
*******************************************************************************/

/* Module: Bonobo */

typedef  IOR  Unknown ;

/* Module: Bonobo */

typedef  StringSeq  StringList ;

/* Module: Bonobo::Activation */

typedef  char  *ImplementationID ;

typedef  char  *ActivationID ;

typedef  long  ActivationFlags ;

#define  ACTIVATION_FLAG_NO_LOCAL  (1 << 0)
#define  ACTIVATION_FLAG_PRIVATE  (1 << 1)
#define  ACTIVATION_FLAG_EXISTING_ONLY  (1 << 2)

typedef  long  RegistrationFlags ;

#define  REGISTRATION_FLAG_NO_SERVERINFO  (1 << 0)
#define  ACTIVATION_ENV_FLAG_UNSET  (1 << 0L)

typedef  struct  ActivationEnvValue {
    char  *name ;
    char  *value ;
    long  flags ;
}  ActivationEnvValue ;

typedef  SEQUENCE (ActivationEnvValue, ActivationEnvironment) ;

typedef  enum  ActivationResultType {
    ACTIVATION_RESULT_OBJECT,
    ACTIVATION_RESULT_SHLIB,
    ACTIVATION_RESULT_NONE
}  ActivationResultType ;

typedef  struct  ActivationResultData {
    ActivationResultType  which ;
    union {
			/* ACTIVATION_RESULT_OBJECT */
        Object  res_object ;
			/* ACTIVATION_RESULT_SHLIB */
        StringList  res_shlib ;
    }  data ;
}  ActivationResultData ;

typedef  struct  ActivationResult {
    char  *aid ;
    ActivationResultData  res ;
}  ActivationResult ;

typedef  enum  ActivationPropertyType {
    ACTIVATION_P_STRING,
    ACTIVATION_P_NUMBER,
    ACTIVATION_P_BOOLEAN,
    ACTIVATION_P_STRINGV
}  ActivationPropertyType ;

typedef  struct  ActivationPropertyValue {
    ActivationPropertyType  which ;
    union {
			/* ACTIVATION_P_STRING */
        char  *value_string ;
			/* ACTIVATION_P_NUMBER */
        double  value_number ;
			/* ACTIVATION_P_BOOLEAN */
        bool  value_boolean ;
			/* ACTIVATION_P_STRINGV */
        StringList  value_stringv ;
    }  data ;
}  ActivationPropertyValue ;

typedef  struct  ActivationProperty {
    char  *name ;
    ActivationPropertyValue  v ;
}  ActivationProperty ;

typedef  struct  ServerInfo {
    ImplementationID  iid ;
    char  *server_type ;
    char  *location_info ;
    char  *username ;
    char  *hostname ;
    char  *domain ;
    SEQUENCE (ActivationProperty, props) ;
}  ServerInfo ;

typedef  SEQUENCE (ServerInfo, ServerInfoList) ;

typedef  enum  RegistrationResult {
    ACTIVATION_REG_SUCCESS,
    ACTIVATION_REG_NOT_LISTED,
    ACTIVATION_REG_ALREADY_ACTIVE,
    ACTIVATION_REG_ERROR
}  RegistrationResult ;

typedef  enum  DynamicPathLoadResult {
    DYNAMIC_LOAD_SUCCESS,
    DYNAMIC_LOAD_ERROR,
    DYNAMIC_LOAD_NOT_LISTED,
    DYNAMIC_LOAD_ALREADY_LISTED
}  DynamicPathLoadResult ;

/* Module: Bonobo */

/* Module: Bonobo */

/* Module: Bonobo */

typedef  long  ResolveFlag ;

#define  MONIKER_ALLOW_USER_INTERACTION  (1)

typedef  struct  ResolveOptions {
    ResolveFlag  flags ;
    long  timeout ;
}  ResolveOptions ;

typedef  IOR  Moniker ;
typedef  IOR  MonikerExtender ;

/* Module: Bonobo */

typedef  long  StorageInfoFields ;

#define  FIELD_CONTENT_TYPE  (1)
#define  FIELD_SIZE  (2)
#define  FIELD_TYPE  (4)

typedef  char  *ContentType ;

typedef  enum  StorageType {
    STORAGE_TYPE_REGULAR,
    STORAGE_TYPE_DIRECTORY
}  StorageType ;

typedef  struct  StorageInfo {
    char  *name ;
    StorageType  type ;
    ContentType  content_type ;
    long  size ;
}  StorageInfo ;

typedef  OctetSeq  iobuf ;

typedef  enum  SeekType {
    SeekSet,
    SeekCur,
    SeekEnd
}  SeekType ;

typedef  IOR  Storage ;

typedef  SEQUENCE (StorageInfo, DirectoryList) ;

typedef  long  OpenMode ;

#define  BONOBO_READ  (1)
#define  BONOBO_WRITE  (2)
#define  BONOBO_CREATE  (4)
#define  BONOBO_FAILIFEXIST  (8)
#define  BONOBO_COMPRESSED  (16)
#define  BONOBO_TRANSACTED  (32)

/* Module: Bonobo */

typedef  IOR  MonikerContext ;
typedef  IOR  RunningContext ;

/* Module: Bonobo */

typedef  IOR  Persist ;

/*===== "ContentType" already defined =====*/

typedef  SEQUENCE (ContentType, ContentTypeList) ;

typedef  char  *IID ;

typedef  IOR  PersistFile ;
typedef  IOR  PersistStorage ;
typedef  IOR  PersistStream ;

/* Module: Bonobo */

typedef  IOR  Listener ;
typedef  IOR  EventSource ;

/* Module: Bonobo */

/* Module: Bonobo::Gdk */

typedef  enum  EventType {
    FOCUS,
    KEY,
    MOTION,
    BUTTON,
    CROSSING
}  EventType ;

typedef  enum  ButtonType {
    BUTTON_PRESS,
    BUTTON_2_PRESS,
    BUTTON_3_PRESS,
    BUTTON_RELEASE
}  ButtonType ;

typedef  enum  KeyType {
    KEY_PRESS,
    KEY_RELEASE
}  KeyType ;

typedef  enum  CrossType {
    ENTER,
    LEAVE
}  CrossType ;

typedef  enum  CrossMode {
    GDK_NORMAL,
    GDK_GRAB,
    GDK_UNGRAB
}  CrossMode ;

typedef  long  GdkTime ;

typedef  struct  MotionEvent {
    GdkTime  time ;
    double  x ;
    double  y ;
    double  x_root ;
    double  y_root ;
    double  pressure ;
    double  xtilt ;
    double  ytilt ;
    long  state ;
    bool  is_hint ;
}  MotionEvent ;

typedef  struct  ButtonEvent {
    ButtonType  type ;
    GdkTime  time ;
    double  x ;
    double  y ;
    double  x_root ;
    double  y_root ;
    short  button ;
}  ButtonEvent ;

typedef  struct  KeyEvent {
    KeyType  type ;
    GdkTime  time ;
    short  state ;
    short  keyval ;
    short  length ;
    char  *str ;
}  KeyEvent ;

typedef  struct  CrossingEvent {
    CrossType  type ;
    GdkTime  time ;
    double  x ;
    double  y ;
    double  x_root ;
    double  y_root ;
    CrossMode  mode ;
    bool  focus ;
    short  state ;
}  CrossingEvent ;

typedef  struct  FocusEvent {
    bool  inside ;
}  FocusEvent ;

typedef  struct  Event {
    EventType  which ;
    union {
			/* FOCUS */
        FocusEvent  focus ;
			/* KEY */
        KeyEvent  key ;
			/* MOTION */
        MotionEvent  motion ;
			/* BUTTON */
        ButtonEvent  button ;
			/* CROSSING */
        CrossingEvent  crossing ;
    }  data ;
}  Event ;

typedef  char  *WindowId ;

/* Module: Bonobo::Gtk */

typedef  struct  Requisition {
    long  width ;
    long  height ;
}  Requisition ;

typedef  enum  State {
    StateNormal,
    StateActive,
    StatePrelight,
    StateSelected,
    StateInsensitive
}  State ;

typedef  enum  Direction {
    DirectionTabForward,
    DirectionTabBackward,
    DirectionUp,
    DirectionDown,
    DirectionLeft,
    DirectionRight
}  Direction ;

/* Module: Bonobo */

typedef  IOR  UIContainer ;
typedef  IOR  UIComponent ;

/* Module: Bonobo */

typedef  struct  Pair {
    char  *name ;
    Any  value ;
}  Pair ;

typedef  SEQUENCE (Pair, BonoboPropertySet) ;

typedef  StringSeq  KeyList ;

typedef  long  PropertyFlags ;

#define  PROPERTY_READABLE  (1)
#define  PROPERTY_WRITEABLE  (2)
#define  PROPERTY_NO_LISTENING  (4)
#define  PROPERTY_NO_AUTONOTIFY  (8)
#define  PROPERTY_NO_PERSIST  (16)

typedef  IOR  PropertyBag ;
typedef  IOR  ConfigDatabase ;

typedef  enum  DBFlags {
    DBF_DEFAULT,
    DBF_WRITE,
    DBF_MANDATORY
}  DBFlags ;

/* Module: Bonobo */

typedef  IOR  ControlFrame ;
typedef  IOR  PropertyControl ;

/* Module: Bonobo */

typedef  float  ZoomLevel ;

typedef  char  *ZoomLevelName ;

typedef  SEQUENCE (ZoomLevel, ZoomLevelList) ;

typedef  SEQUENCE (ZoomLevelName, ZoomLevelNameList) ;

typedef  IOR  ZoomableFrame ;
typedef  IOR  Zoomable ;

/* Module: Bonobo */

typedef  IOR  ItemContainer ;

typedef  StringSeq  ObjectNames ;

/* Module: Bonobo */

/* Module: Bonobo::Canvas */

typedef  OctetSeq  pixbuf ;

typedef  long  Int32 ;

typedef  struct  IRect {
    long  x0 ;
    long  y0 ;
    long  x1 ;
    long  y1 ;
}  IRect ;

typedef  struct  DRect {
    double  x0 ;
    double  y0 ;
    double  x1 ;
    double  y1 ;
}  DRect ;

#define  IS_BG  (1)
#define  IS_BUF  (2)

typedef  double  affine[6] ;

typedef  struct  Buf {
    pixbuf  rgb_buf ;
    long  row_stride ;
    IRect  rect ;
    Int32  bg_color ;
    short  flags ;
}  Buf ;

typedef  struct  Point {
    double  x ;
    double  y ;
}  Point ;

typedef  SEQUENCE (Point, Points) ;

typedef  struct  SVPSegment {
    bool  up ;
    DRect  bbox ;
    Points  points ;
}  SVPSegment ;

typedef  SEQUENCE (SVPSegment, SVP) ;

typedef  struct  ArtUTA {
    short  x0 ;
    short  y0 ;
    short  width ;
    short  height ;
    SEQUENCE (Int32, utiles) ;
}  ArtUTA ;

typedef  IOR  Component ;
typedef  IOR  ComponentProxy ;

/* Module: Bonobo */

typedef  IOR  ControlFactory ;
typedef  IOR  CanvasComponentFactory ;
typedef  IOR  Embeddable ;

/* Module: Bonobo */

typedef  struct  PrintScissor {
    double  width_first_page ;
    double  width_per_page ;
    double  height_first_page ;
    double  height_per_page ;
}  PrintScissor ;

typedef  struct  PrintDimensions {
    double  width ;
    double  height ;
}  PrintDimensions ;

typedef  IOR  Print ;

/* Module: Bonobo */

typedef  IOR  Clipboard ;
typedef  IOR  ClipboardStore ;

/* Module: Bonobo */

typedef  IOR  Application ;

typedef  SEQUENCE (Any, ArgList) ;

typedef  StringSeq  argv_t ;

typedef  struct  MessageDesc {
    char  *name ;
    SEQUENCE (TypeCode, types) ;
    TypeCode  return_type ;
    char  *description ;
}  MessageDesc ;

typedef  SEQUENCE (MessageDesc, MessageList) ;

/*******************************************************************************
    ../idl/GNOME/bonobo-activation-2.0/Bonobo_ObjectDirectory.idl
*******************************************************************************/

/* Module: Bonobo */

/* Module: Bonobo */

/*===== "StringList" already defined =====*/

/* Module: Bonobo::Activation */

/*===== "ImplementationID" already defined =====*/

/*===== "ActivationID" already defined =====*/

/*===== "ActivationFlags" already defined =====*/

/*===== "ACTIVATION_FLAG_NO_LOCAL" previously defined in Bonobo =====*/

/*===== "ACTIVATION_FLAG_PRIVATE" previously defined in Bonobo =====*/

/*===== "ACTIVATION_FLAG_EXISTING_ONLY" previously defined in Bonobo =====*/

/*===== "RegistrationFlags" already defined =====*/

/*===== "REGISTRATION_FLAG_NO_SERVERINFO" previously defined in Bonobo =====*/

/*===== "ACTIVATION_ENV_FLAG_UNSET" previously defined in Bonobo =====*/

/*===== "ActivationEnvValue" already defined =====*/

/*===== "ActivationEnvironment" already defined =====*/

/*===== "ActivationResultData" already defined =====*/

/* Module: Bonobo::Bonobo */

typedef  IOR  ObjectDirectory ;

/* Module: Bonobo::Bonobo */

typedef  IOR  ActivationContext ;
typedef  IOR  ActivationClient ;

/* Module: Bonobo::Bonobo */

typedef  SEQUENCE (ObjectDirectory, ObjectDirectoryList) ;

/* Module: Bonobo::Bonobo */

typedef  LONGLONG  CacheTime ;

typedef  struct  ServerInfoListCache {
    bool  which ;
    union {
			/* TRUE */
        ServerInfoList  server_list ;
    }  data ;
}  ServerInfoListCache ;

typedef  struct  ServerStateCache {
    bool  which ;
    union {
			/* TRUE */
        SEQUENCE (ImplementationID, active_servers) ;
    }  data ;
}  ServerStateCache ;

/*******************************************************************************
    Tables for mapping enumerated values to names and vice-versa;
    see the coliToName() and coliToNumber() functions.
*******************************************************************************/

extern  const  ColiMap  ActivationPropertyTypeLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  ActivationResultTypeLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  ButtonTypeLUT[]  OCD ("Gdk") ;
extern  const  ColiMap  CrossModeLUT[]  OCD ("Gdk") ;
extern  const  ColiMap  CrossTypeLUT[]  OCD ("Gdk") ;
extern  const  ColiMap  DBFlagsLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  DirectionLUT[]  OCD ("Gtk") ;
extern  const  ColiMap  DynamicPathLoadResultLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  EventTypeLUT[]  OCD ("Gdk") ;
extern  const  ColiMap  KeyTypeLUT[]  OCD ("Gdk") ;
extern  const  ColiMap  RegistrationResultLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  SeekTypeLUT[]  OCD ("Bonobo") ;
extern  const  ColiMap  StateLUT[]  OCD ("Gtk") ;
extern  const  ColiMap  StorageTypeLUT[]  OCD ("Bonobo") ;

/*******************************************************************************
    Public functions.
*******************************************************************************/

/* Marshaling functions for the defined data types. */

extern  errno_t  bomxActivationEnvironment P_((ComxChannel channel,
                                               ActivationEnvironment *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxActivationEnvValue P_((ComxChannel channel,
                                            ActivationEnvValue *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxActivationProperty P_((ComxChannel channel,
                                            ActivationProperty *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxActivationPropertyValue P_((ComxChannel channel,
                                                 ActivationPropertyValue *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxActivationResult P_((ComxChannel channel,
                                          ActivationResult *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxActivationResultData P_((ComxChannel channel,
                                              ActivationResultData *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxArgList P_((ComxChannel channel,
                                 ArgList *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxArtUTA P_((ComxChannel channel,
                                ArtUTA *value))
    OCD ("Canvas") ;

extern  errno_t  bomxBonoboPropertySet P_((ComxChannel channel,
                                           BonoboPropertySet *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxBuf P_((ComxChannel channel,
                             Buf *value))
    OCD ("Canvas") ;

extern  errno_t  bomxButtonEvent P_((ComxChannel channel,
                                     ButtonEvent *value))
    OCD ("Gdk") ;

extern  errno_t  bomxContentTypeList P_((ComxChannel channel,
                                         ContentTypeList *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxCrossingEvent P_((ComxChannel channel,
                                       CrossingEvent *value))
    OCD ("Gdk") ;

extern  errno_t  bomxDirectoryList P_((ComxChannel channel,
                                       DirectoryList *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxDRect P_((ComxChannel channel,
                               DRect *value))
    OCD ("Canvas") ;

extern  errno_t  bomxEvent P_((ComxChannel channel,
                               Event *value))
    OCD ("Gdk") ;

extern  errno_t  bomxFocusEvent P_((ComxChannel channel,
                                    FocusEvent *value))
    OCD ("Gdk") ;

extern  errno_t  bomxIRect P_((ComxChannel channel,
                               IRect *value))
    OCD ("Canvas") ;

extern  errno_t  bomxKeyEvent P_((ComxChannel channel,
                                  KeyEvent *value))
    OCD ("Gdk") ;

extern  errno_t  bomxMessageDesc P_((ComxChannel channel,
                                     MessageDesc *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxMessageList P_((ComxChannel channel,
                                     MessageList *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxMotionEvent P_((ComxChannel channel,
                                     MotionEvent *value))
    OCD ("Gdk") ;

extern  errno_t  bomxObjectDirectoryList P_((ComxChannel channel,
                                             ObjectDirectoryList *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxPair P_((ComxChannel channel,
                              Pair *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxPoint P_((ComxChannel channel,
                               Point *value))
    OCD ("Canvas") ;

extern  errno_t  bomxPoints P_((ComxChannel channel,
                                Points *value))
    OCD ("Canvas") ;

extern  errno_t  bomxPrintDimensions P_((ComxChannel channel,
                                         PrintDimensions *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxPrintScissor P_((ComxChannel channel,
                                      PrintScissor *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxRequisition P_((ComxChannel channel,
                                     Requisition *value))
    OCD ("Gtk") ;

extern  errno_t  bomxResolveOptions P_((ComxChannel channel,
                                        ResolveOptions *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxServerInfo P_((ComxChannel channel,
                                    ServerInfo *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxServerInfoList P_((ComxChannel channel,
                                        ServerInfoList *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxServerInfoListCache P_((ComxChannel channel,
                                             ServerInfoListCache *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxServerStateCache P_((ComxChannel channel,
                                          ServerStateCache *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxStorageInfo P_((ComxChannel channel,
                                     StorageInfo *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxSVP P_((ComxChannel channel,
                             SVP *value))
    OCD ("Canvas") ;

extern  errno_t  bomxSVPSegment P_((ComxChannel channel,
                                    SVPSegment *value))
    OCD ("Canvas") ;

extern  errno_t  bomxZoomLevelList P_((ComxChannel channel,
                                       ZoomLevelList *value))
    OCD ("Bonobo") ;

extern  errno_t  bomxZoomLevelNameList P_((ComxChannel channel,
                                           ZoomLevelNameList *value))
    OCD ("Bonobo") ;
