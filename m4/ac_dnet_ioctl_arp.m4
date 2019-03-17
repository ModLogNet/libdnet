dnl
dnl Check for arp(7) ioctls
dnl
dnl usage:      AC_DNET_IOCTL_ARP
dnl results:    HAVE_IOCTL_ARP
dnl
AC_DEFUN([AC_DNET_IOCTL_ARP],
    [AC_MSG_CHECKING(for arp(7) ioctls)
    AC_CACHE_VAL(ac_cv_dnet_ioctl_arp,
	AC_EGREP_CPP(werd, [
#	include <sys/types.h>
#	define BSD_COMP
#	include <sys/ioctl.h>
#	ifdef SIOCGARP
	werd
#	endif],
	ac_cv_dnet_ioctl_arp=yes,
	ac_cv_dnet_ioctl_arp=no))
    case "$host_os" in
    irix*)
        ac_cv_dnet_ioctl_arp=no ;;
    esac
    AC_MSG_RESULT($ac_cv_dnet_ioctl_arp)
    if test $ac_cv_dnet_ioctl_arp = yes ; then
        AC_DEFINE(HAVE_IOCTL_ARP, 1,
                  [Define if you have arp(7) ioctls.])
    fi])
