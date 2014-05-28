#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([sinkhole], [@VERSION@], [adam@sigterm.info])
AC_CONFIG_SRCDIR([src/sinkhole.cpp])
AC_CONFIG_HEADERS([include/config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CPP
AC_PATH_PROG(MKDIR, mkdir)
AC_PATH_PROG(CP, cp)
AC_PATH_PROG(INSTALL, install)
AC_PATH_PROG(STRIP, strip)

# Checks for libraries.
AC_CHECK_LIB([dl], [dlopen], [ LIBS="$LIBS -ldl" ])
AC_CHECK_LIB([pthread], [pthread_create], [ LIBS="$LIBS -lpthread" ])
AC_CHECK_LIB([mysqlclient], [mysql_real_connect], [ HAVE_MYSQLCLIENT="yes"], [ HAVE_MYSQLCLIENT="no" ])
AC_CHECK_LIB([pcap], [pcap_create], [ HAVE_PCAP="yes" ], [ HAVE_PCAP="no" ])
AC_CHECK_LIB([rt], [aio_error], [ HAVE_RT="yes" ], [ HAVE_RT="no" ])

# Checks for header files.
AC_CHECK_HEADERS([pcap.h], [ HAVE_PCAP_H="yes" ], [ HAVE_PCAP_H="no" ])
AC_CHECK_HEADERS([mysql/mysql.h], [ HAVE_MYSQL_H="yes" ], [ HAVE_MYSQL_H="no"])
AC_CHECK_HEADERS([aio.h], [ HAVE_AIO_H="yes" ], [ HAVE_AIO_H="no" ])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
IO_ENGINE="poll"
AC_CHECK_FUNCS([epoll_create], [ IO_ENGINE="epoll" ])
AC_CHECK_FUNCS([kqueue], [ IO_ENGINE="kqueue" ])

CXXFLAGS=
AC_ARG_ENABLE(debug, [  --enable-debug          enable debugging symbols ], [ CXXFLAGS="$CXXFLAGS -g" ], [ CXXFLAGS="$CXXFLAGS -O3"] )
AC_ARG_ENABLE(poll,  [  --enable-poll           force poll usage ], [ IO_ENGINE="poll" ])
AC_ARG_ENABLE(epoll, [  --enable-epoll          force epoll usage ], [ IO_ENGINE="epoll" ])
AC_ARG_ENABLE(kqueue, [  --enable-kqueue         force kqueue usage ], [ IO_ENGINE="kqueue" ])

AC_ARG_WITH(include-dir, [  --with-include-dir      extra include paths if you have headers in nonstandard locations ], [ CXXFLAGS="$CXXFLAGS -I$withval" ])
AC_ARG_WITH(lib-dir,     [  --with-lib-dir          extra library paths if you have libraries in nonstandard locations ], [ LDFLAGS="$LDFLAGS -L$withval" ])

AC_SUBST(IO_ENGINE)

AC_SUBST(HAVE_MYSQLCLIENT)
AC_SUBST(HAVE_PCAP)
AC_SUBST(HAVE_RT)
AC_SUBST(HAVE_PCAP_H)
AC_SUBST(HAVE_MYSQL_H)
AC_SUBST(HAVE_AIO_H)

AC_CONFIG_FILES(@CONFIGFILES@)

AC_OUTPUT
