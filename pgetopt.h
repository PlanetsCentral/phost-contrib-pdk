#ifndef _PGETOPT_H_
#define _PGETOPT_H_

extern char *poptarg;           /* Global argument pointer. */
extern int poptind;             /* Global argv index. */

extern int pgetopt(int argc, char *argv[], char *optstring);

#endif /* _PGETOPT_H_ */
