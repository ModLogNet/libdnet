dnl
dnl Check for route(7) STREAMS (UnixWare only?)
dnl
dnl usage:      AC_DNET_STREAMS_ROUTE
dnl results:    HAVE_STREAMS_ROUTE
dnl
AC_DEFUN([AC_DNET_STREAMS_ROUTE],
    [AC_MSG_CHECKING(for route(7) STREAMS)
    AC_CACHE_VAL(ac_cv_dnet_streams_route,
        if grep RTSTR_SEND /usr/include/net/route.h >/dev/null 2>&1 ; then
            ac_cv_dnet_streams_route=yes
        else
            ac_cv_dnet_streams_route=no
        fi)
    AC_MSG_RESULT($ac_cv_dnet_streams_route)
    if test $ac_cv_dnet_streams_route = yes ; then
        AC_DEFINE(HAVE_STREAMS_ROUTE, 1,
                  [Define if you have route(7) STREAMS.])
    fi])
