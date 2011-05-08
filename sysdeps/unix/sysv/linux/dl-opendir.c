/* In this implementation we do not really care whether the opened
   file descriptor has the CLOEXEC bit set.  The only call happens
   long before there is a call to fork or exec.  */
#undef __ASSUME_O_CLOEXEC
#define __ASSUME_O_CLOEXEC 1
#include <opendir.c>
