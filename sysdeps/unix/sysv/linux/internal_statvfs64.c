#define STATFS statfs64
#define STATVFS statvfs64
#define INTERNAL_STATVFS __internal_statvfs64
#include "internal_statvfs.c"
