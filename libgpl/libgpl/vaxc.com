! $Id: vaxc.com,v 1.3 2011/07/18 17:34:58 alex Exp alex $
!*******************************************************************************
!    Compile and create general purpose library.
!*******************************************************************************
$
$ sources =	-
	"aperror,"	+-
	"bio_util,"	+-
	"bit_util,"	+-
	"bmw_util,"	+-
	"coli_util,"	+-
	"comx_util,"	+-
	"cosn_util,"	+-
	"crlf_util,"	+-
	"drs_util,"	+-
	"f1750a_util,"	+-
	"ffs,"		+-
	"fnm_util,"	+-
	"get_util,"	+-
	"gimx_util,"	+-
	"gsc_util,"	+-
	"hash_util,"	+-
	"id3_util,"	+-
	"ieee_util,"	+-
	"iiop_util,"	+-
	"ioctl_ucx,"	+-
	"iox_util,"	+-
	"lfn_util,"	+-
	"list_util,"	+-
	"log_util,"
$ sources = sources	+-
	"meo_util,"	+-
	"net_util,"	+-
	"nft_proc,"	+-
	"nft_util,"	+-
	"nvl_util,"	+-
	"nvp_util,"	+-
	"opt_util,"	+-
	"port_util,"	+-
	"rex_util,"	+-
	"rex_util_y,"	+-
	"skt_util,"	+-
	"str_util,"	+-
	"tcp_util,"	+-
	"tpl_util,"	+-
	"ts_util,"	+-
	"tv_util,"	+-
	"udp_util,"	+-
	"utf_util,"	+-
	"vim_util,"	+-
	"wcs_util,"	+-
	"xnet_util,"	+-
	"xqt_util"
$ objectlib = "libgpl.olb"
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
