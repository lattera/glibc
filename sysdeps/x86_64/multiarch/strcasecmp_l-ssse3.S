#define USE_SSSE3 1
#define USE_AS_STRCASECMP_L
#define STRCMP __strcasecmp_l_ssse3
#define __strcasecmp __strcasecmp_ssse3
#include "../strcmp.S"
