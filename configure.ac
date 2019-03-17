#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libdnet], [1.12])
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_HEADERS([include/config.h])
AC_CONFIG_SRCDIR([include/dnet.h])
AC_SUBST(ac_aux_dir)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIRS([m4])

dnl XXX - stop the insanity!@#$
AM_MAINTAINER_MODE

dnl Check for system type.
dnl XXX - we do this to qualify our later feature checks, since some
dnl systems claim to support multiple features, but are quite b0rked.
AC_CANONICAL_HOST

# Checks for programs.
AC_LIBTOOL_DLOPEN
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AM_PROG_LIBTOOL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_RANLIB

dnl Checks for Python.
dnl XXX - use AM_PATH_PYTHON after automake upgrade
AC_MSG_CHECKING(for Python)
AC_ARG_WITH(python,
[  --with-python=DIR       build Python module (using python in DIR)],
[ case "$withval" in
  yes)
     AC_MSG_RESULT(yes)
     PYTHON="python"
     ;;
  no)
     AC_MSG_RESULT(no)
     ;;
  *)
     AC_MSG_RESULT($withval)
     for subdir in . bin; do
        if test -x $withval/$subdir/python; then
	   owd=`pwd`
	   if cd $withval/$subdir; then withval=`pwd`; cd $owd; fi
	   PYTHON="$withval/python"
	   break
	fi
     done
     if test "x$PYTHON" = "x"; then
        AC_ERROR(python not found in $withval)
     fi
     ;;
  esac 
])
AC_SUBST(PYTHON)
AC_SUBST(TCLINC)
AC_SUBST(TCLLIB)
AM_CONDITIONAL(PYTHON, [test "x$PYTHON" != "x"])
AM_CONDITIONAL(TCL, [test "x$TCLINC" != "x"])

dnl Checks for libraries.
AC_LBL_LIBRARY_NET

dnl Checks for Check.
AC_MSG_CHECKING(for Check)
AC_ARG_WITH(check,
[  --with-check=DIR        use Check (http://check.sf.net) in DIR],
[ case "$withval" in
  yes|no)
     AC_MSG_RESULT(no)
     ;;
  *)
     AC_MSG_RESULT($withval)
     if test -f $withval/include/check.h -a -f $withval/lib/libcheck.a; then
        owd=`pwd`
        if cd $withval; then withval=`pwd`; cd $owd; fi
        CHECKINC="-I$withval/include"
        CHECKLIB="-L$withval/lib -lcheck"
     elif test -f $withval/src/check.h -a -f $withval/src/libcheck.a; then
        owd=`pwd`
        if cd $withval; then withval=`pwd`; cd $owd; fi
        CHECKINC="-I$withval/src"
        CHECKLIB="-L$withval/src -lcheck"
     else
        AC_ERROR(check.h or libcheck.a not found in $withval)
     fi
     ;;
  esac ],
[ if test -f ${prefix}/include/check.h -a -f ${prefix}/lib/libcheck.a; then
     CHECKINC="-I${prefix}/include"
     CHECKLIB="-L${prefix}/lib -lcheck"
     AC_MSG_RESULT(yes)
  else
     AC_MSG_RESULT(no)
  fi
])
AC_SUBST(CHECKINC)
AC_SUBST(CHECKLIB)
AM_CONDITIONAL(HAVE_CHECK, test "x$CHECKLIB" != "x")

# Checks for libraries.

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h \
		  fcntl.h \
		  inttypes.h \
		  netdb.h \
		  net/bpf.h \
		  net/if.h \
		  net/if_dl.h \
		  net/if_tun.h \
		  net/radix.h \
		  netinet/in.h \
		  nlist.h \
		  stdlib.h \
		  string.h \
		  sys/ioctl.h \
		  sys/param.h \
		  sys/socket.h \
		  sys/sockio.h \
		  sys/sysctl.h \
		  sys/stdint.h \
		  sys/time.h])
AC_CHECK_HEADERS([net/pfvar.h], [], [],
 		 [[
 		  #include <net/if.h>
 		  #include <sys/socket.h>
 		  ]])
AC_CHECK_HEADERS([net/route.h], [], [],
 		 [[
 		  #include <sys/socket.h>
 		  #include <sys/types.h>
 		  ]])
AC_CHECK_HEADERS([netinet/in_var.h], [], [],
 		 [[
 		  #include <net/if.h>
 		  #include <netinet/in.h>
 		  #include <sys/types.h>
 		  ]])
dnl AC_CHECK_HEADERS([netinet/ip_fw.h], [], [],
dnl 		 [[
dnl 		  #include <net/if.h>
dnl 		  #include <netinet/in.h>
dnl 		  #include <sys/types.h>
dnl 		  ]])
AC_CHECK_HEADERS([netinet/ip_compat.h], [], [],
 		 [[
 		  #include <net/if.h>
		  #include <netinet/in.h>
		  #include <netinet/ip6.h>
		  #include <netinet/icmp6.h>
 		  ]])
dnl AC_CHECK_HEADERS([netinet/ip_fil.h], [], [],
dnl 		 [[
dnl 		  #include <net/if.h>
dnl 		  ]])
AC_CHECK_HEADERS([net/if_arp.h], [], [],
		 [[
		  #include <sys/socket.h>
		  #include <netinet/in.h>
		  #include <sys/types.h>
		  ]])
AC_CHECK_HEADERS([net/if_var.h], [], [],
		 [[
		  #include <sys/types.h>
		  #include <sys/socket.h>
		  #include <net/if.h>
		  ]])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_DNET_SOCKADDR_IN6
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
if test "$ac_cv_header_sys_socket_h" = yes ; then
	AC_DNET_SOCKADDR_SA_LEN
fi
if test "$ac_cv_header_net_if_arp_h" = yes ; then
	AC_DNET_ARPREQ_ARP_DEV
fi
if test "$ac_cv_header_net_route_h" = yes ; then
	AC_DNET_ROUTE_RT_MSGHDR
fi
if test "$GCC" = yes ; then
   	CFLAGS="$CFLAGS -Wall"
fi

# Checks for library functions.
AC_FUNC_MEMCMP
AC_CHECK_FUNCS([gethostbyname gettimeofday memmove memset pow socket strchr strdup strerror strncasecmp strpbrk strstr strtol strtoul])
AC_REPLACE_FUNCS(err strlcat strlcpy strsep)

dnl Checks for other system-specific jonks.
AC_DNET_BSD_BPF
AC_DNET_STREAMS_MIB2
AC_DNET_STREAMS_ROUTE
AC_DNET_IOCTL_ARP
AC_DNET_RAWIP_HOST_OFFLEN

dnl Check for arp interface.
if test "$ac_cv_dnet_ioctl_arp" = yes ; then
	AC_LIBOBJ([arp-ioctl])
elif test "$ac_cv_dnet_route_h_has_rt_msghdr" = yes ; then
	AC_LIBOBJ([arp-bsd])
else
	AC_LIBOBJ([arp-none])
fi

dnl Check for Ethernet interface.
if test "$ac_cv_dnet_bsd_bpf" = yes ; then
	AC_LIBOBJ([eth-bsd])
else
	AC_LIBOBJ([eth-none])
fi

dnl Check for firewall interface.
if test "$ac_cv_header_net_pfvar_h" = yes ; then
	AC_LIBOBJ([fw-pf])
elif test "$ac_cv_header_netinet_ip_fw_h" = yes ; then
	AC_LIBOBJ([fw-ipfw])
     dnl XXX - ipfw2 support later...
     dnl case "$host_os" in
     dnl *freebsd5*)
     dnl	AC_LIBOBJ([fw-none]) ;;
     dnl *)
     dnl 	AC_LIBOBJ([fw-ipfw]) ;;
     dnl esac
elif test "$ac_cv_header_netinet_ip_fil_h" = yes ; then
	AC_LIBOBJ([fw-ipf])
else
	AC_LIBOBJ([fw-none])
fi

dnl Check for network interface interface.
AC_LIBOBJ([intf])

dnl Check for raw IP interface.
AC_LIBOBJ([ip])

dnl Check for routing interface.
if test "$ac_cv_dnet_route_h_has_rt_msghdr" = yes ; then
	AC_LIBOBJ([route-bsd])
else
	AC_LIBOBJ([route-none])
fi

dnl Check for tun interface.
if test "$ac_cv_header_net_if_tun_h" = yes ; then
	AC_LIBOBJ([tun-bsd])
elif test -c "/dev/tun0" ; then
	AC_LIBOBJ([tun-bsd])
else
	AC_LIBOBJ([tun-none])
fi

AC_CONFIG_FILES([Makefile
		 dnet-config
                 include/Makefile
                 include/dnet/Makefile
                 man/Makefile
                 python/Makefile
		 python/setup.py
                 src/Makefile
                 test/Makefile
                 test/check/Makefile
                 test/dnet/Makefile])
AC_OUTPUT
