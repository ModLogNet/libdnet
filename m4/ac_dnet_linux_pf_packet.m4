dnl
dnl Check for Linux PF_PACKET sockets
dnl
dnl usage:	AC_DNET_LINUX_PF_PACKET
dnl results:	HAVE_LINUX_PF_PACKET
dnl
dnl This is a Linux-specific check, even though other operating systems
dnl (OpenSolaris) may have the PF_PACKET interface. The eth-linux.c code
dnl activated by this check is specific to Linux.
AC_DEFUN([AC_DNET_LINUX_PF_PACKET],
    [AC_CHECK_DECL([ETH_P_ALL],
       ac_cv_dnet_linux_pf_packet=yes,
       ac_cv_dnet_linux_pf_packet=no,
        [
#include <netpacket/packet.h>
#include <linux/if_ether.h>
])
    AC_MSG_CHECKING(for Linux PF_PACKET sockets)
    AC_MSG_RESULT($ac_cv_dnet_linux_pf_packet)
    if test $ac_cv_dnet_linux_pf_packet = yes ; then
	AC_DEFINE(HAVE_LINUX_PF_PACKET, 1,
		  [Define if you have Linux PF_PACKET sockets.])
    fi])
