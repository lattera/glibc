#define scandirat64 scandirat64_renamed

#include "../../dirent/scandirat.c"

#undef scandirat64
weak_alias (scandirat, scandirat64)
