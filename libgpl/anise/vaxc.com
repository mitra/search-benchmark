! $Id: vaxc.com,v 1.2 2008/01/14 20:03:35 alex Exp alex $
!*******************************************************************************
!    Compile and link an application.
!*******************************************************************************
$
$ sources =	-
	"anise,"	+-
	"ftpClient,"	+-
	"http_util,"	+-
	"passClient,"	+-
	"wwwClient"
$
$ echo := write sys$output
$
$ define sys sys$library
$ define arpa sys$library
$ define netinet sys$library
$ rpcinc = f$parse ("[-.include.rpc]")
$ define rpc 'f$extract (0, f$locate ("]", rpcinc) + 1, rpcinc)'
$
$ command =								-
	"cc/debug=all/opt"						+-
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
$             echo "Up-to-date ''file'.obj ..."
$         endif
$         i = i + 1
$         goto NextFile
$     endif
$
$ echo "Linking ''sources' ..."
$ link 'sources', [-.libgpl]libgpl/lib, [-.libxdr]libxdr/lib, sys$library:ucx$ipc/lib, sys$library:vaxcrtl/lib
