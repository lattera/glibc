#define alphasort64 rename_alphasort64

#include "../../dirent/alphasort.c"

#undef alphasort64

weak_alias (alphasort, alphasort64)
