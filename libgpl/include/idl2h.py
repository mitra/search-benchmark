#! /usr/bin/python
#
# $Id: idl2h.py,v 1.7 2011/03/31 22:18:13 alex Exp $
#
#*******************************************************************************
#
#    IDL2H - scans one or more CORBA IDL files and generates header file
#        declarations for the structures, enumerations, and marshalling
#        functions defined in the IDL files.  The output is written to
#        standard output.
#
#*******************************************************************************


import getopt, os, os.path, shlex, string, subprocess, sys, traceback


gAppendSeq = 0			# Rewrite sloppy sequential type names?
gAutoSequence = 0		# Automatically generate sequence types?
gPostDefs = ""			# Output file for post-processing definitions.
gPrefix = "mx"			# Prefix for marshaling function names.
gScanInterface = 1		# Scan interface declarations?
gUseAlias = 0			# Resolve data type aliases?

gCPPCommand = "gcc -E -D_COMPONENT_REPOSITORY_ -DGIOP_1_2 -DNO_ESCAPED_IDENTIFIERS"
gInclude = 0			# Process #include's in IDL?

gPipedFile = ""			# Name of file fed to CPP.
gFileName = ""			# Current file as reported by CPP.

gMaxSectionNameLength = 8	# Maximum section name length for GCC.

gLastPrint = ""
gModulePath = ""
gConstMap = {}			# Maps constants to defining module.
gCustomTypes = []		# Data types with custom marshaling functions.
gEnumMap = {}			# Maps enumerated types to defining module.
gEnumValueMap = {}		# Maps enumerated types to value prefix/suffix.
gHookMap = {}			# Maps names to processing hooks.
gRenameMap = {}			# Maps types to equivalent types.

#    AliasMap - maps data types to shorter aliases.

gAliasMap = {}

#    CDR Map - maps the basic CORBA data types (plus a couple of others)
#        to the corresponding CoLi types.

gCDRMap = {}

#    MarshalMap - maps data types to their marshaling functions.

gMarshalMap = {}

#    TypeMap - maps defined data types to their defining modules.
#        Data types that are predefined in CoLi header files map
#        to module "None".

gTypeMap = {}

#*******************************************************************************
#
#    Utility Functions.
#
#*******************************************************************************


def addAlias (this, that):
    global gAliasMap
    if this == that:
        sys.stderr.write ("addAlias:  this = \"%s\"  that = \"%s\"\n" % (this, that))
        return 0

	# Avoid circular definitions; i.e., "<this> => <that> => ... => <this>".
    numSteps = 0
    mapping = that
    while gAliasMap.has_key (mapping):
        mapping = gAliasMap[mapping]
        if mapping == this:  return 0
        numSteps = numSteps + 1
        if numSteps > 5:
            sys.stderr.write ("addAlias:  this = \"%s\"  that = \"%s\"  mapping = \"%s\"\n" % (this, that, mapping))
        if numSteps > 10:
            return 0
    gAliasMap[this] = that
    return 1


def capitalize (name):
    return string.upper (name[0]) + name[1:]


def declareSeq (type):
    global gAliasMap, gAutoSequence, gModulePath, gRenameMap, gTypeMap
    if not gAutoSequence:  return
    typeSeq = type + "Seq"
    if gRenameMap.has_key (modularize (typeSeq)):
        typeSeq = gRenameMap[modularize (typeSeq)]
    if gTypeMap.has_key (typeSeq):  return
    print
    print "typedef  SEQUENCE (%s, %s) ;" % (type, typeSeq)
    gTypeMap[typeSeq] = gModulePath
    if gAliasMap.has_key ("sequence<" + type + ">"):
        addAlias (typeSeq, "sequence<" + type + ">")
    return


def getModule (modulePath):
    last = string.rfind (modulePath, "::")
    if last >= 0:
        return modulePath[last+2:]
    elif modulePath == "":
        return "CORBA"
    else:
        return modulePath


def getValue (stream):
    value = ""
    token = stream.get_token ()
    while (token != "") and (token != ";"):
        if token == ":":
            value = ""			# Remove namespace qualifier.
        elif value == "":
            value = token
        else:
            value = value + " " + token
        token = stream.get_token ()
    if token == ";":  stream.push_token (token)
    return value.replace ("< <", "<<")


def hookIt (name):
    global gHookMap
    if not gHookMap.has_key (name):  return ""
    hook = gHookMap[name]
    if hook == "ignore":  return hook
    return hook (name)


def modularize (name):
    global gModulePath
    if string.find (name, "::") >= 0:  return name
    if gModulePath == "":
        return name
    else:
        return getModule (gModulePath) + "::" + name


def renameOf (name):
    global gRenameMap
    if gRenameMap.has_key (modularize (name)):
        return gRenameMap[modularize (name)]
    elif gCDRMap.has_key (name):
        return gCDRMap[name]
    else:
        return name


def renumerate (name):
    global gEnumValueMap
    last = string.rfind (name, "::")
    if last >= 0:
        enumType = modularize (name[:last])
    else:
        enumType = ""
    if gEnumValueMap.has_key (enumType):
        tag = gEnumValueMap[enumType]
        if tag[0] == "^":
            return tag[1:] + getModule (name)
        elif tag[0] == "$":
            return getModule (name) + tag[1:]
        else:
            return getModule (name) + tag
    else:
        return renameOf (name)


def resolve (dataType, steps = 1):
    global gAliasMap
    while (steps > 0) and gAliasMap.has_key (dataType):
        dataType = gAliasMap[dataType]
        steps = steps - 1
    return dataType


def sequify (name):
    global gAppendSeq
    if gAppendSeq and (string.find (name, "sequence<") < 0):
        if string.upper (name[:3]) == "SEQ":
            name = name[3:] + "Seq"
        elif string.upper (name[-3:]) == "SEQ":
            name = name[:-3] + "Seq"
    return name


def skipTo (stream, target = ";"):
    token = stream.get_token ()
    while (token != "") and (token != target):
        if token == "{":  skipTo (stream, "}")
        token = stream.get_token ()
    return token


def unmodularize (name):
    return getModule (name)

#*******************************************************************************
#
#    getField(), parseField() - retrieve a field declaration (e.g., in a
#        structure or union) consisting of a data type and the field name.
#
#*******************************************************************************


def getField (stream, terminator = ";", unexpected = ["}"], steps = 1):

    global gAliasMap, gCDRMap

    names = []

#    Parse the field declaration.

    dataType, names, token = parseField (stream, terminator, unexpected)
    if token != terminator:  return (dataType, names, token)

#    Replace very long data types by their Windows counterparts.

    dataType = string.replace (dataType, "unsigned  long  long", "ULONGLONG")
    dataType = string.replace (dataType, "long  long", "LONGLONG")
    dataType = string.replace (dataType, "long  double", "LONGDOUBLE")

#    Perform renaming, etc. to the data type.  For a sequence type, first
#    apply the renaming and such to the data type being sequenced.

    if string.find (dataType, "sequence<") >= 0:
        dataType = dataType[9:-1]
        dataType = renameOf (dataType)
        dataType = unmodularize (dataType)
        if gCDRMap.has_key (dataType):  dataType = gCDRMap[dataType]
        seqType = "sequence<" + dataType + ">"
        if gAliasMap.has_key (seqType):
            dataType = gAliasMap[seqType]
        else:
            if gUseAlias:  dataType = resolve (dataType, 10)
            dataType = "sequence<" + dataType + ">"

    dataType = renameOf (dataType)		# Rename type.
    dataType = unmodularize (dataType)		# Remove namespace qualifiers.
    if gUseAlias:
        dataType = resolve (dataType, 10)	# Resolve aliases.

#    Convert CORBA data types to the corresponding CoLi data types.

    if gCDRMap.has_key (dataType):  dataType = gCDRMap[dataType]

    return (dataType, names, token)


def parseField (stream, terminator = ";", unexpected = ["}"]):
    dataType = ""
    names = []
    name = ""
    token = stream.get_token ()
    while token != terminator:			# Name precedes teminator.
        if (token == ""):  break
        if (token == "#"):
            global idlMap
            idlMap[token].parse (stream, 0)
            token = stream.get_token ()
            continue
        if token in unexpected:  break
        if string.find (terminator, token) >= 0:  break
        if (token == ","):
            names.append (name)
        elif len (names) > 0:
            "Dummy Statement!  Multiple variables per line."
            # Data type known: drop down, save token as name, get next token.
        elif dataType == "":			# Data type precedes name.
            dataType = name
        elif string.find (":<[", dataType[-1]) >= 0:
            dataType = dataType + name
        elif string.find (":<>[]", name) >= 0:
            dataType = dataType + name
        else:
            dataType = dataType + "  "  + name
        name = token
        token = stream.get_token ()
    if name == "]":
        last = string.rfind (dataType, " ")
        if last < 0:
            name = dataType + "]"
            dataType = ""
        else:
            name = dataType[last+1:] + "]"
            dataType = dataType [:last-1]
    if name != ",":
        names.append (name)
    return (dataType, names, token)

#*******************************************************************************
#
#    Multi-line (Block) Declarations; e.g., modules, structures, unions, etc.
#    BLOCK is the base class that parses a basic block:
#
#        <keyword> <name> {
#            ... declarations ...
#        } ;
#
#*******************************************************************************

class  BLOCK:

    def __init__ (self, keyword, skip = ""):
        self.keyword = keyword
        self.skip = skip
        self.name = ""

    def parseHeader (self, stream, depth):
        self.name = stream.get_token ()		# Block name.
        if self.name == "":  return
        return stream.get_token ()		# Opening curly brace.

    def parseBody (self, stream, depth):
        global gFileName
        while 1:
            token = stream.get_token ()
            if token == "":  break
            if token == "}":  break		# Closing curly brace?
            if idlMap.has_key (token):
                idlMap[token].parse (stream, depth + 1)
            else:
                if self.skip == "":		# Unexpected field?
                    sys.stderr.write ("Unexpected %s at line %d in %s\n" % (token, stream.lineno, gFileName))
                else:				# Expected fields to skip.
                    if skipTo (stream, self.skip) == "":  break
        return token

    def parseTrailer (self, stream, depth):
        global gLastPrint
        gLastPrint = self.keyword
        return stream.get_token ()		# Following semi-colon.

    def parse (self, stream, depth):
        if self.parseHeader (stream, depth) == "":  return ""
        self.parseBody (stream, depth)
        self.parseTrailer (stream, depth)


class  ENUM (BLOCK):

    def parseHeader (self, stream, depth):
        token = BLOCK.parseHeader (self, stream, depth)
        if token == "":  return ""
        if hookIt (modularize (self.name)) == "ignore":
            stream.push_token (token)
            skipTo (stream, ";")
            return ""

    def parseBody (self, stream, depth):
        global gEnumMap, gModulePath, gTypeMap
        self.name = renameOf (self.name)
        if gTypeMap.has_key (self.name):
            if gTypeMap[self.name] != "None":
                print "\n/*===== \"%s\" already defined =====*/" % self.name
            return skipTo (stream, "}")
        gEnumMap[self.name] = gModulePath
        gTypeMap[self.name] = "None"
        sys.stdout.write ("\ntypedef  enum  %s {" % self.name)
        while 1:
            token = stream.get_token ()
            if token == "":  break
            if (token == "#"):
                global idlMap
                idlMap[token].parse (stream, 0)
                continue
            if token == "}":  break		# Closing curly brace.
            if token == ",":			# Comma between enumerations.
                sys.stdout.write (",")
                continue
            enumeration = renumerate (self.name + "::" + token)
            if string.find (enumeration, "::") >= 0:  enumeration = token
            sys.stdout.write ("\n    %s" % enumeration)
        sys.stdout.write ("\n}  %s ;\n" % self.name)
        return token

    def parseTrailer (self, stream, depth):
        token = BLOCK.parseTrailer (self, stream, depth)
        return token


class  INTERFACE (BLOCK):

    def parse (self, stream, depth):
        global gAliasMap, gLastPrint, gScanInterface
        self.name = stream.get_token ()
        if (self.name == ""):  return ""
        if self.name != "IOR":		# Avoid "interface IOR".
            self.name = renameOf (self.name)
            if not gAliasMap.has_key (self.name):
                if gLastPrint != "interface":  print
                print "typedef  IOR  %s ;" % self.name
                gLastPrint = "interface"
                addAlias (self.name, "IOR")
        if gScanInterface:		# Process declarations inside interface?
            token = stream.get_token ()
            while token != "":		# Skip inheritance specification.
                if (token == "{") or (token == ";"):  break
                token = stream.get_token ()
            if token == "{":
                BLOCK.parseBody (self, stream, depth + 1)
            else:
                stream.push_token (token)
        return skipTo (stream, ";")


class  MODULE (BLOCK):

    def parseHeader (self, stream, depth):
        global gModulePath
        token = BLOCK.parseHeader (self, stream, depth)
        if token == "{":
            if gModulePath == "":
                gModulePath = self.name
            else:
                gModulePath = gModulePath + "::" + self.name
            print
            print "/* Module: %s */" % gModulePath
        return token

    def parseTrailer (self, stream, depth):
        global gModulePath
        token = BLOCK.parseTrailer (self, stream, depth)
        last = string.rfind (gModulePath, "::")
        if last < 0:  last = 0
        gModulePath = gModulePath[:last]
        return token


class  STRUCT (BLOCK):

    def parseHeader (self, stream, depth):
        global gModulePath, gTypeMap
        token = BLOCK.parseHeader (self, stream, depth)
        if token == "":  return ""
        if hookIt (modularize (self.name)) == "ignore":
            stream.push_token (token)
            skipTo (stream, ";")
            return ""
        self.name = renameOf (self.name)
        if gTypeMap.has_key (self.name):
            if gTypeMap[self.name] != "None":
                print "\n/*===== \"%s\" already defined =====*/" % self.name
            if skipTo (stream, "}") != "":  stream.get_token ()
            return ""
        gTypeMap[self.name] = gModulePath
        print
        print "typedef  struct  %s {" % self.name
        return token

    def parseBody (self, stream, depth):
        global gUseAlias
        while 1:
            dataType, names, token = getField (stream, steps = 0)
            if token != ";":  break
            dataType = sequify (dataType)
            if gUseAlias:  dataType = resolve (dataType)
            for name in names:
                if string.find (dataType, "sequence<") >= 0:	# Sequence type?
                    dataType = dataType[9:-1]
                    print "    SEQUENCE (%s, %s) ;" % (dataType, name)
                elif dataType[-1] == "*":
                    print "    %s%s ;" % (dataType, name)
                else:
                    print "    %s  %s ;" % (dataType, name)
        return token

    def parseTrailer (self, stream, depth):
        token = BLOCK.parseTrailer (self, stream, depth)
        print "}  %s ;" % self.name
        declareSeq (self.name)
        return token


class  UNION (BLOCK):

    def parseHeader (self, stream, depth):
        global gLastPrint, gModulePath, gTypeMap
        gLastPrint = self.keyword
        self.name = stream.get_token ()		# Union name.
        if self.name == "":  return ""
        if hookIt (modularize (self.name)) == "ignore":
            skipTo (stream, ";")
            return ""
        token = stream.get_token ()		# "switch" keyword.
        if token != "switch":  return token
        token = stream.get_token ()		# Left parenthesis.
        if token != "(":  return token
        self.switch = stream.get_token ()	# Switch type.
        if self.switch == "":  return ""
        token = stream.get_token ()		# Right parenthesis.
        if token != ")":  return token
        self.name = renameOf (self.name)
        if gTypeMap.has_key (self.name):
            if gTypeMap[self.name] != "None":
                print "\n/*===== \"%s\" already defined =====*/" % self.name
            if skipTo (stream, "}") != "":  stream.get_token ()
            return ""
        gTypeMap[self.name] = gModulePath
        self.switch = renameOf (self.switch)
        print
        print "typedef  struct  %s {" % self.name
        print "    %s  which ;" % self.switch
        print "    union {"
        return stream.get_token ()		# Opening curly brace.

    def parseBody (self, stream, depth):
        while 1:
            dataType, names, token = getField (stream, ";", ["case", "default", "}"])
            if token == "case":
                value = stream.get_token ()	# Switch value.
                if value == "":  break
                token = stream.get_token ()	# Skip colon.
                if token != ":":  break
                enumeration = renumerate (self.switch + "::" + value)
                if string.find (enumeration, "::") >= 0:  enumeration = value
                print "\t\t\t/* %s */" % enumeration
                continue
            elif token == "default":
                token = stream.get_token ()	# Skip colon.
                if token != ":":  break
                print "\t\t\t/* <default> */"
                continue
            if token != ";":  break
            dataType = sequify (dataType)
            if gUseAlias:  dataType = resolve (dataType)
            for name in names:
                if string.find (dataType, "sequence<") >= 0:	# Sequence type?
                    dataType = dataType[9:-1]
                    print "        SEQUENCE (%s, %s) ;" % (dataType, name)
                elif dataType[-1] == "*":
                    print "        %s%s ;" % (dataType, name)
                else:
                    print "        %s  %s ;" % (dataType, name)
        return token

    def parseTrailer (self, stream, depth):
        token = BLOCK.parseTrailer (self, stream, depth)
        print "    }  data ;"
        print "}  %s ;" % self.name
        declareSeq (self.name)
        return token

#*******************************************************************************
#
#    Single-line Declarations; e.g., constants and typedef's.
#
#*******************************************************************************

class  SINGLE:

    def __init__ (self, keyword):
        self.keyword = keyword


class  COMMENT (SINGLE):

    def parse (self, stream, depth):
        global gFileName, gInclude, gPipedFile
        token = stream.get_token ()
        if token == "pragma":
            stream.instream.readline ()
        else:
            lineno = string.atoi (token)
            token = stream.get_token ()
            stream.instream.readline ()
            stream.lineno = lineno
            gFileName = token[1:-1]
            if gFileName == "":  gFileName = gPipedFile
#            if gInclude:  print "/*  %s:%d  */" % (gFileName, lineno)
        return token


class  CONST (SINGLE):

    def parse (self, stream, depth):
        global gConstMap, gLastPrint, gModulePath
        dataType, names, token = getField (stream, "=")
        if token != "=":  return token
        name = names[0]
        value = getValue (stream)		# Get constant's value.
        if value == "":  return ""
        if dataType == "long":
            value = value + "L"
        elif dataType == "unsigned  long":
            value = value + "UL"
        if gConstMap.has_key (renameOf (name)):
            print "\n/*===== \"%s\" previously defined in %s =====*/" % (name, gConstMap[renameOf (name)])
        else:
            name = renameOf (name)
            gConstMap[name] = gModulePath
            if gLastPrint != "const":  print
            gLastPrint = "const"
            print "#define  %s  (%s)" % (name, value)
        token = stream.get_token ()		# Following semi-colon.
        return token


class  TYPEDEF (SINGLE):

    def parse (self, stream, depth):
        global gAliasMap, gCDRMap, gLastPrint, gModulePath, gTypeMap
        gLastPrint = "typedef"
        dataType, names, token = getField (stream, steps = 0)
        if token != ";":  return token
        name = names[0]
        if hookIt (modularize (name)) == "ignore":  return token
        name = renameOf (name)
#        if (string.find (dataType, "sequence<") >= 0):
#            name = sequify (name)
#            baseType = dataType[9:-1] ;
#            if name == (string.upper (baseType[:1]) + baseType[1:] + "Seq"):
#                name = baseType + "Seq"
#        elif dataType[-3:] == "Seq":
#            name = sequify (name)
        if (string.find (dataType, "sequence<") >= 0) or (dataType[-3:] == "Seq"):
            name = sequify (name)
        if gTypeMap.has_key (name):		# Already defined?
            print "\n/*===== \"%s\" already defined =====*/" % name
        elif string.find (dataType, "sequence<") >= 0:
            print
            dataType = resolve (dataType)
            if gMarshalMap.has_key (dataType):
                gMarshalMap[name] = gMarshalMap[dataType]
                print "typedef  %s  %s ;" % (dataType, name)
                gTypeMap[name] = "None"
            else:
                dataType = dataType[9:-1]
                print "typedef  SEQUENCE (%s, %s) ;" % (dataType, name)
                gTypeMap[name] = gModulePath
        else:					# Non-sequence type.
            addAlias (name, dataType)
            if gTypeMap.has_key (dataType) or gAliasMap.has_key (dataType):
                gTypeMap[name] = "None"
            elif dataType in gCDRMap.values ():
                gTypeMap[name] = "None"
            else:
                gTypeMap[name] = gModulePath
            print
            if dataType[-1] == "*":
                print "typedef  %s%s ;" % (dataType, name)
            else:
                print "typedef  %s  %s ;" % (dataType, name)
        return token


class  TYPEPREFIX (SINGLE):

    def parse (self, stream, depth):
        token = stream.get_token ()
        token = stream.get_token ()
        token = stream.get_token ()
        if token != ";":
            stream.push_token (token)
        return token


class  IGNORE (SINGLE):

    def parse (self, stream, depth):
        return self.keyword


class  SKIP (SINGLE):

    def __init__ (self, keyword, terminator = ";"):
        SINGLE.__init__ (self, keyword)
        self.terminator = terminator

    def parse (self, stream, depth):
        return skipTo (stream, self.terminator)

#*******************************************************************************
#
#    idlMap - maps IDL delcaration keywords to the objects used to parse the
#        declarations.
#
#*******************************************************************************

idlMap = {
    "abstract"		: SKIP ("abstract"),
    "const"		: CONST ("const"),
    "enum"		: ENUM ("enum"),
    "exception"		: SKIP ("exception"),
    "import"		: SKIP ("import"),
    "interface"		: INTERFACE ("interface", ";"),
    "local"		: SKIP ("local"),
    "module"		: MODULE ("module"),
    "native"		: SKIP ("native"),
    "struct"		: STRUCT ("struct"),
    "typedef"		: TYPEDEF ("typedef"),
    "typeprefix"	: TYPEPREFIX ("typeprefix"),
    "union"		: UNION ("union"),
    "valuetype"		: SKIP ("valuetype"),
    "#"			: COMMENT ("#")
}

#*******************************************************************************
#*******************************************************************************
#
#    Main Program - parses each IDL file on the command line and writes
#        the generated C header file to standard output.
#
#*******************************************************************************
#*******************************************************************************

opts, args = getopt.getopt (sys.argv[1:], "",
                            ["appendSeq", "autoSequence",
                             "cppCommand=", "include",
                             "postDefs=", "preDefs=",
                             "prefix=", "useAlias",
                             "epoch"])

for option, argument in opts:
    if option == "--appendSeq":
        gAppendSeq = 1
    elif option == "--autoSequence":
        gAutoSequence = 1
    elif option == "--cppCommand":
        gCPPCommand = argument
    elif option == "--include":
        gInclude = 1
    elif option == "--preDefs":
        exec (open (argument))
    elif option == "--postDefs":
        gPostDefs = argument
    elif option == "--prefix":
        gPrefix = argument
    elif option == "--useAlias":
        gUseAlias = 1
    elif option == "--epoch":
        gAppendSeq = 1
        gAutoSequence = 1
        gPrefix = "epmx"
        gUseAlias = 1

firstFile = 1

for file in args:
    gPipedFile = file
    gFileName = gPipedFile
    gLastPrint = ""
    if firstFile:
        firstFile = 0
    else:
        print "\f"
    print "/*******************************************************************************"
    print "   ", gFileName
    print "*******************************************************************************/"
    directory, filename = os.path.split (gPipedFile)
    if hookIt (filename) == "ignore":  continue
    if gInclude:
        command = gCPPCommand + " - < " + gPipedFile
    else:
        command = "grep -v '#include' " + gPipedFile + " | " + gCPPCommand + " -"
    sys.stderr.write ("%s\n" % command)
    pipe = subprocess.Popen (command, shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE, close_fds=True)
    stream = shlex.shlex (pipe.stdout)
    stream.commenters = ""
    while 1:
        token = stream.get_token ()
        if token == "":  break
        if idlMap.has_key (token):
            idlMap[token].parse (stream, 1)
        else:
            sys.stderr.write ("MAIN: Unexpected %s at line %d in %s\n" % (token, stream.lineno, gFileName))
            skipTo (stream, ";")

uMap = {}

if len (gEnumMap) > 0:
    numEnums = 0
    uMap.clear ()
    items = gEnumMap.items ()
    for key, value in items:
        if value == "None":  continue
        uMap[string.upper (key)] = key
        numEnums = numEnums + 1

if numEnums > 0:
    print "\f"
    print "/*******************************************************************************"
    print "    Tables for mapping enumerated values to names and vice-versa;"
    print "    see the coliToName() and coliToNumber() functions."
    print "*******************************************************************************/"
    print
    enumList = uMap.keys ()
    enumList.sort ()
    for enum in enumList:
        enum = uMap[enum]
        module = getModule (gEnumMap[enum])
        print "extern  const  ColiMap  %.28sLUT[]  OCD (\"%s\") ;" % (enum, module[:gMaxSectionNameLength])

if len (gTypeMap) > 0:
    print "\f"
    print "/*******************************************************************************"
    print "    Public functions."
    print "*******************************************************************************/"
    print
    print "/* Marshaling functions for the defined data types. */"
    uMap.clear ()
    items = gTypeMap.items ()
    for key, value in items:
        if value == "None":  continue
        uMap[string.upper (key)] = key
    typeList = uMap.keys ()
    typeList.sort ()
    for type in typeList:
        type = uMap[type]
        if type in gCustomTypes:  continue		# Skip custom types.
        module = getModule (gTypeMap[type])
        print
        print "extern  errno_t  %s%s P_((ComxChannel channel," % (gPrefix, string.upper (type[0]) + type[1:])
        numSpaces = len ("extern  errno_t  ") + len (gPrefix)
        numSpaces = numSpaces + len (type) + len (" P_((")
        print "%*s%s *value))" % (numSpaces, "", type)
        print "    OCD (\"%s\") ;" % module[:gMaxSectionNameLength]

if gPostDefs != "":

    f = open (gPostDefs, "w")
    f.write ("# %s Definitions\n" % string.upper (gPrefix))

    f.write ("\n# Aliases\n\nglobal gAliasMap\n")
    keyList = gAliasMap.keys ()
    keyList.sort ()
    for key in keyList:
        f.write ("gAliasMap[\"%s\"] = \"%s\"\n" % (key, gAliasMap[key]))

    f.write ("\n# CDR Types\n\nglobal gCDRMap\n")
    keyList = gCDRMap.keys ()
    keyList.sort ()
    for key in keyList:
        f.write ("gCDRMap[\"%s\"] = \"%s\"\n" % (key, gCDRMap[key]))

    f.write ("\n# Custom Types\n\nglobal gCustomTypes\n")
    gCustomTypes.sort ()
    for dataType in gCustomTypes:
        f.write ("gCustomTypes = gCustomTypes + [\"%s\"]\n" % dataType)

    f.write ("\n# Enumerated Types\n\nglobal gEnumMap\n")
    keyList = gEnumMap.keys ()
    keyList.sort ()
    for key in keyList:
        f.write ("gEnumMap[\"%s\"] = \"None\"\n" % key)

    f.write ("\n# Enumerated Value Prefixes/Suffixes\n\nglobal gEnumValueMap\n")
    keyList = gEnumValueMap.keys ()
    keyList.sort ()
    for key in keyList:
        f.write ("gEnumValueMap[\"%s\"] = \"%s\"\n" % (key, gEnumValueMap[key]))

    f.write ("\n# Rename Types\n\nglobal gRenameMap\n")
    keyList = gRenameMap.keys ()
    keyList.sort ()
    for key in keyList:
        f.write ("gRenameMap[\"%s\"] = \"%s\"\n" % (key, gRenameMap[key]))

    f.write ("\n# Defined Types\n\nglobal gTypeMap\n")
    keyList = gTypeMap.keys ()
    keyList.sort ()
    for key in keyList:
        f.write ("gTypeMap[\"%s\"] = \"None\"\n" % key)

    f.write ("\n# Marshaling Functions (implicit)\n\nglobal gMarshalMap\n")
    keyList = gMarshalMap.keys ()
    keyList.sort ()
    for key in keyList:
        f.write ("gMarshalMap[\"%s\"] = \"%s\"\n" % (key, gMarshalMap[key]))

    f.write ("\n# Marshaling Functions (explicit)\n\n")
    keyList = gTypeMap.keys ()
    keyList.sort ()
    for key in keyList:
        value = gTypeMap[key]
        if value == "None":  continue
        dataType = resolve (key)
        if gMarshalMap.has_key (dataType):
            f.write ("gMarshalMap[\"%s\"] = \"%s\"\n" % (key, gMarshalMap[dataType]))
        else:
            f.write ("gMarshalMap[\"%s\"] = \"%s%s\"\n" % (key, gPrefix, capitalize (key)))

    f.close ()
