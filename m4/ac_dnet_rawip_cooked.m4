dnl
dnl Check for cooked raw IP sockets
dnl
dnl usage:      AC_DNET_RAWIP_COOKED
dnl results:    HAVE_RAWIP_COOKED
dnl
AC_DEFUN([AC_DNET_RAWIP_COOKED],
    [AC_MSG_CHECKING(for cooked raw IP sockets)
    AC_CACHE_VAL(ac_cv_dnet_rawip_cooked, [
	case "$host_os" in
	solaris*|irix*)
	    ac_cv_dnet_rawip_cooked=yes ;;
	*)
	    ac_cv_dnet_rawip_cooked=no ;;
	esac])
    AC_MSG_RESULT($ac_cv_dnet_rawip_cooked)
    if test $ac_cv_dnet_rawip_cooked = yes ; then
        AC_DEFINE(HAVE_RAWIP_COOKED, 1,
                  [Define if you have cooked raw IP sockets.])
    fi])
