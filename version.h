#ifndef _VERSION_H_
#define _VERSION_H_

#define PDK_PHOST4_SUPPORT
#define PDK_HOST999_SUPPORT

/* This defines the version of VGAP host that we are claiming compatibility
   with */

#define HOST_VERSION_MAJOR 3
#define HOST_VERSION_MINOR 2

/* Major and minor host versions are used to indicate when data file
   format incompatibilities will occur. A major version change indicates
   a file incompatibility somewhere in the system (not necessarily in all
   data files). Thus, version 2.15 will be data-file compatible with 2.XX
   but not with 1.XX nor 3.XX. */

#ifndef PDK_PHOST4_SUPPORT
/* A change in PHOST_VERSION_MAJOR invalidates all previous data file
   formats. */
#  define PHOST_VERSION_MAJOR 3

/* A change in PHOST_VERSION_MINOR does *not* invalidate data file formats */
#  define PHOST_VERSION_MINOR 4
#else
#  define PHOST_VERSION_MAJOR 4
#  define PHOST_VERSION_MINOR 0
#endif

/* PDK Version */
#define PDK_VERSION_MAJOR 4
#define PDK_VERSION_MINOR 4

#endif /* _VERSION_H_ */
