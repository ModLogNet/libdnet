dnl
dnl Check for rt_msghdr struct in <net/route.h>
dnl
dnl usage:	AC_DNET_ROUTE_RT_MSGHDR
dnl results:	HAVE_ROUTE_RT_MSGHDR
dnl
AC_DEFUN([AC_DNET_ROUTE_RT_MSGHDR],
    [AC_MSG_CHECKING(for rt_msghdr struct in <net/route.h>)
    AC_CACHE_VAL(ac_cv_dnet_route_h_has_rt_msghdr,
        AC_TRY_COMPILE([
#       include <sys/types.h>
#       include <sys/socket.h>
#       include <net/if.h>
#       include <net/route.h>],
        [struct rt_msghdr rtm; rtm.rtm_msglen = 0;],
	ac_cv_dnet_route_h_has_rt_msghdr=yes,
	ac_cv_dnet_route_h_has_rt_msghdr=no))
    AC_MSG_RESULT($ac_cv_dnet_route_h_has_rt_msghdr)
    if test $ac_cv_dnet_route_h_has_rt_msghdr = yes ; then
        AC_DEFINE(HAVE_ROUTE_RT_MSGHDR, 1,
	          [Define if <net/route.h> has rt_msghdr struct.])
    fi])
