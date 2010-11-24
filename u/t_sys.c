/**
  *  \file t_sys.c
  *  \brief Platform Definition
  *
  *  THIS IS NOT AN AUTOMATIC TEST-CASE.  It is intended for humans
  *  to check what the PDK thinks it runs on.
  */

#include <stdio.h>
#include "conf.h"         /* This is a PDK-internal header! */

int main()
{
    printf("SYSTEM_ID = %s\n", SYSTEM_ID);
    printf("SYSTEM_CODE = %d\n", SYSTEM_CODE);
#ifdef HAVE_MEMMOVE
    printf("HAVE_MEMMOVE = defined\n");
#else
    printf("HAVE_MEMMOVE = undefined\n");
#endif

#ifdef HAVE_MEMMOVE
    printf("HAVE_MEMMOVE = defined\n");
#else
    printf("HAVE_MEMMOVE = undefined\n");
#endif

#ifdef HAVE_BCOPY
    printf("HAVE_BCOPY = defined\n");
#else
    printf("HAVE_BCOPY = undefined\n");
#endif

#ifdef HAVE_STRERROR
    printf("HAVE_STRERROR = defined\n");
#else
    printf("HAVE_STRERROR = undefined\n");
#endif

#ifdef HAVE_SGN
    printf("HAVE_SGN = defined\n");
#else
    printf("HAVE_SGN = undefined\n");
#endif

#ifdef HAVE_STRICMP
    printf("HAVE_STRICMP = defined\n");
#else
    printf("HAVE_STRICMP = undefined\n");
#endif

#ifdef HAVE_STRUPR
    printf("HAVE_STRUPR = defined\n");
#else
    printf("HAVE_STRUPR = undefined\n");
#endif

#ifdef HAVE_STRDUP
    printf("HAVE_STRDUP = defined\n");
#else
    printf("HAVE_STRDUP = undefined\n");
#endif

#ifdef PHOST_LITTLE_ENDIAN
    printf("PHOST_LITTLE_ENDIAN = defined\n");
#else
    printf("PHOST_LITTLE_ENDIAN = undefined\n");
#endif

#ifdef PHOST_BIG_ENDIAN
    printf("PHOST_BIG_ENDIAN = defined\n");
#else
    printf("PHOST_BIG_ENDIAN = undefined\n");
#endif

#ifdef PHOST_LITTLE_ENDIAN
    printf("PHOST_LITTLE_ENDIAN = defined\n");
#else
    printf("PHOST_LITTLE_ENDIAN = undefined\n");
#endif

#ifdef HAVE_ANSI_INCLUDES
    printf("HAVE_ANSI_INCLUDES = defined\n");
#else
    printf("HAVE_ANSI_INCLUDES = undefined\n");
#endif

#ifdef FILES_HAVE_CRLF
    printf("FILES_HAVE_CRLF = defined\n");
#else
    printf("FILES_HAVE_CRLF = undefined\n");
#endif

    return 0;
}
