/* We must use __syscall_ioctl since __ioctl does some extra work.  */
extern int __syscall_ioctl (int __fd, unsigned long int __request, ...);
#define __ioctl __syscall_ioctl
#include <sysdeps/unix/sysv/linux/tcgetattr.c>
