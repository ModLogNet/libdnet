dnl
dnl Check for sockaddr_in6 struct in <netinet/in.h>
dnl
dnl usage:	AC_DNET_SOCKADDR_IN6
dnl results:	HAVE_SOCKADDR_IN6
dnl
AC_DEFUN([AC_DNET_SOCKADDR_IN6],
    [AC_MSG_CHECKING(for sockaddr_in6 struct in <netinet/in.h>)
    AC_CACHE_VAL(ac_cv_dnet_netinet_in_h_has_sockaddr_in6,
        AC_TRY_COMPILE([
#       include <sys/types.h>
#	include <sys/socket.h>
#       include <netinet/in.h>],
        [struct sockaddr_in6 sin6; sin6.sin6_family = AF_INET6;],
	ac_cv_dnet_netinet_in_h_has_sockaddr_in6=yes,
	ac_cv_dnet_netinet_in_h_has_sockaddr_in6=no))
    AC_MSG_RESULT($ac_cv_dnet_netinet_in_h_has_sockaddr_in6)
    if test $ac_cv_dnet_netinet_in_h_has_sockaddr_in6 = yes ; then
        AC_DEFINE(HAVE_SOCKADDR_IN6, 1,
	          [Define if <netinet/in.h> has sockaddr_in6 struct.])
    fi])
