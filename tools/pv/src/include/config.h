/* src/include/config.h.  Generated from header.in by configure.  */
/*!NOINDEX*/
/* Define if you have standard C headers. */
#define STDC_HEADERS 1

/* Define if you have "config.h" (yes, you have). */
#define HAVE_CONFIG_H 1

/* Various other header files. */
#define HAVE_GETOPT_H 1
#define HAVE_LIMITS_H 1
#define HAVE_SYS_IPC_H 1
#define HAVE_SYS_PARAM_H 1
#define HAVE_LIBGEN_H 1

/* Functions. */
#define HAVE_GETOPT 1
#define HAVE_GETOPT_LONG 1
#define HAVE_MEMCPY 1
#define HAVE_BASENAME 1
#define HAVE_SNPRINTF 1
#define HAVE_STAT64 1

#define HAVE_SPLICE 1
#ifdef HAVE_SPLICE
# define _GNU_SOURCE 1
#endif
/* NB the above must come before NLS, as NLS includes other system headers. */

/* NLS stuff. */
#define ENABLE_NLS 1
#define HAVE_LIBINTL_H 1
#define HAVE_LOCALE_H 1
#define HAVE_GETTEXT 1
#ifdef ENABLE_NLS
# include "library/gettext.h"
#else
# define _(String) (String)
# define N_(String) (String)
#endif

/* The name of the program. */
#define PROGRAM_NAME "pv"

/* The name of the package. */
#define PACKAGE "pv"

/* The current package version. */
#define VERSION "1.6.0"

/* Various identification and legal stuff. */
#define COPYRIGHT_YEAR   _("2015")
#define COPYRIGHT_HOLDER _("Andrew Wood <andrew.wood@ivarch.com>")
#define PROJECT_HOMEPAGE "http://www.ivarch.com/programs/" PROGRAM_NAME ".shtml"
#define BUG_REPORTS_TO   _("<pv@ivarch.com>")

/* LFS support. */
#define ENABLE_LARGEFILE 1
#ifdef ENABLE_LARGEFILE
# define __USE_LARGEFILE64 1
# define _LARGEFILE64_SOURCE 1
#else
/*
 * Some Macs have stat64 despite not having open64 while others don't have
 * either, so here even if we don't have open64 or LFS is disabled, we have
 * to check whether stat64 exists and only redefine it if it doesn't
 * otherwise some Macs fail to compile.
 */
# ifdef __APPLE__
#  ifndef HAVE_STAT64
#   define stat64 stat
#   define fstat64 fstat
#   define lstat64 lstat
#  endif
# else
#  define stat64 stat
#  define fstat64 fstat
#  define lstat64 lstat
# endif
# define open64 open
# define lseek64 lseek
#endif

/* #undef HAVE_IPC */
#ifdef HAVE_SYS_IPC_H
#define HAVE_IPC 1
#endif

/* #undef CURSOR_ANSWERBACK_BYTE_BY_BYTE */
#ifndef _AIX
#define CURSOR_ANSWERBACK_BYTE_BY_BYTE 1
#endif

/* Support for debugging output. */
/* #undef ENABLE_DEBUGGING */

/* EOF */
