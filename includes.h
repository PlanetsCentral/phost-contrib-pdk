#ifndef _INCLUDES_H_
#define _INCLUDES_H_

#include <stdio.h>
#include "squishio.h"
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <stdarg.h>
#include <stddef.h>
#include <math.h>
#include <time.h>
#ifdef __alpha
#include <sys/limits.h>
#include <sys/errno.h>
#else
#include <limits.h>
#include <errno.h>
#endif

#include "conf.h"

/* Function prototypes for systems that don't declare them. Can you
 * believe some of these?
 */
#ifndef HAVE_ANSI_INCLUDES
#ifdef __cplusplus
extern "C" {
#endif
  extern int printf(const char *str, ...);
  extern int fprintf(FILE * f, const char *str, ...);
  extern int vfprintf(FILE * f, const char *str, va_list ap);
  extern int vsprintf(char *s, const char *format, va_list ap);
  extern int sscanf(char *, const char *, ...);
  extern int fread(void *buf, size_t num, size_t size, FILE * f);
  extern int fwrite(const void *buf, size_t num, size_t size, FILE * f);
  extern int fclose(FILE * fil);
  extern int fseek(FILE * stream, long offset, int origin);
  extern int fputc(int c, FILE * stream);
  extern int fgetc(FILE * stream);
  extern int fflush(FILE * stream);
  extern void rewind(FILE * stream);
  extern void bcopy(const void *src, void *dst, int size);
  extern char *strerror(int errnum);
  extern void *memmove(void *dst, const void *src, size_t size);
  extern time_t time(time_t * timePtr);
  extern size_t strftime(char *s, size_t smax, const char *fmt,
        const struct tm *tp);
  extern double difftime(time_t time2, time_t time1);
  extern int atexit(void (*func) (void));
  extern long strtol(const char *str, char **endp, int radix);
  extern int toupper(int ch);
  extern int tolower(int ch);
  extern int rename(const char *old, const char *new);
  extern char *getwd(char *path);
#ifdef __cplusplus
}
#endif
#endif                          /* HAVE_ANSI_INCLUDES */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __bsdi__
#include <unistd.h>
#endif

#ifdef __linux__
#include <unistd.h>
#endif

#ifdef __alpha
#include <unistd.h>
#endif

#ifdef _AIX
#include <unistd.h>
#endif

#ifdef __hppa__
#include <unistd.h>
#endif

#if defined SUNOS
#include <unistd.h>

#define atexit(func) on_exit(func, 0)
  extern int on_exit(void (*proc) (), char *);
#define remove unlink
#define difftime(t2,t1) ((t2)-(t1))
#endif                          /* SUNOS */

#ifdef __MSDOS__
#  define HUGE __huge
#else
#  undef HUGE
#  define HUGE
#endif                          /* __MSDOS__ */

  extern int stricmp(const char *s1, const char *s2);
  extern char *strupr(char *s);

  extern const char *SystemName(int syscode);

#ifdef __cplusplus
}
#endif

/* min and max are not ANSI */
#if !defined(min) && !defined(HAVE_MINMAX)
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#if !defined(max) && !defined(HAVE_MINMAX)
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif
#ifndef HAVE_SGN
#define sgn(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
#endif
#define fround(x) ((fabs(x)+0.5)*sgn(x))
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

/* Borland C++ defines offsetof in a way that the C++ compiler doesn't like.
   This way is a bit more cumbersome but at least it works */
#ifdef offsetof
#undef offsetof
#endif
#define offsetof(struc, memb) ((size_t) ((char *) &(((struc *)0)->memb) - (char *)0))

/* If we've gotten this far, we expect certain things. Here is where we
   check for them. */
#if !defined(LITTLE_ENDIAN) && !defined(BIG_ENDIAN)
#error "You must indicate whether this machine is LITTLE_ENDIAN or BIG_ENDIAN"
#endif
#if !defined(SYSTEM_ID)
#error "You must define a SYSTEM_ID string to identify your system"
#endif
#if !defined(SYSTEM_CODE)
#error "You must define a unique SYSTEM_CODE number to identify your system"
#endif
#endif                          /* _INCLUDES_H_ */
