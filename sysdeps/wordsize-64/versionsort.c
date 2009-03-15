#define versionsort64 rename_versionsort64

#include "../../dirent/versionsort.c"

#undef versionsort64

weak_alias (versionsort, versionsort64)
