#ifndef __CDIO_CONFIG_H__
#define __CDIO_CONFIG_H__
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define if building universal (internal helper macro) */
/* #undef AC_APPLE_UNIVERSAL_BUILD */

/* Define 1 if you are compiling using cygwin */
/* #undef CYGWIN */

/* what to put between the brackets for empty arrays */
#define CDIO_EMPTY_ARRAY_SIZE 

/* Define 1 if you have BSDI-type CD-ROM support */
/* #undef HAVE_BSDI_CDROM */

/* Define this if you have libcddb installed */
#define CDIO_HAVE_CDDB /**/

/* Define to 1 if you have the `chdir' function. */
#define CDIO_HAVE_CHDIR 1

/* Define to 1 if you have the <CoreFoundation/CFBase.h> header file. */
/* #undef HAVE_COREFOUNDATION_CFBASE_H */

/* Define to 1 if you have the <curses.h> header file. */
/* #undef HAVE_CURSES_H */

/* Define 1 if you have Darwin OS X-type CD-ROM support */
/* #undef HAVE_DARWIN_CDROM */

/* Define if time.h defines extern long timezone and int daylight vars. */
/* #undef HAVE_DAYLIGHT */

/* Define to 1 if you have the <ddk/ntddcdrm.h> header file. */
/* #undef HAVE_DDK_NTDDCDRM_H */

/* Define to 1 if you have the <ddk/ntddscsi.h> header file. */
/* #undef HAVE_DDK_NTDDSCSI_H */

/* Define to 1 if you have the <ddk/scsi.h> header file. */
/* #undef HAVE_DDK_SCSI_H */

/* Define 1 if you have the Apple DiskArbitration framework */
/* #undef HAVE_DISKARBITRATION */

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the `drand48' function. */
/* #undef HAVE_DRAND48 */

/* Define to 1 if you have the <dvd.h> header file. */
/* #undef HAVE_DVD_H */

/* Define to 1 if you have the <errno.h> header file. */
#define CDIO_HAVE_ERRNO_H 1

/* Define to 1 if you have the <fcntl.h> header file. */
#define CDIO_HAVE_FCNTL_H 1

/* Define 1 if you have FreeBSD CD-ROM support */
/* #undef HAVE_FREEBSD_CDROM */

/* Define to 1 if you have the `fseeko' function. */
#define CDIO_HAVE_FSEEKO 1

/* Define to 1 if you have the `fseeko64' function. */
#define CDIO_HAVE_FSEEKO64 1

/* Define to 1 if you have the `ftruncate' function. */
#define CDIO_HAVE_FTRUNCATE 1

/* Define to 1 if you have the `geteuid' function. */
/* #undef HAVE_GETEUID */

/* Define to 1 if you have the `getgid' function. */
/* #undef HAVE_GETGID */

/* Define to 1 if you have the <getopt.h> header file. */
#define CDIO_HAVE_GETOPT_H 1

/* Define to 1 if you have the `getpwuid' function. */
/* #undef HAVE_GETPWUID */

/* Define to 1 if you have the `gettimeofday' function. */
#define CDIO_HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the `getuid' function. */
/* #undef HAVE_GETUID */

/* Define to 1 if you have the <glob.h> header file. */
/* #undef HAVE_GLOB_H */

/* Define to 1 if you have the `gmtime_r' function. */
/* #undef HAVE_GMTIME_R */

/* Define if you have the iconv() function and it works. */
#define CDIO_HAVE_ICONV 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define CDIO_HAVE_INTTYPES_H 1

/* Define to 1 if you have the <IOKit/IOKitLib.h> header file. */
/* #undef HAVE_IOKIT_IOKITLIB_H */

/* Supports ISO _Pragma() macro */
#define CDIO_HAVE_ISOC99_PRAGMA /**/

/* Define 1 if you want ISO-9660 Joliet extension support. On most platforms,
   this requires libiconv to be installed. */
#define CDIO_HAVE_JOLIET 1

/* Define this if your libcurses has keypad */
#define CDIO_HAVE_KEYPAD /**/

/* Define if you have <langinfo.h> and nl_langinfo(CODESET). */
/* #undef HAVE_LANGINFO_CODESET */

/* Define to 1 if you have the `nsl' library (-lnsl). */
/* #undef HAVE_LIBNSL */

/* Define to 1 if you have the `socket' library (-lsocket). */
/* #undef HAVE_LIBSOCKET */

/* Define to 1 if you have the <limits.h> header file. */
#define CDIO_HAVE_LIMITS_H 1

/* Define 1 if you have Linux-type CD-ROM support */
/* #undef HAVE_LINUX_CDROM */

/* Define to 1 if you have the <linux/cdrom.h> header file. */
/* #undef HAVE_LINUX_CDROM_H */

/* Define 1 if timeout is in cdrom_generic_command struct */
/* #undef HAVE_LINUX_CDROM_TIMEOUT */

/* Define to 1 if you have the <linux/major.h> header file. */
/* #undef HAVE_LINUX_MAJOR_H */

/* Define to 1 if you have the <linux/version.h> header file. */
/* #undef HAVE_LINUX_VERSION_H */

/* Define to 1 if you have the `localtime_r' function. */
/* #undef HAVE_LOCALTIME_R */

/* Define to 1 if you have the `lseek64' function. */
#define CDIO_HAVE_LSEEK64 1

/* Define to 1 if you have the `lstat' function. */
/* #undef HAVE_LSTAT */

/* Define to 1 if you have the `memcpy' function. */
#define CDIO_HAVE_MEMCPY 1

/* Define to 1 if you have the <memory.h> header file. */
#define CDIO_HAVE_MEMORY_H 1

/* Define to 1 if you have the `memset' function. */
#define CDIO_HAVE_MEMSET 1

/* Define to 1 if you have the `mkstemp' function. */
#define CDIO_HAVE_MKSTEMP 1

/* Define to 1 if you have the <ncurses.h> header file. */
/* #undef HAVE_NCURSES_H */

/* Define to 1 if you have the <ncurses/ncurses.h> header file. */
/* #undef HAVE_NCURSES_NCURSES_H */

/* Define 1 if you have NetBSD CD-ROM support */
/* #undef HAVE_NETBSD_CDROM */

/* Define to 1 if you have the <ntddcdrm.h> header file. */
#define CDIO_HAVE_NTDDCDRM_H 1

/* Define to 1 if you have the <ntddscsi.h> header file. */
#define CDIO_HAVE_NTDDSCSI_H 1

/* Define 1 if you have OS/2 CD-ROM support */
/* #undef HAVE_OS2_CDROM */

/* Define to 1 if you have the <pwd.h> header file. */
/* #undef HAVE_PWD_H */

/* Define to 1 if you have the `rand' function. */
#define CDIO_HAVE_RAND 1

/* Define to 1 if you have the `readlink' function. */
/* #undef HAVE_READLINK */

/* Define to 1 if you have the `realpath' function. */
/* #undef HAVE_REALPATH */

/* Define 1 if you want ISO-9660 Rock-Ridge extension support. */
#define CDIO_HAVE_ROCK 1

/* Define to 1 if you have the `setegid' function. */
/* #undef HAVE_SETEGID */

/* Define to 1 if you have the `setenv' function. */
/* #undef HAVE_SETENV */

/* Define to 1 if you have the `seteuid' function. */
/* #undef HAVE_SETEUID */

/* Define to 1 if you have the `sleep' function. */
#define CDIO_HAVE_SLEEP 1

/* Define to 1 if you have the `snprintf' function. */
#define CDIO_HAVE_SNPRINTF 1

/* Define 1 if you have Solaris CD-ROM support */
/* #undef HAVE_SOLARIS_CDROM */

/* Define to 1 if you have the <stdarg.h> header file. */
#define CDIO_HAVE_STDARG_H 1

/* Define to 1 if you have the <stdbool.h> header file. */
#define CDIO_HAVE_STDBOOL_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define CDIO_HAVE_STDINT_H 1

/* Define to 1 if you have the <stdio.h> header file. */
#define CDIO_HAVE_STDIO_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define CDIO_HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define CDIO_HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define CDIO_HAVE_STRING_H 1

/* Define to 1 if you have the `strndup' function. */
/* #undef HAVE_STRNDUP */

/* Define this if you have struct timespec */
#define CDIO_HAVE_STRUCT_TIMESPEC /**/

/* Define to 1 if you have the <sys/cdio.h> header file. */
/* #undef HAVE_SYS_CDIO_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#define CDIO_HAVE_SYS_PARAM_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define CDIO_HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/timeb.h> header file. */
#define CDIO_HAVE_SYS_TIMEB_H 1

/* Define to 1 if you have the <sys/time.h> header file. */
#define CDIO_HAVE_SYS_TIME_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define CDIO_HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <sys/utsname.h> header file. */
/* #undef HAVE_SYS_UTSNAME_H */

/* Define this <sys/stat.h> defines S_ISLNK() */
/* #undef HAVE_S_ISLNK */

/* Define this <sys/stat.h> defines S_ISSOCK() */
/* #undef HAVE_S_ISSOCK */

/* Define 1 if timegm is available */
/* #undef HAVE_TIMEGM */

/* Define if you have an extern long timenzone variable. */
#define CDIO_HAVE_TIMEZONE_VAR 1

/* Define if struct tm has the tm_gmtoff member. */
/* #undef HAVE_TM_GMTOFF */

/* Define if time.h defines extern extern char *tzname[2] variable */
/* #undef HAVE_TZNAME */

/* Define to 1 if you have the `tzset' function. */
#define CDIO_HAVE_TZSET 1

/* Define to 1 if you have the <unistd.h> header file. */
#define CDIO_HAVE_UNISTD_H 1

/* Define to 1 if you have the `unsetenv' function. */
/* #undef HAVE_UNSETENV */

/* Define to 1 if you have the `usleep' function. */
#define CDIO_HAVE_USLEEP 1

/* Define this if you have libvcdinfo installed */
/* #undef HAVE_VCDINFO */

/* Define to 1 if you have the `vsnprintf' function. */
#define CDIO_HAVE_VSNPRINTF 1

/* Define 1 if you have MinGW CD-ROM support */
#define CDIO_HAVE_WIN32_CDROM 1

/* Define to 1 if you have the <windows.h> header file. */
#define CDIO_HAVE_WINDOWS_H 1

/* Define to 1 if you have the `_stati64' function. */
#define CDIO_HAVE__STATI64 1

/* Define as const if the declaration of iconv() needs const. */
#define CDIO_ICONV_CONST 

/* Is set when libcdio's config.h has been included. Applications wishing to
   sue their own config.h values (such as set by the application's configure
   script can define this before including any of libcdio's headers. */
#define CDIO_LIBCDIO_CONFIG_H 1

/* Full path to libcdio top_sourcedir. */
/* #undef LIBCDIO_SOURCE_PATH */

/* Define if running the test suite so that test #27 works on MinGW. */
/* #undef LT_MINGW_STATIC_TESTSUITE_HACK */

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define CDIO_LT_OBJDIR ".libs/"

/* Define 1 if you are compiling using MinGW */
#define CDIO_MINGW32 1

/* Define this if mingw-w64 headers are used */
#define CDIO_MINGW_W64_HEADERS /**/

/* Define 1 if you need timezone defined to get timzone defined as a variable.
   In cygwin it is a function too */
/* #undef NEED_TIMEZONEVAR */

/* Name of package */
#define CDIO_PACKAGE "libcdio"

/* Define to the address where bug reports for this package should be sent. */
#define CDIO_PACKAGE_BUGREPORT "https://savannah.gnu.org/bugs/?group=libcdio"

/* Define to the full name of this package. */
#define CDIO_PACKAGE_NAME "libcdio"

/* Define to the full name and version of this package. */
#define CDIO_PACKAGE_STRING "libcdio 0.94"

/* Define to the one symbol short name of this package. */
#define CDIO_PACKAGE_TARNAME "libcdio"

/* Define to the home page for this package. */
#define CDIO_PACKAGE_URL ""

/* Define to the version of this package. */
#define CDIO_PACKAGE_VERSION "0.94"

/* Define to 1 if you have the ANSI C header files. */
#define CDIO_STDC_HEADERS 1

/* Enable extensions on AIX 3, Interix.  */
#ifndef _ALL_SOURCE
# define CDIO__ALL_SOURCE 1
#endif
/* Enable GNU extensions on systems that have them.  */
#ifndef _GNU_SOURCE
# define CDIO__GNU_SOURCE 1
#endif
/* Enable threading extensions on Solaris.  */
#ifndef _POSIX_PTHREAD_SEMANTICS
# define CDIO__POSIX_PTHREAD_SEMANTICS 1
#endif
/* Enable extensions on HP NonStop.  */
#ifndef _TANDEM_SOURCE
# define CDIO__TANDEM_SOURCE 1
#endif
/* Enable general extensions on Solaris.  */
#ifndef __EXTENSIONS__
# define CDIO___EXTENSIONS__ 1
#endif


/* Version number of package */
#define CDIO_VERSION "0.94"

/* Define WORDS_BIGENDIAN to 1 if your processor stores words with the most
   significant byte first (like Motorola and SPARC, unlike Intel). */
#if defined AC_APPLE_UNIVERSAL_BUILD
# if defined __BIG_ENDIAN__
#  define CDIO_WORDS_BIGENDIAN 1
# endif
#else
# ifndef WORDS_BIGENDIAN
/* #  undef WORDS_BIGENDIAN */
# endif
#endif

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define CDIO__DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
#define CDIO__FILE_OFFSET_BITS 64

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define to 1 if on MINIX. */
/* #undef _MINIX */

/* Define to 2 if the system does not provide POSIX.1 features except with
   this defined. */
/* #undef _POSIX_1_SOURCE */

/* Define to 1 if you need to in order for `stat' and other things to work. */
/* #undef _POSIX_SOURCE */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif
#endif /* #ifndef CDIO_CONFIG_H */
