# sysctl system call has been deprecated.  It is provided for backward
# compatility.  New target shouldn't add it (see x86_64/x32/sysctl.mk).
sysdep_routines += sysctl
