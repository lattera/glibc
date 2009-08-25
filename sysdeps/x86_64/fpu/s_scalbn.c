#define scalbln __renamed_scalbln
#define __scalbln __renamed___scalbln

#include <sysdeps/ieee754/dbl-64/wordsize-64/s_scalbn.c>

#undef scalbln
#undef __scalbln
strong_alias (__scalbn, __scalbln)
weak_alias (__scalbn, scalbln)
