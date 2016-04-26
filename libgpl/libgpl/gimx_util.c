/* $Id: gimx_util.c,v 1.18 2011/03/31 21:52:21 alex Exp $ */
/*******************************************************************************

File:

    gimx_util.c

    GIOP Marshaling Utilities.


Author:    Alex Measday


Purpose:

    The GIMX utilities are used to convert various General Inter-ORB Protocol
    (GIOP)-specific data types to and from their Common Data Representation
    (CDR) encodings, as defined in Chapter 15 of the CORBA specification.
    (The primitive CDR types are handled by the COMX utilities.)

    This package was originally part of the COMX utilities.  To make the
    source file sizes more manageable, I limited the COMX utilities to
    the basic CDR data types and moved the GIOP-specific data types and
    marshaling functions to the GIMX package.  See the COMX utilities
    prolog for design and usage information common to both packages.


Public Procedures:

    gimxAny() - decodes/encodes/erases a CORBA "any" structure.
    gimxAnySeq() - decodes/encodes/erases a sequence of CORBA "any" structure.
    gimxObjectKey() - decodes/encodes/erases a CORBA object key (sequence of
        octets).
    gimxProfileBody() - decodes/encodes/erases a CORBA "ProfileBody" structure.
    gimxTaggedProfile() - decodes/encodes/erases a CORBA "TaggedProfile"
        structure.
    gimxTimeval() - decodes/encodes a UNIX "timeval" structure.
    gimxTimevalSeq() - decodes/encodes/erases a sequence of UNIX "timeval"
        structures.

    gimx<Type>() - decode/encode/erase GIOP constructed types.
    .
    .
    .

*******************************************************************************/


#include  "pragmatics.h"		/* Compiler, OS, logging definitions. */
#include  <stdio.h>			/* Standard I/O definitions. */
#include  "coli_util.h"			/* CORBA-Lite utilities. */
#include  "gimx_util.h"			/* GIOP marshaling utilities. */


#if HAVE_NAMESPACES
    namespace  CoLi {
#endif

/*******************************************************************************

Procedures:

    gimx<Type> ()

    Decode/Encode/Erase CORBA Data Types.


Purpose:

    These automatically generated functions decode, encode, and erase
    CORBA data types.


    Invocation:

        status = gimx<Type> (channel, &value) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <value>		- I/O
            is the address of the host value involved in the marshaling
            operation.  If the operation is MxDECODE, the data flow is
            from the CDR value (in the channel's buffer) to the host
            value.  If the operation is MxENCODE, the data flow is from
            the host value to the CDR value (in the channel's buffer).
            If the operation is MxERASE, the dynamically-allocated fields
            of the host value are deallocated.
        <status>	- O
            returns the status of performing the marshaling operation,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/


/*******************************************************************************
    Auto-generated marshaling functions - generated from the auto-generated
        header file.
*******************************************************************************/

#include  "gimx_idl.c"			/* Auto-generated marshaling functions. */

/*******************************************************************************
    Lookup Tables - for converting IOP/GIOP/IIOP named constants to numbers
        and vice-versa.
*******************************************************************************/

const  ColiMap  AddressingDispositionLUT[] = {
  { (long) KeyAddr, "KeyAddr" },
  { (long) ProfileAddr, "ProfileAddr" },
  { (long) ReferenceAddr, "ReferenceAddr" },
  { 0L, NULL }
} ;

const  ColiMap  AssociationOptionsLUT[] = {
  { (long) NoProtection, "NoProtection" },
  { (long) Integrity, "Integrity" },
  { (long) Confidentiality, "Confidentiality" },
  { (long) DetectReplay, "DetectReplay" },
  { (long) DetectMisordering, "DetectMisordering" },
  { (long) EstablishTrustInTarget, "EstablishTrustInTarget" },
  { (long) EstablishTrustInClient, "EstablishTrustInClient" },
  { (long) NoDelegation, "NoDelegation" },
  { (long) SimpleDelegation, "SimpleDelegation" },
  { (long) CompositeDelegation, "CompositeDelegation" },
  { (long) IdentityAssertion, "IdentityAssertion" },
  { (long) DelegationByClient, "DelegationByClient" },
  { 0L, NULL }
} ;

const  ColiMap  ComponentIdLUT[] = {
  { (long) IOP_TAG_ORB_TYPE, "TAG_ORB_TYPE" },
  { (long) IOP_TAG_CODE_SETS, "TAG_CODE_SETS" },
  { (long) IOP_TAG_POLICIES, "TAG_POLICIES" },
  { (long) IOP_TAG_ALTERNATE_IIOP_ADDRESS, "TAG_ALTERNATE_IIOP_ADDRESS" },
  { (long) IOP_TAG_ASSOCIATION_OPTIONS, "TAG_ASSOCIATION_OPTIONS" },
  { (long) IOP_TAG_SEC_NAME, "TAG_SEC_NAME" },
  { (long) IOP_TAG_SPKM_1_SEC_MECH, "TAG_SPKM_1_SEC_MECH" },
  { (long) IOP_TAG_SPKM_2_SEC_MECH, "TAG_SPKM_2_SEC_MECH" },
  { (long) IOP_TAG_KerberosV5_SEC_MECH, "TAG_KerberosV5_SEC_MECH" },
  { (long) IOP_TAG_CSI_ECMA_Secret_SEC_MECH, "TAG_CSI_ECMA_Secret_SEC_MECH" },
  { (long) IOP_TAG_CSI_ECMA_Hybrid_SEC_MECH, "TAG_CSI_ECMA_Hybrid_SEC_MECH" },
  { (long) IOP_TAG_SSL_SEC_TRANS, "TAG_SSL_SEC_TRANS" },
  { (long) IOP_TAG_CSI_ECMA_Public_SEC_MECH, "TAG_CSI_ECMA_Public_SEC_MECH" },
  { (long) IOP_TAG_GENERIC_SEC_MECH, "TAG_GENERIC_SEC_MECH" },
  { (long) IOP_TAG_FIREWALL_TRANS, "TAG_FIREWALL_TRANS" },
  { (long) IOP_TAG_SCCP_CONTACT_INFO, "TAG_SCCP_CONTACT_INFO" },
  { (long) IOP_TAG_JAVA_CODEBASE, "TAG_JAVA_CODEBASE" },
  { (long) IOP_TAG_TRANSACTION_POLICY, "TAG_TRANSACTION_POLICY" },
  { (long) IOP_TAG_MESSAGE_ROUTER, "TAG_MESSAGE_ROUTER" },
  { (long) IOP_TAG_OTS_POLICY, "TAG_OTS_POLICY" },
  { (long) IOP_TAG_INV_POLICY, "TAG_INV_POLICY" },
  { (long) IOP_TAG_CSI_SEC_MECH_LIST, "TAG_CSI_SEC_MECH_LIST" },
  { (long) IOP_TAG_NULL_TAG, "TAG_NULL_TAG" },
  { (long) IOP_TAG_SECIOP_SEC_TRANS, "TAG_SECIOP_SEC_TRANS" },
  { (long) IOP_TAG_TLS_SEC_TRANS, "TAG_TLS_SEC_TRANS" },
  { (long) IOP_TAG_ACTIVITY_POLICY, "TAG_ACTIVITY_POLICY" },
  { (long) IOP_TAG_COMPLETE_OBJECT_KEY, "TAG_COMPLETE_OBJECT_KEY" },
  { (long) IOP_TAG_ENDPOINT_ID_POSITION, "TAG_ENDPOINT_ID_POSITION" },
  { (long) IOP_TAG_LOCATION_POLICY, "TAG_LOCATION_POLICY" },
  { (long) IOP_TAG_DCE_STRING_BINDING, "TAG_DCE_STRING_BINDING" },
  { (long) IOP_TAG_DCE_BINDING_NAME, "TAG_DCE_BINDING_NAME" },
  { (long) IOP_TAG_DCE_NO_PIPES, "TAG_DCE_NO_PIPES" },
  { (long) IOP_TAG_DCE_SEC_MECH, "TAG_DCE_SEC_MECH" },
  { (long) IOP_TAG_INET_SEC_TRANS, "TAG_INET_SEC_TRANS" },
  { 0L, NULL }
} ;

const  ColiMap  GIOPMsgTypeLUT[] = {
  { (long) Request, "Request" },
  { (long) Reply, "Reply" },
  { (long) CancelRequest, "CancelRequest" },
  { (long) LocateRequest, "LocateRequest" },
  { (long) LocateReply, "LocateReply" },
  { (long) CloseConnection, "CloseConnection" },
  { (long) MessageError, "MessageError" },
  { (long) Fragment, "Fragment" },
  { 0L, NULL }
} ;

const  ColiMap  LocateStatusTypeLUT[] = {
  { (long) UNKNOWN_OBJECT, "UNKNOWN_OBJECT" },
  { (long) OBJECT_HERE, "OBJECT_HERE" },
  { (long) OBJECT_FORWARD, "OBJECT_FORWARD" },
  { (long) OBJECT_FORWARD_PERM, "OBJECT_FORWARD_PERM" },
  { (long) LOC_SYSTEM_EXCEPTION, "LOC_SYSTEM_EXCEPTION" },
  { (long) LOC_NEEDS_ADDRESSING_MODE, "LOC_NEEDS_ADDRESSING_MODE" },
  { 0L, NULL }
} ;

const  ColiMap  ProfileIdLUT[] = {
  { (long) IOP_TAG_INTERNET_IOP, "TAG_INTERNET_IOP" },
  { (long) IOP_TAG_MULTIPLE_COMPONENTS, "TAG_MULTIPLE_COMPONENTS" },
  { (long) IOP_TAG_SCCP_IOP, "TAG_SCCP_IOP" },
  { 0L, NULL }
} ;

const  ColiMap  ReplyStatusTypeLUT[] = {
  { (long) NO_EXCEPTION, "NO_EXCEPTION" },
  { (long) USER_EXCEPTION, "USER_EXCEPTION" },
  { (long) SYSTEM_EXCEPTION, "SYSTEM_EXCEPTION" },
  { (long) LOCATION_FORWARD, "LOCATION_FORWARD" },
  { (long) LOCATION_FORWARD_PERM, "LOCATION_FORWARD_PERM" },
  { (long) NEEDS_ADDRESSING_MODE, "NEEDS_ADDRESSING_MODE" },
  { 0L, NULL }
} ;

const  ColiMap  ServiceIdLUT[] = {
  { (long) IOP_TransactionService, "TransactionService" },
  { (long) IOP_CodeSets, "CodeSets" },
  { (long) IOP_ChainBypassCheck, "ChainBypassCheck" },
  { (long) IOP_ChainBypassInfo, "ChainBypassInfo" },
  { (long) IOP_LogicalThreadId, "LogicalThreadId" },
  { (long) IOP_BI_DIR_IIOP, "BI_DIR_IIOP" },
  { (long) IOP_SendingContextRunTime, "SendingContextRunTime" },
  { (long) IOP_INVOCATION_POLICIES, "INVOCATION_POLICIES" },
  { (long) IOP_FORWARDED_IDENTITY, "FORWARDED_IDENTITY" },
  { (long) IOP_UnknownExceptionInfo, "UnknownExceptionInfo" },
  { (long) IOP_RTCorbaPriority, "RTCorbaPriority" },
  { (long) IOP_RTCorbaPriorityRange, "RTCorbaPriorityRange" },
  { (long) IOP_FT_GROUP_VERSION, "FT_GROUP_VERSION" },
  { (long) IOP_FT_REQUEST, "FT_REQUEST" },
  { (long) IOP_ExceptionDetailMessage, "ExceptionDetailMessage" },
  { (long) IOP_SecurityAttributeService, "SecurityAttributeService" },
  { (long) IOP_ActivityService, "ActivityService" },
  { 0L, NULL }
} ;

const  ColiMap  SyncScopeLUT[] = {
  { (long) MESSAGING_SYNC_NONE, "SYNC_NONE" },
  { (long) MESSAGING_SYNC_WITH_TRANSPORT, "SYNC_WITH_TRANSPORT" },
  { (long) MESSAGING_SYNC_WITH_SERVER, "SYNC_WITH_SERVER" },
  { (long) MESSAGING_SYNC_WITH_TARGET, "SYNC_WITH_TARGET" },
  { 0L, NULL }
} ;

/*******************************************************************************
    Code Set ID Lookup Table - for converting a code set ID to its lengthy
        name.  The code set values are from the OSF CHARACTER AND CODE SET
        REGISTRY.  (I later grabbed the more professional looking table
        from MICO.)
*******************************************************************************/

const  ColiMap  CodeSetIdLUT[] = {
  { 0x00010001L, "ISO 8859-1:1987; Latin Alphabet No. 1" },
  { 0x00010002L, "ISO 8859-2:1987; Latin Alphabet No. 2" },
  { 0x00010003L, "ISO 8859-3:1988; Latin Alphabet No. 3" },
  { 0x00010004L, "ISO 8859-4:1988; Latin Alphabet No. 4" },
  { 0x00010005L, "ISO/IEC 8859-5:1988; Latin-Cyrillic Alphabet" },
  { 0x00010006L, "ISO 8859-6:1987; Latin-Arabic Alphabet" },
  { 0x00010007L, "ISO 8859-7:1987; Latin-Greek Alphabet" },
  { 0x00010008L, "ISO 8859-8:1988; Latin-Hebrew Alphabet" },
  { 0x00010009L, "ISO/IEC 8859-9:1989; Latin Alphabet No. 5" },
  { 0x0001000aL, "ISO/IEC 8859-10:1992; Latin Alphabet No. 6" },
  { 0x00010020L, "ISO 646:1991 IRV (International Reference Version)" },
  { 0x00010100L, "ISO/IEC 10646-1:1993; UCS-2, Level 1" },
  { 0x00010101L, "ISO/IEC 10646-1:1993; UCS-2, Level 2" },
  { 0x00010102L, "ISO/IEC 10646-1:1993; UCS-2, Level 3" },
  { 0x00010104L, "ISO/IEC 10646-1:1993; UCS-4, Level 1" },
  { 0x00010105L, "ISO/IEC 10646-1:1993; UCS-4, Level 2" },
  { 0x00010106L, "ISO/IEC 10646-1:1993; UCS-4, Level 3" },
  { 0x00010108L, "ISO/IEC 10646-1:1993; UTF-1, UCS Transformation Format 1" },
  { 0x00010109L, "ISO/IEC 10646-1:1993; UTF-16, UCS Transformation Format 16-bit form" },
  { 0x00030001L, "JIS X0201:1976; Japanese phonetic characters" },
  { 0x00030004L, "JIS X0208:1978 Japanese Kanji Graphic Characters" },
  { 0x00030005L, "JIS X0208:1983 Japanese Kanji Graphic Characters" },
  { 0x00030006L, "JIS X0208:1990 Japanese Kanji Graphic Characters" },
  { 0x0003000aL, "JIS X0212:1990; Supplementary Japanese Kanji Graphic Chars" },
  { 0x00030010L, "JIS eucJP:1993; Japanese EUC" },
  { 0x00040001L, "KS C5601:1987; Korean Hangul and Hanja Graphic Characters" },
  { 0x00040002L, "KS C5657:1991; Supplementary Korean Graphic Characters" },
  { 0x0004000aL, "KS eucKR:1991; Korean EUC" },
  { 0x00050001L, "CNS 11643:1986; Taiwanese Hanzi Graphic Characters" },
  { 0x00050002L, "CNS 11643:1992; Taiwanese Extended Hanzi Graphic Chars" },
  { 0x0005000aL, "CNS eucTW:1991; Taiwanese EUC" },
  { 0x00050010L, "CNS eucTW:1993; Taiwanese EUC" },
  { 0x000b0001L, "TIS 620-2529, Thai characters" },
  { 0x000d0001L, "TTB CCDC:1984; Chinese Code for Data Communications" },
  { 0x05000010L, "OSF Japanese UJIS" },
  { 0x05000011L, "OSF Japanese SJIS-1" },
  { 0x05000012L, "OSF Japanese SJIS-2" },
  { 0x05010001L, "X/Open UTF-8; UCS Transformation Format 8 (UTF-8)" },
  { 0x05020001L, "JVC_eucJP" },
  { 0x05020002L, "JVC_SJIS" },
  { 0x10000001L, "DEC Kanji" },
  { 0x10000002L, "Super DEC Kanji" },
  { 0x10000003L, "DEC Shift JIS" },
  { 0x10010001L, "HP roman8; English and Western European languages" },
  { 0x10010002L, "HP kana8; Japanese katakana (incl JIS X0201:1976)" },
  { 0x10010003L, "HP arabic8; Arabic" },
  { 0x10010004L, "HP greek8; Greek" },
  { 0x10010005L, "HP hebrew8; Hebrew" },
  { 0x10010006L, "HP turkish8; Turkish" },
  { 0x10010007L, "HP15CN; encoding method for Simplified Chinese" },
  { 0x10010008L, "HP big5; encoding method for Traditional Chinese" },
  { 0x10010009L, "HP japanese15 (sjis); Shift-JIS for mainframe (incl JIS X0208:1990)" },
  { 0x1001000aL, "HP sjishi; Shift-JIS for HP user (incl JIS X0208:1990)" },
  { 0x1001000bL, "HP sjispc; Shift-JIS for PC (incl JIS X0208:1990)" },
  { 0x1001000cL, "HP ujis; EUC (incl JIS X0208:1990)" },
  { 0x10020025L, "IBM-037 (CCSID 00037); CECP for USA, Canada, NL, Ptgl, Brazil, Australia, NZ" },
  { 0x10020111L, "IBM-273 (CCSID 00273); CECP for Austria, Germany" },
  { 0x10020115L, "IBM-277 (CCSID 00277); CECP for Denmark, Norway" },
  { 0x10020116L, "IBM-278 (CCSID 00278); CECP for Finland, Sweden" },
  { 0x10020118L, "IBM-280 (CCSID 00280); CECP for Italy" },
  { 0x1002011aL, "IBM-282 (CCSID 00282); CECP for Portugal" },
  { 0x1002011cL, "IBM-284 (CCSID 00284); CECP for Spain, Latin America (Spanish)" },
  { 0x1002011dL, "IBM-285 (CCSID 00285); CECP for United Kingdom" },
  { 0x10020122L, "IBM-290 (CCSID 00290); Japanese Katakana Host Ext SBCS" },
  { 0x10020129L, "IBM-297 (CCSID 00297); CECP for France" },
  { 0x1002012cL, "IBM-300 (CCSID 00300); Japanese Host DBCS incl 4370 UDC" },
  { 0x1002012dL, "IBM-301 (CCSID 00301); Japanese PC Data DBCS incl 1880 UDC" },
  { 0x100201a4L, "IBM-420 (CCSID 00420); Arabic (presentation shapes)" },
  { 0x100201a8L, "IBM-424 (CCSID 00424); Hebrew" },
  { 0x100201b5L, "IBM-437 (CCSID 00437); PC USA" },
  { 0x100201f4L, "IBM-500 (CCSID 00500); CECP for Belgium, Switzerland" },
  { 0x10020341L, "IBM-833 (CCSID 00833); Korean Host Extended SBCS" },
  { 0x10020342L, "IBM-834 (CCSID 00834); Korean Host DBCS incl 1227 UDC" },
  { 0x10020343L, "IBM-835 (CCSID 00835); T-Ch Host DBCS incl 6204 UDC" },
  { 0x10020344L, "IBM-836 (CCSID 00836); S-Ch Host Extended SBCS" },
  { 0x10020345L, "IBM-837 (CCSID 00837); S-Ch Host DBCS incl 1880 UDC" },
  { 0x10020346L, "IBM-838 (CCSID 00838); Thai Host Extended SBCS" },
  { 0x10020347L, "IBM-839 (CCSID 00839); Thai Host DBCS incl 374 UDC" },
  { 0x10020352L, "IBM-850 (CCSID 00850); Multilingual IBM PC Data-MLP 222" },
  { 0x10020354L, "IBM-852 (CCSID 00852); Multilingual Latin-2" },
  { 0x10020357L, "IBM-855 (CCSID 00855); Cyrillic PC Data" },
  { 0x10020358L, "IBM-856 (CCSID 00856); Hebrew PC Data (extensions)" },
  { 0x10020359L, "IBM-857 (CCSID 00857); Turkish Latin-5 PC Data" },
  { 0x1002035dL, "IBM-861 (CCSID 00861); PC Data Iceland" },
  { 0x1002035eL, "IBM-862 (CCSID 00862); PC Data Hebrew" },
  { 0x1002035fL, "IBM-863 (CCSID 00863); PC Data Canadian French" },
  { 0x10020360L, "IBM-864 (CCSID 00864); Arabic PC Data" },
  { 0x10020362L, "IBM-866 (CCSID 00866); PC Data Cyrillic 2" },
  { 0x10020364L, "IBM-868 (CCSID 00868); Urdu PC Data" },
  { 0x10020365L, "IBM-869 (CCSID 00869); Greek PC Data" },
  { 0x10020366L, "IBM-870 (CCSID 00870); Multilingual Latin-2 EBCDIC" },
  { 0x10020367L, "IBM-871 (CCSID 00871); CECP for Iceland" },
  { 0x1002036aL, "IBM-874 (CCSID 00874); Thai PC Display Extended SBCS" },
  { 0x1002036bL, "IBM-875 (CCSID 00875); Greek" },
  { 0x10020370L, "IBM-880 (CCSID 00880); Multilingual Cyrillic" },
  { 0x1002037bL, "IBM-891 (CCSID 00891); Korean PC Data SBCS" },
  { 0x10020380L, "IBM-896 (CCSID 00896); Japanese Katakana characters; superset of JIS X0201:1976" },
  { 0x10020381L, "IBM-897 (CCSID 00897); PC Data Japanese SBCS (use with CP 00301)" },
  { 0x10020387L, "IBM-903 (CCSID 00903); PC Data Simplified Chinese SBCS (use with DBCS)" },
  { 0x10020388L, "IBM-904 (CCSID 00904); PC Data Traditional Chinese SBCS (use with DBCS)" },
  { 0x10020396L, "IBM-918 (CCSID 00918); Urdu" },
  { 0x10020399L, "IBM-921 (CCSID 00921); Baltic 8-Bit" },
  { 0x1002039aL, "IBM-922 (CCSID 00922); Estonia 8-Bit" },
  { 0x1002039eL, "IBM-926 (CCSID 00926); Korean PC Data DBCS incl 1880 UDC" },
  { 0x1002039fL, "IBM-927 (CCSID 00927); T-Ch PC Data DBCS incl 6204 UDC" },
  { 0x100203a0L, "IBM-928 (CCSID 00928); S-Ch PC Data DBCS incl 1880 UDC" },
  { 0x100203a1L, "IBM-929 (CCSID 00929); Thai PC Data DBCS incl 374 UDC" },
  { 0x100203a2L, "IBM-930 (CCSID 00930); Kat-Kanji Host MBCS Ext-SBCS" },
  { 0x100203a4L, "IBM-932 (CCSID 00932); Japanese PC Data Mixed" },
  { 0x100203a5L, "IBM-933 (CCSID 00933); Korean Host Extended SBCS" },
  { 0x100203a6L, "IBM-934 (CCSID 00934); Korean PC Data Mixed" },
  { 0x100203a7L, "IBM-935 (CCSID 00935); S-Ch Host Mixed" },
  { 0x100203a8L, "IBM-936 (CCSID 00936); PC Data S-Ch MBCS" },
  { 0x100203a9L, "IBM-937 (CCSID 00937); T-Ch Host Mixed" },
  { 0x100203aaL, "IBM-938 (CCSID 00938); PC Data T-Ch MBCS" },
  { 0x100203abL, "IBM-939 (CCSID 00939); Latin-Kanji Host MBCS" },
  { 0x100203adL, "IBM-941 (CCSID 00941); Japanese PC DBCS for Open" },
  { 0x100203aeL, "IBM-942 (CCSID 00942); Japanese PC Data Mixed" },
  { 0x100203afL, "IBM-943 (CCSID 00943); Japanese PC MBCS for Open" },
  { 0x100203b2L, "IBM-946 (CCSID 00946); S-Ch PC Data Mixed" },
  { 0x100203b3L, "IBM-947 (CCSID 00947); T-Ch PC Data DBCS incl 6204 UDC" },
  { 0x100203b4L, "IBM-948 (CCSID 00948); T-Ch PC Data Mixed" },
  { 0x100203b5L, "IBM-949 (CCSID 00949); IBM KS PC Data Mixed" },
  { 0x100203b6L, "IBM-950 (CCSID 00950); T-Ch PC Data Mixed" },
  { 0x100203b7L, "IBM-951 (CCSID 00951); IBM KS PC Data DBCS incl 1880 UDC" },
  { 0x100203bbL, "IBM-955 (CCSID 00955); Japan Kanji characters; superset of JIS X0208:1978" },
  { 0x100203c4L, "IBM-964 (CCSID 00964); T-Chinese EUC CNS1163 plane 1,2" },
  { 0x100203caL, "IBM-970 (CCSID 00970); Korean EUC" },
  { 0x100203eeL, "IBM-1006 (CCSID 01006); Urdu 8-bit" },
  { 0x10020401L, "IBM-1025 (CCSID 01025); Cyrillic Multilingual" },
  { 0x10020402L, "IBM-1026 (CCSID 01026); Turkish Latin-5" },
  { 0x10020403L, "IBM-1027 (CCSID 01027); Japanese Latin Host Ext SBCS" },
  { 0x10020410L, "IBM-1040 (CCSID 01040); Korean PC Data Extended SBCS" },
  { 0x10020411L, "IBM-1041 (CCSID 01041); Japanese PC Data Extended SBCS" },
  { 0x10020413L, "IBM-1043 (CCSID 01043); T-Ch PC Data Extended SBCS" },
  { 0x10020416L, "IBM-1046 (CCSID 01046); Arabic PC Data" },
  { 0x10020417L, "IBM-1047 (CCSID 01047); Latin-1 Open System" },
  { 0x10020440L, "IBM-1088 (CCSID 01088); IBM KS Code PC Data SBCS" },
  { 0x10020449L, "IBM-1097 (CCSID 01097); Farsi" },
  { 0x1002044aL, "IBM-1098 (CCSID 01098); Farsi PC Data" },
  { 0x10020458L, "IBM-1112 (CCSID 01112); Baltic Multilingual" },
  { 0x1002045aL, "IBM-1114 (CCSID 01114); T-Ch PC Data SBCS (IBM BIG-5)" },
  { 0x1002045bL, "IBM-1115 (CCSID 01115); S-Ch PC Data SBCS (IBM GB)" },
  { 0x10020462L, "IBM-1122 (CCSID 01122); Estonia" },
  { 0x100204e2L, "IBM-1250 (CCSID 01250); MS Windows Latin-2" },
  { 0x100204e3L, "IBM-1251 (CCSID 01251); MS Windows Cyrillic" },
  { 0x100204e4L, "IBM-1252 (CCSID 01252); MS Windows Latin-1" },
  { 0x100204e5L, "IBM-1253 (CCSID 01253); MS Windows Greek" },
  { 0x100204e6L, "IBM-1254 (CCSID 01254); MS Windows Turkey" },
  { 0x100204e7L, "IBM-1255 (CCSID 01255); MS Windows Hebrew" },
  { 0x100204e8L, "IBM-1256 (CCSID 01256); MS Windows Arabic" },
  { 0x100204e9L, "IBM-1257 (CCSID 01257); MS Windows Baltic" },
  { 0x10020564L, "IBM-1380 (CCSID 01380); S-Ch PC Data DBCS incl 1880 UDC" },
  { 0x10020565L, "IBM-1381 (CCSID 01381); S-Ch PC Data Mixed incl 1880 UDC" },
  { 0x10020567L, "IBM-1383 (CCSID 01383); S-Ch EUC GB 2312-80 set (1382)" },
  { 0x1002112cL, "IBM-300 (CCSID 04396); Japanese Host DBCS incl 1880 UDC" },
  { 0x10021352L, "IBM-850 (CCSID 04946); Multilingual IBM PC Data-190" },
  { 0x10021354L, "IBM-852 (CCSID 04948); Latin-2 Personal Computer" },
  { 0x10021357L, "IBM-855 (CCSID 04951); Cyrillic Personal Computer" },
  { 0x10021358L, "IBM-856 (CCSID 04952); Hebrew PC Data" },
  { 0x10021359L, "IBM-857 (CCSID 04953); Turkish Latin-5 PC Data" },
  { 0x10021360L, "IBM-864 (CCSID 04960); Arabic PC Data (all shapes)" },
  { 0x10021364L, "IBM-868 (CCSID 04964); PC Data for Urdu" },
  { 0x10021365L, "IBM-869 (CCSID 04965); Greek PC Data" },
  { 0x100213a2L, "IBM-5026 (CCSID 05026); Japanese Katakana-Kanji Host Mixed" },
  { 0x100213a7L, "IBM-5031 (CCSID 05031); S-Ch Host MBCS" },
  { 0x100213abL, "IBM-1027 and -300 (CCSID 05035); Japanese Latin-Kanji Host Mixed" },
  { 0x100213b8L, "IBM-5048 (CCSID 05048); Japanese Kanji characters; superset of JIS X0208:1990 (and 1983)" },
  { 0x100213b9L, "IBM-5049 (CCSID 05049); Japanese Kanji characters; superset of JIS X0212:1990" },
  { 0x100213cbL, "IBM-5067 (CCSID 05067); Korean Hangul and Hanja; superset of KS C5601:1987" },
  { 0x100221a4L, "IBM-420 (CCSID 08612); Arabic (base shapes only)" },
  { 0x10022341L, "IBM-833 (CCSID 09025); Korean Host SBCS" },
  { 0x10022342L, "IBM-834 (CCSID 09026); Korean Host DBCS incl 1880 UDC" },
  { 0x10022346L, "IBM-838 (CCSID 09030); Thai Host Extended SBCS" },
  { 0x10022360L, "IBM-864 (CCSID 09056); Arabic PC Data (unshaped)" },
  { 0x1002236aL, "IBM-874 (CCSID 09066); Thai PC Display Extended SBCS" },
  { 0x100223a5L, "IBM-9125 (CCSID 09125); Korean Host Mixed incl 1880 UDC" },
  { 0x10026352L, "IBM-850 (CCSID 25426); Multilingual IBM PC Display-MLP" },
  { 0x10026358L, "IBM-856 (CCSID 25432); Hebrew PC Display (extensions)" },
  { 0x10026412L, "IBM-1042 (CCSID 25618); S-Ch PC Display Ext SBCS" },
  { 0x10027025L, "IBM-037 (CCSID 28709); T-Ch Host Extended SBCS" },
  { 0x10028358L, "IBM-856 (CCSID 33624); Hebrew PC Display" },
  { 0x100283baL, "IBM33722 (CCSID 33722); Japanese EUC JISx201,208,212" },
  { 0x10030001L, "HTCsjis : Hitachi SJIS 90-1" },
  { 0x10030002L, "HTCujis : Hitachi eucJP 90-1" },
  { 0xffff0001L, "ASCII7" },
  { 0xffff0002L, "EBCDIC" },
  { 0xffff0003L, "HTML3" },
  { 0xffff0004L, "MACINTOSH" },
  { 0xffff0005L, "Windows 3.1 Latin 1" },
  { 0xffff0006L, "KOI8-R" },
  { 0xffff0007L, "UTF-7; UCS Transformation Format 7 (UTF-7)" },
  { 0L, NULL }
} ;

/*******************************************************************************

Procedures:

    gimxAny ()
    gimxAnySeq ()
    gimxObjectKey ()
    gimxTaggedProfile ()
    gimxTimeval ()
    gimxTimevalSeq ()


    Decode/Encode/Erase GIOP Constructed Types.


Purpose:

    These functions decode, encode, and erase GIOP constructed types,
    which are ultimately broken down into CDR primitive types.  As such,
    these functions largely depend on the COMX primitive functions for
    sorting out the marshaling direction, maintaining alignment, and
    checking for errors.


    Invocation:

        status = gimx<Type> (channel, &value) ;

    where

        <channel>	- I
            is the channel handle returned by comxCreate().
        <value>		- I/O
            is the address of the host value involved in the marshaling
            operation.  If the operation is MxDECODE, the data flow is
            from the CDR value (in the channel's buffer) to the host
            value.  If the operation is MxENCODE, the data flow is from
            the host value to the CDR value (in the channel's buffer).
            If the operation is MxERASE, the dynamically-allocated fields
            of the host value are deallocated.
        <status>	- O
            returns the status of performing the marshaling operation,
            zero if there were no errors and ERRNO otherwise.

*******************************************************************************/

/*!*****************************************************************************

    gimxAny() - decode/encode/erase a CORBA Any value.  The implemented
        types are limited to the basic CDR types.

*******************************************************************************/


errno_t  gimxAny (

#    if PROTOTYPES
        ComxChannel  channel,
        Any  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        Any  *value ;
#    endif

{    /* Local variables. */
    unsigned  long  which = (unsigned long) tk_null ;
    unsigned  long  max = 0 ;



/* Marshal the TypeCode. */

    if (value != NULL)  which = (unsigned long) value->which ;
    CHECK (comxEnum (channel, &which)) ;
    if (value != NULL)  value->which = (TCKind) which ;

/* Marshal the data value. */

    switch (which) {
    case tk_null:
    case tk_void:
        return (0) ;
    case tk_short:
        return (comxShort (channel, NULL_OR (value, data.vShort))) ;
    case tk_long:
        return (comxLong (channel, NULL_OR (value, data.vLong))) ;
    case tk_ushort:
        return (comxUShort (channel, NULL_OR (value, data.vUShort))) ;
    case tk_ulong:
        return (comxULong (channel, NULL_OR (value, data.vULong))) ;
    case tk_float:
        return (comxFloat (channel, NULL_OR (value, data.vFloat))) ;
    case tk_double:
        return (comxDouble (channel, NULL_OR (value, data.vDouble))) ;
    case tk_boolean:
        return (comxBoolean (channel, NULL_OR (value, data.vBoolean))) ;
    case tk_char:
        return (comxChar (channel, NULL_OR (value, data.vChar))) ;
    case tk_octet:
        return (comxOctet (channel, NULL_OR (value, data.vOctet))) ;
    case tk_TypeCode:
        which = (TCKind) tk_null ;
        if (value != NULL)  which = (unsigned long) value->data.vTypeCode ;
        CHECK (comxEnum (channel, &which)) ;
        if (value != NULL)  value->data.vTypeCode = (TCKind) which ;
        return (0) ;
    case tk_Principal:
        return (comxOctetSeq (channel, NULL_OR (value, data.vPrincipal))) ;
    case tk_string:
        CHECK (comxULong (channel, &max)) ;
        return (comxString (channel, NULL_OR (value, data.vString))) ;
    case tk_longlong:
        return (comxLongLong (channel, NULL_OR (value, data.vLongLong))) ;
    case tk_ulonglong:
        return (comxULongLong (channel, NULL_OR (value, data.vULongLong))) ;
    case tk_longdouble:
        return (comxLongDouble (channel, NULL_OR (value, data.vLongDouble))) ;
    case tk_wchar:
        return (comxWChar (channel, NULL_OR (value, data.vWChar))) ;
    case tk_wstring:
        CHECK (comxULong (channel, &max)) ;
        return (comxWString (channel, NULL_OR (value, data.vWString))) ;
    case ~0:
        return (comxLong (channel, NULL_OR (value, data.vIndirection))) ;
    case tk_any:
    case tk_objref:
    case tk_struct:
    case tk_union:
    case tk_enum:
    case tk_sequence:
    case tk_array:
    case tk_alias:
    case tk_except:
    case tk_fixed:
    case tk_value:
    case tk_value_box:
    case tk_native:
    case tk_abstract_interface:
    case tk_local_interface:
    default:
        SET_ERRNO (EINVAL) ;
        LGE "(gimxAny) Unsupported TypeCode: %s\n",
            coliToName (TCKindLUT, (int) which)) ;
        return (errno) ;
    }

}

/*!*****************************************************************************

    gimxAnySeq() - decode/encode/erase a sequence of Any structures.

*******************************************************************************/


errno_t  gimxAnySeq (

#    if PROTOTYPES
        ComxChannel  channel,
        AnySeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        AnySeq  *value ;
#    endif

{

    return (comxSequence (channel, value, (ComxFunc) gimxAny, sizeof (Any))) ;

}

/*!*****************************************************************************

    gimxObjectKey() - decode/encode/erase a GIOP octet sequence representing
        an object key.

*******************************************************************************/


errno_t  gimxObjectKey (

#    if PROTOTYPES
        ComxChannel  channel,
        ObjectKey  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ObjectKey  *value ;
#    endif

{

    return (comxOctetSeq (channel, (OctetSeq *) value)) ;

}

/*!*****************************************************************************

    gimxProfileBody() - decode/encode/erase a GIOP ProfileBody structure.
        GIOP versions 1.1 and on added a components (TaggedComponent) field
        at the end of the ProfileBody structure.  To simplify the internal
        representation of profile bodies, all versions have the components
        field.  If a version 1.0 body is being marshalled, simply skip the
        component field.

*******************************************************************************/


errno_t  gimxProfileBody (

#    if PROTOTYPES
        ComxChannel  channel,
        ProfileBody  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        ProfileBody  *value ;
#    endif

{

    CHECK (comxVersion (channel, NULL_OR (value, iiop_version))) ;
    CHECK (comxString (channel, NULL_OR (value, host))) ;
    CHECK (comxUShort (channel, NULL_OR (value, port))) ;
    CHECK (gimxObjectKey (channel, NULL_OR (value, object_key))) ;

/* GIOP version 1.0 lacks the components field. */

    if ((value == NULL) || GIOP_VERSION_GE (value->iiop_version, 1, 1)) {
        CHECK (comxSequence (channel,
                             NULL_OR (value, components),
                             (ComxFunc) gimxTaggedComponent,
                             sizeof (TaggedComponent))) ;
    } else if (comxGetOp (channel) == MxDECODE) {
        value->components.count = 0 ;		/* Empty sequence for GIOP 1.0. */
        value->components.elements = NULL ;
    }

    return (0) ;

}

/*!*****************************************************************************

    gimxTaggedProfile() - decode/encode/erase a GIOP TaggedProfile structure.
        The profile is encapsulated in an octet sequence.  The marshaling
        function converts to/from the encapsulated type for the TAG_INTERNET_IOP
        and TAG_MULTIPLE_COMPONENTS tags; for all other tags, the profile is
        encapsulated in the octet sequence.

*******************************************************************************/


errno_t  gimxTaggedProfile (

#    if PROTOTYPES
        ComxChannel  channel,
        TaggedProfile  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        TaggedProfile  *value ;
#    endif

{    /* Local variables. */
    ComxOperation  operation = comxGetOp (channel) ;
    OctetSeq  encapsulation = { 0, NULL } ;
    unsigned  long  which = ~0LU ;
    Version  version = comxGetVersion (channel) ;



    if (value != NULL)  which = value->which ;
    CHECK (comxULong (channel, &which)) ;
    if (value != NULL)  value->which = which ;

    switch (which) {

    case IOP_TAG_INTERNET_IOP:
        switch (operation) {
        case MxDECODE:
            if (comxOctetSeq (channel, &encapsulation))  goto onError ;
            if (comxEncapsule (version, operation, &encapsulation,
                               gimxProfileBody,
                               NULL_OR (value, data.iiop_body), NULL))
                goto onError ;
            break ;
        case MxENCODE:
            RETURN_IF_NULL (value) ;
            if (comxEncapsule (version, operation, &encapsulation,
                               gimxProfileBody, &value->data.iiop_body,
                               NULL))
                goto onError ;
            if (comxOctetSeq (channel, &encapsulation))  goto onError ;
            break ;
        case MxERASE:
            CHECK (gimxProfileBody (channel,
                                    NULL_OR (value, data.iiop_body))) ;
        default:
            break ;
        }
        comxErase ((ComxFunc) comxOctetSeq, &encapsulation) ;
        break ;

    case IOP_TAG_MULTIPLE_COMPONENTS:
        switch (operation) {
        case MxDECODE:
            if (comxOctetSeq (channel, &encapsulation))  goto onError ;
            if (comxEncapsule (version, operation, &encapsulation,
                               gimxMultipleComponentProfile,
                               NULL_OR (value, data.components), NULL))
                goto onError ;
            break ;
        case MxENCODE:
            RETURN_IF_NULL (value) ;
            if (comxEncapsule (version, operation, &encapsulation,
                               gimxMultipleComponentProfile,
                               &value->data.components, NULL))
                goto onError ;
            if (comxOctetSeq (channel, &encapsulation))  goto onError ;
            break ;
        case MxERASE:
            CHECK (gimxMultipleComponentProfile (channel,
                                            NULL_OR (value, data.components))) ;
            break ;
        default:
            break ;
        }
        comxErase ((ComxFunc) comxOctetSeq, &encapsulation) ;
        break ;

    default:
        CHECK (comxOctetSeq (channel, NULL_OR (value, data.profile_data))) ;
        break ;

    }

    return (0) ;


/*******************************************************************************
    Error Return - deallocate any allocated memory.
*******************************************************************************/

onError:
    PUSH_ERRNO ;
    comxErase ((ComxFunc) comxOctetSeq, &encapsulation) ;
    POP_ERRNO ;

    return (errno) ;

}

/*!*****************************************************************************

    gimxTimeval() - decode/encode a UNIX "timeval" structure.

*******************************************************************************/


errno_t  gimxTimeval (

#    if PROTOTYPES
        ComxChannel  channel,
        struct  timeval  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        struct  timeval  *value ;
#    endif

{

    CHECK (comxLong (channel, NULL_OR (value, tv_sec))) ;
    CHECK (comxLong (channel, NULL_OR (value, tv_usec))) ;

    return (0) ;

}

/*!*****************************************************************************

    gimxTimevalSeq() - decode/encode/erase a sequence of UNIX "timeval"
        structures.

*******************************************************************************/


errno_t  gimxTimevalSeq (

#    if PROTOTYPES
        ComxChannel  channel,
        TimevalSeq  *value)
#    else
        channel, value)

        ComxChannel  channel ;
        TimevalSeq  *value ;
#    endif

{

    return (comxSequence (channel, value,
                          (ComxFunc) gimxTimeval, sizeof (struct timeval))) ;

}


#if HAVE_NAMESPACES
    } ;     /* CoLi namespace. */
#endif
