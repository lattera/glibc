/* Copyright (C) 1994, 1996 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* From Scott Bartram.  */

#ifndef _SYSCALL_H
#define _SYSCALL_H

#define SYS_access	33
#define SYS_acct	51
#define SYS_advfs	70
#define SYS_alarm	27
#define SYS_break	17
#define SYS_brk		17
#define SYS_chdir	12
#define SYS_chmod	15
#define SYS_chown	16
#define SYS_chroot	61
#define SYS_chsize	0x0a28
#define SYS_close	6
#define SYS_creat	8
#define SYS_dup		41
#define SYS_exec	11
#define SYS_exece	59
#define SYS_exit	1
#define SYS_fcntl	62
#define SYS_fork	2
#define SYS_fpathconf	0x2f28
#define SYS_fstat	28
#define SYS_fstatfs	38
#define SYS_ftime	0x0b28
#define SYS_getdents	81
#define SYS_getgid	47
#define SYS_getgroups	0x2b28
#define SYS_getitimer	0x3728
#define SYS_getmsg	85
#define SYS_getpid	20
#define SYS_getuid	24
#define SYS_gtty	32
#define SYS_ioctl	54
#define SYS_kill	37
#define SYS_link	9
#define SYS_lock	45
#define SYS_lseek	19
#define SYS_lstat	91
#define SYS_mkdir	80
#define SYS_mknod	14
#define SYS_mount	21
#define SYS_msgsys	49
#define SYS_nap		0x0c28
#define SYS_nice	34
#define SYS_open	5
#define SYS_pathconf	0x2e28
#define SYS_pause	29
#define SYS_pgrpsys	39
#define SYS_pipe	42
#define SYS_plock	45
#define SYS_poll	87
#define SYS_prof	44
#define SYS_ptrace	26
#define SYS_putmsg	86
#define SYS_rdebug	76
#define SYS_read	3
#define SYS_readlink	92
#define SYS_rename	0x3028
#define SYS_rfstart	74
#define SYS_rfstop	77
#define SYS_rfsys	78
#define SYS_rmdir	79
#define SYS_rmount	72
#define SYS_rumount	73
#define SYS_seek	19
#define SYS_select	0x2428
#define SYS_semsys	53
#define SYS_setgid	46
#define SYS_setgroups	0x2c28
#define SYS_setitimer	0x3828
#define SYS_setpgrp	39
#define SYS_setuid	23
#define SYS_shmsys	52
#define SYS_sigaction	0x2728
#define SYS_signal	48
#define SYS_sigpending	0x2928
#define SYS_sigprocmask	0x2828
#define SYS_sigsuspend	0x2a28
#define SYS_stat	18
#define SYS_statfs	35
#define SYS_stime	25
#define SYS_stty	31
#define SYS_symlink	90
#define SYS_sync	36
#define SYS_sys3b	50
#define SYS_sysacct	51
#define SYS_sysconf	0x2d28
#define SYS_sysfs	84
#define SYS_sysi86  	50
#define SYS_time	13
#define SYS_times	43
#define SYS_uadmin	55
#define SYS_ulimit	63
#define SYS_umask	60
#define SYS_umount	22
#define SYS_unadvfs	71
#define SYS_unlink	10
#define SYS_utime	30
#define SYS_utssys	57
#define SYS_wait	7
#define SYS_write	4

#endif
