dnl
dnl Check for arp_dev member in arpreq struct
dnl
dnl usage:	AC_DNET_ARPREQ_ARP_DEV
dnl results:	HAVE_ARPREQ_ARP_DEV (defined)
dnl
AC_DEFUN([AC_DNET_ARPREQ_ARP_DEV],
    [AC_MSG_CHECKING(for arp_dev in arpreq struct)
    AC_CACHE_VAL(ac_cv_dnet_arpreq_has_arp_dev,
	AC_TRY_COMPILE([
#       include <sys/types.h>
#	include <sys/socket.h>
#	include <net/if_arp.h>],
	[void *p = ((struct arpreq *)0)->arp_dev],
	ac_cv_dnet_arpreq_has_arp_dev=yes,
	ac_cv_dnet_arpreq_has_arp_dev=no))
    AC_MSG_RESULT($ac_cv_dnet_arpreq_has_arp_dev)
    if test $ac_cv_dnet_arpreq_has_arp_dev = yes ; then
	AC_DEFINE(HAVE_ARPREQ_ARP_DEV, 1,
		[Define if arpreq struct has arp_dev.])
    fi])
