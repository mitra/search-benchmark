! $Id: vaxc.com,v 1.1 2004/05/21 22:49:44 alex Exp alex $
!*******************************************************************************
!    Compile and create XDR library.
!*******************************************************************************
$
$ sources =	-
	"xdr_array,"		+-
	"xdr,"			+-
	"xdr_float,"		+-
	"xdr_mem,"		+-
	"xdr_rec,"		+-
	"xdr_reference,"	+-
	"xdr_stdio"
$ objectlib = "libxdr.olb"
$
$ echo := write sys$output
$
$ define sys sys$library
$ define arpa sys$library
$ define netinet sys$library
$ rpcinc = f$parse ("[-.include.rpc]")
$ define rpc 'f$extract (0, f$locate ("]", rpcinc) + 1, rpcinc)'
$
$ command =						-
	"cc/debug=all/opt"				+-
	"/include=([-.include],[-.include.vms])"	+-
	"/define=(""""""__vax__"""""")"
$ echo "''command'"
$ compile := 'command'
$ show symbol compile
$
$ i = 0
$ NextFile:
$     file = f$element (i, ",", sources)
$     if (file .nes. ",")
$     then
$         srev = f$file_attributes (file + ".c", "RDT")
$         orev = f$file_attributes (file + ".obj", "RDT")
$         if (.not. $status) then orev = " "
$         if (srev .gts. orev)
$         then
$             echo "Compiling ''file' ..."
$             compile 'file'
$         else
$             echo "Up-to-date ''file' ..."
$         endif
$         i = i + 1
$         goto NextFile
$     endif
$
$ CreateLibrary:
$
$ echo "Creating ''objectlib' ..."
$ library/create/list 'objectlib' 'sources'
