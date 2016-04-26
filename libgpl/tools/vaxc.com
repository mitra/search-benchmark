! $Id: vaxc.com,v 1.2 2011/03/31 22:32:21 alex Exp $
!*******************************************************************************
!    Compile and link some applications.
!*******************************************************************************
$
$ sources =	-
	"chafn,"	+-
	"colior,"	+-
	"dump,"		+-
	"gflow,"	+-
	"pjoin,"	+-
	"tag311"
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
$             echo "Up-to-date ''file'.obj ..."
$         endif
$         i = i + 1
$         goto NextFile
$     endif
$
$ i = 0
$ NextProgram:
$     file = f$element (i, ",", sources)
$     if (file .nes. ",")
$     then
$         orev = f$file_attributes (file + ".obj", "RDT")
$         erev = f$file_attributes (file + ".exe", "RDT")
$         if (.not. $status) then erev = " "
$         if (orev .gts. erev)
$         then
$             echo "Linking ''file' ..."
$             link 'file', [-.libgpl]libgpl/lib, [-.libxdr]libxdr/lib, sys$library:ucx$ipc/lib, sys$library:vaxcrtl/lib
$         else
$             echo "Up-to-date ''file'.exe ..."
$         endif
$         i = i + 1
$         goto NextProgram
$     endif
