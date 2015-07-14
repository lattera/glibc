/* Bypass sysdeps/posix/dl-fileid.h, which relies on st_dev/st_ino being
   reliable.  Under NaCl, we cannot always expect them to be useful.
   Fortunately, in the ways NaCl is used it's far less likely that two
   different names for the same file would be used in dlopen or the like,
   so failing to notice re-opening the same file is not so likely to be a
   problem in practice.  */

#include <sysdeps/generic/dl-fileid.h>
