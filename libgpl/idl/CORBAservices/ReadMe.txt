//File: ReadMe.txt
//March 2002 - this file has been updated due to Issue 4979 (CosTime Module).
//
// This file (or archive) contains the OMG IDL for CORBAservices. 
//
// The contents of the IDL files that appear in the CORBAservices manual were 
//  modified to implement the recently adopted OMG IDL Style Guide (OMG 
//  document ab/98-06-03), including module naming conventions and required 
//  contents of files. These changes did not affect the contained IDL; that is,
//  no identifiers were changed to match the guide since those would be 
//  incompatible changes.
//  
// In preparation for publication of this set of IDL files, there have been 
//  changes to the services IDL to take care of typos and syntax errors in 
//  the published IDL.
//
// This IDL is compilable when split into individual files.
//
// If this "Readme.txt" file is contained within a text file, the rest of the 
//  text file is a concatenation of the many files making up the CORBAservices.
//  Each file starts at a line beginning with "//File: ". The order of services
//  is that of the their position in the CORBAservices manual. Within a given 
//  service, the files appear in alphabetical order. (See below for exact order.)
// 
// If this "Readme.txt" file is within a .zip archive, the remainder of the archive
//  is made up of directories, one for each of the services. Each of those 
//  directories contains the modules/files making up that service.
//
// The IDL files contain tabs which are optimized for reading or printing with 
//  tab stops every four characters.
//
// Some IDL compilers might not support the IDL data type of "long long" because
//  their hardware or target language doesn't deal with such a type. The default
//  assumed in this package is the CORBA 2.2 specification, which has the data
//  type. For compilers not supporting it, some modules in this this package 
//  have workarounds. Such modules must be compiled with the preprocessor 
//  definition NOLONGLONG. These modules have a comment at the beginning of the 
//  module explaining this. 
//
// In CORBA 2.3, which introduces some new keywords, new IDL lexical rules
//  specify that any identifier which conflicts with a new keyword
//  can continue to be used by pre-pending the identifier with an underscore
//  "_". The IDL compiler will recognize such constructs as an identifier 
//  without the underscore. That means that all generated stubs and skeletons 
//  use the identifier as before, maintaining compatibility for code. Of course,
//  the IDL must be changed, but this is a small task compared to changing all
//  generated code. Some of the previously defined services have some 
//  identifiers that clash with the new keywords. To use CORBA 2.3 IDL, these 
//  identifiers must have have the underscore pre-pended. This is called an 
//  "Escaped Identifier". An escaped identifier is illegal in pre-CORBA 2.3.
//  Since this package is intended to be used with all versions of CORBA IDL
//  compilers, the technique to avoid problems is to use #ifdef pre-processor
//  constructs: use the ordinary identifier for pre-CORBA 2.3 and use the 
//  escaped identifier for CORBA 2.3 and later. The default in this package 
//  is to expect CORBA V2.3, so that the preprocessor definition 
//  NO_ESCAPED_IDENTIFIERS must be specified if you compile with a 
//  pre-CORBA 2.3 IDL compiler. Several modules in this package are affected; 
//  each has a comment at the beginning of the module explaining this.
//
//
// The services included at the time of publication of this version of the IDL 
//  are the following, listed in order of appearance in the CORBAservices book.
//  Under each service is a list of the modules associated with that service.
//
//    Naming 
//       CosNaming.idl
//       Lname-library.idl
//    Event 
//       CosEventChannelAdmin.idl
//       CosEventComm.idl
//       CosTypedEventChannelAdmin.idl
//       CosTypedEventComm.idl
//    Persistent Object 
//       CosPersistenceDDO.idl
//       CosPersistenceDS_CLI.idl
//       CosPersistencePDS.idl
//       CosPersistencePDS_DA.idl
//       CosPersistencePID.idl
//       CosPersistencePO.idl
//       CosPersistencePOM.idl
//    Life Cycle 
//       CosLifeCycle.idl
//       CosLifeCycleContainment.idl
//       CosLifeCycleReference.idl
//       CosCompoundLifeCycle.idl
//       LifeCycleService.idl
//    Concurrency Control 
//       ConcurrencyControl.idl
//    Externalization 
//       CosExternalization.idl
//       CosExternalizationContainment.idl
//       CosExternalizationReference.idl
//       CosStream.idl
//    Relationship 
//       CosContainment.idl
//       CosGraphs.idl
//       CosObjectIdentity.idl
//       CosReference.idl
//       CosRelationships.idl
//    Transaction 
//       CosTransactions.idl
//       CosTSPortability.idl
//    Query 
//       CosQuery.idl
//       CosQueryCollection.idl
//    Licensing 
//       CosLicensingManager.idl
//    Property 
//       CosPropertyService.idl
//    Time 
//       CosTime.idl
//       CosTimerEvent.idl
//       TimeBase.idl
//    Security 
//       DCE_CIOPSecurity.idl
//       NRService.idl
//       SECIOP.idl
//       Security.idl
//       SecurityAdmin.idl
//       SecurityLevel1.idl
//       SecurityLevel2.idl
//       SecurityReplaceable.idl
//       SSLIOP.idl
//    Object Trader 
//       CosTrading.idl
//       CosTradingDynamic.idl
//       CosTradingRepos.idl
//    Object Collections 
//       CosCollection.idl

