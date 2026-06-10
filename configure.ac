##########################################################################
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
#
# Copyright 2016 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################

dnl                                               -*- Autoconf -*-
dnl Process this file with autoconf to produce a configure script.

AC_INIT(rdklogger, [3.0.0])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_MACRO_DIR([cfg])
AC_CONFIG_AUX_DIR([cfg])
AM_INIT_AUTOMAKE([foreign no-dist-gzip dist-bzip2 1.9])
LT_INIT

dnl subdirectories.

dnl Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_CPP
AC_PROG_CXXCPP

dnl use pretty build output with automake >= 1.11
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])],
  [AM_DEFAULT_VERBOSITY=1
   AC_SUBST(AM_DEFAULT_VERBOSITY)])




dnl **********************************
dnl checks for dependencies
dnl **********************************
# Optional check: only use systemd if it is found
PKG_CHECK_MODULES([SYSTEMD], [libsystemd >= 209], [
    AC_DEFINE([HAVE_SYSTEMD], [1], [Define if libsystemd is available])
    # The header is part of libsystemd, but we can verify it explicitly
    AC_CHECK_HEADERS([systemd/sd-journal.h])
], [
    AC_MSG_WARN([libsystemd not found; systemd support will be disabled])
])

# Substitute variables for Makefile.am
AC_SUBST([SYSTEMD_CFLAGS])
AC_SUBST([SYSTEMD_LIBS])

dnl Checks for header files.
AC_CHECK_HEADERS([limits.h memory.h stdlib.h string.h sys/socket.h unistd.h])

dnl Checks for syslog header file & functions.
AC_CHECK_HEADERS([syslog.h])
AC_CHECK_FUNCS([openlog syslog closelog])

dnl Checks for typedefs, structures, and compiler characteristics.
dnl AC_CHECK_HEADER_STDBOOL
AC_TYPE_UINT8_T

dnl Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset strdup strerror])

PKG_CHECK_MODULES([LOG4C], [log4c >= 1.2.3])

AC_CONFIG_FILES([Makefile
                 cfg/Makefile
                 src/Makefile
                 test/Makefile
                 utils/Makefile
                 include/Makefile])
AC_OUTPUT
