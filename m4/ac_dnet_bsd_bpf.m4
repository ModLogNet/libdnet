dnl
dnl Check for the Berkeley Packet Filter
dnl
dnl usage:	AC_DNET_BSD_BPF
dnl results:	HAVE_BSD_BPF
dnl
AC_DEFUN([AC_DNET_BSD_BPF],
    [AC_MSG_CHECKING(for Berkeley Packet Filter)
    AC_CACHE_VAL(ac_cv_dnet_bsd_bpf,
	if test -c /dev/bpf0 ; then
	    ac_cv_dnet_bsd_bpf=yes
	else
	    ac_cv_dnet_bsd_bpf=no
	fi)
    AC_MSG_RESULT($ac_cv_dnet_bsd_bpf)
    if test $ac_cv_dnet_bsd_bpf = yes ; then
	AC_DEFINE(HAVE_BSD_BPF, 1,
		  [Define if you have the Berkeley Packet Filter.])
    fi])
