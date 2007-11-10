#define scandir64 scandir64_renamed

#include "../../dirent/scandir.c"

#undef scandir64
weak_alias (scandir, scandir64)
