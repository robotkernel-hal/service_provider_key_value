#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([interface_key_value], [4.0.2], [robert.burger@dlr.de])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

RMPM_ENV # must be before AC_PROG* but after AC_PROG_MAKE_SET and after AM_INIT_AUTOMAKE
RMPM_ARCH

# Checks for programs.
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_CXX
AC_PROG_AWK
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_PROG_LN_S
m4_ifdef([PKG_PROG_PKG_CONFIG], [PKG_PROG_PKG_CONFIG])
m4_ifdef([PKG_INSTALLDIR], [PKG_INSTALLDIR], [pkgconfigdir='${libdir}/pkgconfig' AC_SUBST([pkgconfigdir])])

PKG_CHECK_MODULES([ROBOTKERNEL], [robotkernel], [], [
                    RMPM_CHECK_MODULES([ROBOTKERNEL], [robotkernel.robotkernel])
                   ])

RM_FIND_LN_FROM_ENV

AC_DEFINE_UNQUOTED(BUILD_DATE,"`date -u`",[Date (usually date of extraction from SVN).])
AC_DEFINE_UNQUOTED(BUILD_USER,"`whoami`",[User (usually user logged in on shell).])
AC_DEFINE_UNQUOTED(BUILD_HOST,"`hostname`",[Hostname (build host).])

AC_ARG_WITH([debug],
            [AS_HELP_STRING([--with-debug],
                            [enable support for debug build])],
                            [with_debug=yes],
                            [with_debug=no])

CFLAGS="-Wall"
CXXFLAGS="-Wall"

AS_IF([test "x$with_debug" != xno], [ 
       CFLAGS="-Wall -g3 -O0"
       CXXFLAGS="-Wall -g3 -O0" ])


# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([strdup])

# Checks for library functions.
AC_CHECK_LIB(dl, dlopen, DL_LIBS="-ldl", DL_LIBS=)
AC_SUBST(DL_LIBS)

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([interface_key_value.pt], [cp interface_key_value.pt ${ac_abs_top_srcdir}])

AC_OUTPUT

