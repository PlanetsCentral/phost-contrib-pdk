#ifdef MICROSQUISH
extern FILE __near __cdecl _iob[];
#undef stdin
#undef stdout
#undef stderr
#define stdin  (&_iob[0])
#define stdout (&_iob[1])
#define stderr (&_iob[2])
#endif
