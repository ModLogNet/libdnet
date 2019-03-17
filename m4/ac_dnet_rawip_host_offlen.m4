dnl
dnl Check for raw IP sockets ip_{len,off} host byte ordering
dnl
dnl usage:      AC_DNET_RAWIP_HOST_OFFLEN
dnl results:    HAVE_RAWIP_HOST_OFFLEN
dnl
AC_DEFUN([AC_DNET_RAWIP_HOST_OFFLEN],
    [AC_MSG_CHECKING([for raw IP sockets ip_{len,off} host byte ordering])
    AC_CACHE_VAL(ac_cv_dnet_rawip_host_offlen, [
	case "$host_os" in
	*openbsd*)
	    ac_cv_dnet_rawip_host_offlen=no ;;
	*bsd*|*darwin*|*osf*|*unixware*)
	    ac_cv_dnet_rawip_host_offlen=yes ;;
	*)
	    ac_cv_dnet_rawip_host_offlen=no ;;
	esac])
    AC_MSG_RESULT($ac_cv_dnet_rawip_host_offlen)
    if test $ac_cv_dnet_rawip_host_offlen = yes ; then
        AC_DEFINE(HAVE_RAWIP_HOST_OFFLEN, 1,
                  [Define if raw IP sockets require host byte ordering for ip_off, ip_len.])
    fi])
