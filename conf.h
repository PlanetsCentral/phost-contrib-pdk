#ifndef _CONF_H_
#define _CONF_H_

#if defined(__BCPLUSPLUS__) && (__BCPLUSPLUS__ >= 0x0540)
#undef __MSDOS__
#endif

#ifdef __DJGPP__
#define SYSTEM_ID "DJGPP 2.0/i386"
#define SYSTEM_CODE 19
#define HAVE_MEMMOVE
#undef HAVE_BCOPY
#define HAVE_STRERROR
#undef HAVE_SGN
#define HAVE_STRICMP
#define HAVE_STRUPR
#define HAVE_STRDUP
#define LITTLE_ENDIAN
#define HAVE_ANSI_INCLUDES
#define FILES_HAVE_CRLF
#endif

#ifdef _SCO_COFF
# define SYSTEM_ID "SCO Unix 3.2 i386"
# define SYSTEM_CODE 16
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# undef  HAVE_STRICMP
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# undef  LITTLE_ENDIAN
# undef  BIG_ENDIAN
# define LITTLE_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#if defined(__DGUX__) && defined(__i386__)
# define SYSTEM_ID "i386 DG/UX R4.11"
# define SYSTEM_CODE 15
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# undef  HAVE_STRICMP
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# undef  LITTLE_ENDIAN
# undef  BIG_ENDIAN
# define LITTLE_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __sgi__
# define SYSTEM_ID "SGI IRIX 5.4"
# define SYSTEM_CODE 17
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# undef  HAVE_STRICMP
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# undef  LITTLE_ENDIAN
# undef  BIG_ENDIAN
# define BIG_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#if defined(__DGUX__) && ! defined(__i386__)
# define SYSTEM_ID "mc88100 DG/UX R4.11"
# define SYSTEM_CODE 15
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# undef  HAVE_STRICMP
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# undef  LITTLE_ENDIAN
# undef  BIG_ENDIAN
# define BIG_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __NeXT__
# define SYSTEM_ID "NeXT Mach 3.0"
# define SYSTEM_CODE 14
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# undef  HAVE_STRICMP
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# undef  LITTLE_ENDIAN
# undef  BIG_ENDIAN
# define BIG_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __FreeBSD__
# define SYSTEM_ID "i386 FreeBSD 2.0"
# define SYSTEM_CODE 13
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# define HAVE_STRICMP
#define stricmp strcasecmp
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# undef  LITTLE_ENDIAN
# undef  BIG_ENDIAN
# define LITTLE_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#if defined(__bsdi__) || defined(__386BSD__)
# define SYSTEM_ID "i386 BSD/386 1.1"
# define SYSTEM_CODE 12
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# define HAVE_STRICMP
#define stricmp strcasecmp
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# undef  LITTLE_ENDIAN
# undef  BIG_ENDIAN
# define LITTLE_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef THINK_C
# define SYSTEM_ID "Power Macintosh System 7.5"
# define SYSTEM_CODE 11
# define HAVE_MEMMOVE
# undef HAVE_BCOPY
# define HAVE_STRERROR
# undef HAVE_SGN
# undef HAVE_STRICMP
# undef HAVE_STRUPR
# define HAVE_STRDUP
# define BIG_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __OS2__
# define SYSTEM_ID "PC IBM OS/2"
# define SYSTEM_CODE 10
# define HAVE_MEMMOVE
# undef HAVE_BCOPY
# define HAVE_STRERROR
# undef HAVE_SGN
# define HAVE_STRICMP
# define HAVE_STRUPR
# define HAVE_STRDUP
# define LITTLE_ENDIAN
# define HAVE_ANSI_INCLUDES
# define FILES_HAVE_CRLF
#endif

#ifdef __ultrix
# define SYSTEM_ID "DEC 5000 Ultrix"
# define SYSTEM_CODE 9
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# undef  HAVE_STRICMP
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# define LITTLE_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __linux__
#ifdef __sparc__
#define SYSTEM_ID "Linux/SPARC"
#else
#define SYSTEM_ID "PC Linux"
#endif
#define SYSTEM_CODE 8
#define HAVE_MEMMOVE
#undef  HAVE_BCOPY
#define HAVE_STRERROR
#define HAVE_STRICMP
#define stricmp strcasecmp
#undef  HAVE_STRUPR
# define HAVE_STRDUP
#undef  LITTLE_ENDIAN
#undef  BIG_ENDIAN
#ifdef __sparc__
#define BIG_ENDIAN
#else
#define LITTLE_ENDIAN
#endif
#define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __alpha
# define SYSTEM_ID "Alpha AXP OSF/1"
# define SYSTEM_CODE 7
# define HAVE_MEMMOVE
# undef  HAVE_BCOPY
# define HAVE_STRERROR
# undef  HAVE_SGN
# undef  HAVE_STRICMP
# undef  HAVE_STRUPR
# define HAVE_STRDUP
# define LITTLE_ENDIAN
# define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __sun
# ifdef __svr4__
#   define SYSTEM_ID "Sun Solaris"
#   define SYSTEM_CODE 6
#   define HAVE_MEMMOVE
#   undef HAVE_BCOPY
#   undef HAVE_STRERROR
#   undef HAVE_SGN
#   undef HAVE_STRICMP
#   undef HAVE_STRUPR
#   define HAVE_STRDUP
#   define BIG_ENDIAN
#   define HAVE_ANSI_INCLUDES
#   define SOLARIS 1
#   undef  FILES_HAVE_CRLF
# endif
#endif

#ifdef _AIX
#define SYSTEM_ID "RS/6000 AIX"
#define SYSTEM_CODE 5
#define BIG_ENDIAN
#define HAVE_MEMMOVE
#undef HAVE_BCOPY
#define HAVE_STRERROR
#undef HAVE_SGN
#undef HAVE_STRICMP
#undef HAVE_STRUPR
# define HAVE_STRDUP
#define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __hppa__
#define SYSTEM_ID "HP-PA HP-UX"
#define SYSTEM_CODE 4
#define HAVE_MEMMOVE
#undef HAVE_BCOPY
#define HAVE_STRERROR
#undef HAVE_SGN
#define HAVE_STRICMP
#define stricmp strcasecmp
#undef HAVE_STRUPR
# define HAVE_STRDUP
#define BIG_ENDIAN
#define HAVE_ANSI_INCLUDES
# undef  FILES_HAVE_CRLF
#endif

#ifdef __sun__
# ifndef __svr4__
#  define SYSTEM_ID "SPARC SunOS"
#  define SYSTEM_CODE 3
#  undef HAVE_MEMMOVE
#  define HAVE_BCOPY
#  undef HAVE_STRERROR
#  undef HAVE_SGN
#  define HAVE_STRICMP
#  undef HAVE_STRUPR
#  define HAVE_STRDUP
#  define BIG_ENDIAN
#  undef HAVE_ANSI_INCLUDES
#  define SUNOS
#  undef  FILES_HAVE_CRLF
# endif
#endif

#if defined(__WIN32__) && ! defined(__DPMI32__)
#define SYSTEM_ID "PC Win32/Windows NT"
#define SYSTEM_CODE 2
#define HAVE_MEMMOVE
#undef HAVE_BCOPY
#define HAVE_STRERROR
#undef HAVE_SGN
#define HAVE_STRICMP
#define HAVE_STRUPR
#define HAVE_STRDUP
#define LITTLE_ENDIAN
#define HAVE_ANSI_INCLUDES
#define FILES_HAVE_CRLF
#endif

#if defined(__WIN32__) && defined(__DPMI32__)
#define SYSTEM_ID "PC DOS/386"
#define SYSTEM_CODE 20
#define HAVE_MEMMOVE
#undef HAVE_BCOPY
#define HAVE_STRERROR
#undef HAVE_SGN
#define HAVE_STRICMP
#define HAVE_STRUPR
#define HAVE_STRDUP
#define LITTLE_ENDIAN
#define HAVE_ANSI_INCLUDES
#define FILES_HAVE_CRLF
#endif

#ifdef __MSDOS__
#define SYSTEM_ID "PC MS-DOS"
#define SYSTEM_CODE 1
#define HAVE_MEMMOVE
#undef HAVE_BCOPY
#define HAVE_STRERROR
#undef HAVE_SGN
#define HAVE_STRICMP
#define HAVE_STRUPR
#define HAVE_STRDUP
#define LITTLE_ENDIAN
#define HAVE_ANSI_INCLUDES
#define FILES_HAVE_CRLF
#endif

#endif /* _CONF_H_ */
