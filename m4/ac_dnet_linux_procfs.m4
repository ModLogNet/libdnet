dnl
dnl Check for the Linux /proc filesystem
dnl
dnl usage:	AC_DNET_LINUX_PROCFS
dnl results:	HAVE_LINUX_PROCFS
dnl
AC_DEFUN([AC_DNET_LINUX_PROCFS],
    [AC_MSG_CHECKING(for Linux proc filesystem)
    AC_CACHE_VAL(ac_cv_dnet_linux_procfs,
	if test "x`cat /proc/sys/kernel/ostype 2>&-`" = "xLinux" ; then
	    ac_cv_dnet_linux_procfs=yes
        else
	    ac_cv_dnet_linux_procfs=no
	fi)
    AC_MSG_RESULT($ac_cv_dnet_linux_procfs)
    if test $ac_cv_dnet_linux_procfs = yes ; then
	AC_DEFINE(HAVE_LINUX_PROCFS, 1,
		  [Define if you have the Linux /proc filesystem.])
    fi])
