dnl
dnl Check for SNMP MIB2 STREAMS (Solaris only?)
dnl
dnl usage:      AC_DNET_STREAMS_MIB2
dnl results:    HAVE_STREAMS_MIB2
dnl
AC_DEFUN([AC_DNET_STREAMS_MIB2],
    [AC_MSG_CHECKING(for SNMP MIB2 STREAMS)
    AC_CACHE_VAL(ac_cv_dnet_streams_mib2,
        if test -f /usr/include/inet/mib2.h -a -c /dev/ip ; then
            ac_cv_dnet_streams_mib2=yes
        else
            ac_cv_dnet_streams_mib2=no
        fi)
    AC_MSG_RESULT($ac_cv_dnet_streams_mib2)
    if test $ac_cv_dnet_streams_mib2 = yes ; then
        AC_DEFINE(HAVE_STREAMS_MIB2, 1,
                  [Define if you have SNMP MIB2 STREAMS.])
    fi])
