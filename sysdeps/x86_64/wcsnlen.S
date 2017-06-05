#define AS_WCSLEN
#define AS_STRNLEN
#define strlen	__wcsnlen

#include "strlen.S"

weak_alias(__wcsnlen, wcsnlen)
