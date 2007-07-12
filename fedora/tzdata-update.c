#ifdef __sparc__
register void *__thread_self __asm ("g7");
#endif
#define _GNU_SOURCE 1
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <unistd.h>

#if defined __i386__
# define INTERNAL_SYSCALL_DECL(err) do { } while (0)
# define INTERNAL_SYSCALL(name, err, nr, args...) \
  ({									      \
    register unsigned int resultvar;					      \
    asm volatile (							      \
    "movl %1, %%eax\n\t"						      \
    "int $0x80\n\t"							      \
    : "=a" (resultvar)							      \
    : "i" (__NR_##name) ASMFMT_##nr(args) : "memory", "cc");		      \
    (int) resultvar; })
# define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned int) (val) >= 0xfffff001u)
# define ASMFMT_0()
# define ASMFMT_1(arg1) \
	, "b" (arg1)
# define ASMFMT_2(arg1, arg2) \
	, "b" (arg1), "c" (arg2)
# define ASMFMT_3(arg1, arg2, arg3) \
	, "b" (arg1), "c" (arg2), "d" (arg3)
#elif defined __x86_64__
# define INTERNAL_SYSCALL_DECL(err) do { } while (0)
# define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
  ({									      \
    unsigned long resultvar;						      \
    LOAD_ARGS_##nr (args)						      \
    LOAD_REGS_##nr							      \
    asm volatile (							      \
    "syscall\n\t"							      \
    : "=a" (resultvar)							      \
    : "0" (name) ASM_ARGS_##nr : "memory", "cc", "r11", "cx");		      \
    (long) resultvar; })
# define INTERNAL_SYSCALL(name, err, nr, args...) \
  INTERNAL_SYSCALL_NCS (__NR_##name, err, nr, ##args)
# define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned long) (val) >= -4095L)
# define LOAD_ARGS_0()
# define LOAD_REGS_0
# define ASM_ARGS_0
# define LOAD_ARGS_1(a1)				\
  long int __arg1 = (long) (a1);			\
  LOAD_ARGS_0 ()
# define LOAD_REGS_1					\
  register long int _a1 asm ("rdi") = __arg1;		\
  LOAD_REGS_0
# define ASM_ARGS_1	ASM_ARGS_0, "r" (_a1)
# define LOAD_ARGS_2(a1, a2)				\
  long int __arg2 = (long) (a2);			\
  LOAD_ARGS_1 (a1)
# define LOAD_REGS_2					\
  register long int _a2 asm ("rsi") = __arg2;		\
  LOAD_REGS_1
# define ASM_ARGS_2	ASM_ARGS_1, "r" (_a2)
# define LOAD_ARGS_3(a1, a2, a3)			\
  long int __arg3 = (long) (a3);			\
  LOAD_ARGS_2 (a1, a2)
# define LOAD_REGS_3					\
  register long int _a3 asm ("rdx") = __arg3;		\
  LOAD_REGS_2
# define ASM_ARGS_3	ASM_ARGS_2, "r" (_a3)
#elif defined __powerpc__
# define INTERNAL_SYSCALL_DECL(err) long int err
# define INTERNAL_SYSCALL_NCS(name, err, nr, args...) \
  ({									\
    register long int r0  __asm__ ("r0");				\
    register long int r3  __asm__ ("r3");				\
    register long int r4  __asm__ ("r4");				\
    register long int r5  __asm__ ("r5");				\
    register long int r6  __asm__ ("r6");				\
    register long int r7  __asm__ ("r7");				\
    register long int r8  __asm__ ("r8");				\
    LOADARGS_##nr(name, args);						\
    __asm__ __volatile__						\
      ("sc\n\t"								\
       "mfcr  %0\n\t"							\
       : "=&r" (r0),							\
	 "=&r" (r3), "=&r" (r4), "=&r" (r5),				\
	 "=&r" (r6), "=&r" (r7), "=&r" (r8)				\
       : ASM_INPUT_##nr							\
       : "r9", "r10", "r11", "r12",					\
	 "cr0", "ctr", "memory");					\
	  err = r0;  \
    (int) r3;  \
  })
# define INTERNAL_SYSCALL(name, err, nr, args...)			\
  INTERNAL_SYSCALL_NCS (__NR_##name, err, nr, ##args)
# define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((void) (val), __builtin_expect ((err) & (1 << 28), 0))
# define LOADARGS_0(name, dummy) \
	r0 = name
# define LOADARGS_1(name, __arg1) \
	long int arg1 = (long int) (__arg1); \
	LOADARGS_0(name, 0); \
	r3 = arg1
# define LOADARGS_2(name, __arg1, __arg2) \
	long int arg2 = (long int) (__arg2); \
	LOADARGS_1(name, __arg1); \
	r4 = arg2
# define LOADARGS_3(name, __arg1, __arg2, __arg3) \
	long int arg3 = (long int) (__arg3); \
	LOADARGS_2(name, __arg1, __arg2); \
	r5 = arg3
# define ASM_INPUT_0 "0" (r0)
# define ASM_INPUT_1 ASM_INPUT_0, "1" (r3)
# define ASM_INPUT_2 ASM_INPUT_1, "2" (r4)
# define ASM_INPUT_3 ASM_INPUT_2, "3" (r5)
#elif defined __ia64__
# define DO_INLINE_SYSCALL_NCS(name, nr, args...)		\
    LOAD_ARGS_##nr (args)					\
    register long _r8 asm ("r8");				\
    register long _r10 asm ("r10");				\
    register long _r15 asm ("r15") = name;			\
    long _retval;						\
    LOAD_REGS_##nr						\
    __asm __volatile ("break 0x100000;;"			\
		      : "=r" (_r8), "=r" (_r10), "=r" (_r15)	\
			ASM_OUTARGS_##nr			\
		      : "2" (_r15) ASM_ARGS_##nr		\
		      : "memory" ASM_CLOBBERS_##nr);		\
    _retval = _r8;
# define INTERNAL_SYSCALL_DECL(err) long int err
# define INTERNAL_SYSCALL_NCS(name, err, nr, args...)	\
  ({							\
    DO_INLINE_SYSCALL_NCS (name, nr, args)		\
    err = _r10;						\
    _retval; })
# define INTERNAL_SYSCALL(name, err, nr, args...)	\
  INTERNAL_SYSCALL_NCS (__NR_##name, err, nr, ##args)
# define INTERNAL_SYSCALL_ERROR_P(val, err)	(err == -1)
# define LOAD_ARGS_0()
# define LOAD_REGS_0
# define LOAD_ARGS_1(a1)				\
  long _arg1 = (long) (a1);				\
  LOAD_ARGS_0 ()
# define LOAD_REGS_1					\
  register long _out0 asm ("out0") = _arg1;		\
  LOAD_REGS_0
# define LOAD_ARGS_2(a1, a2)				\
  long _arg2 = (long) (a2);				\
  LOAD_ARGS_1 (a1)
# define LOAD_REGS_2					\
  register long _out1 asm ("out1") = _arg2;		\
  LOAD_REGS_1
# define LOAD_ARGS_3(a1, a2, a3)			\
  long _arg3 = (long) (a3);				\
  LOAD_ARGS_2 (a1, a2)
# define LOAD_REGS_3					\
  register long _out2 asm ("out2") = _arg3;		\
  LOAD_REGS_2
# define ASM_OUTARGS_0
# define ASM_OUTARGS_1	ASM_OUTARGS_0, "=r" (_out0)
# define ASM_OUTARGS_2	ASM_OUTARGS_1, "=r" (_out1)
# define ASM_OUTARGS_3	ASM_OUTARGS_2, "=r" (_out2)
# define ASM_ARGS_0
# define ASM_ARGS_1	ASM_ARGS_0, "3" (_out0)
# define ASM_ARGS_2	ASM_ARGS_1, "4" (_out1)
# define ASM_ARGS_3	ASM_ARGS_2, "5" (_out2)
# define ASM_CLOBBERS_0	ASM_CLOBBERS_1, "out0"
# define ASM_CLOBBERS_1	ASM_CLOBBERS_2, "out1"
# define ASM_CLOBBERS_2	ASM_CLOBBERS_3, "out2"
# define ASM_CLOBBERS_3	ASM_CLOBBERS_4, "out3"
# define ASM_CLOBBERS_4	ASM_CLOBBERS_5, "out4"
# define ASM_CLOBBERS_5	ASM_CLOBBERS_6, "out5"
# define ASM_CLOBBERS_6_COMMON	, "out6", "out7",			\
  /* Non-stacked integer registers, minus r8, r10, r15.  */		\
  "r2", "r3", "r9", "r11", "r12", "r13", "r14", "r16", "r17", "r18",	\
  "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27",	\
  "r28", "r29", "r30", "r31",						\
  /* Predicate registers.  */						\
  "p6", "p7", "p8", "p9", "p10", "p11", "p12", "p13", "p14", "p15",	\
  /* Non-rotating fp registers.  */					\
  "f6", "f7", "f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",	\
  /* Branch registers.  */						\
  "b6"
# define ASM_CLOBBERS_6	ASM_CLOBBERS_6_COMMON , "b7"
#elif defined __s390__
# define INTERNAL_SYSCALL_DECL(err) do { } while (0)
# define INTERNAL_SYSCALL_DIRECT(name, err, nr, args...)		      \
  ({									      \
    DECLARGS_##nr(args)							      \
    register long _ret asm("2");					      \
    asm volatile (							      \
    "svc    %b1\n\t"							      \
    : "=d" (_ret)							      \
    : "i" (__NR_##name) ASMFMT_##nr					      \
    : "memory" );							      \
    _ret; })
# define INTERNAL_SYSCALL_SVC0(name, err, nr, args...)			      \
  ({									      \
    DECLARGS_##nr(args)							      \
    register unsigned long _nr asm("1") = (unsigned long)(__NR_##name);	      \
    register long _ret asm("2");					      \
    asm volatile (							      \
    "svc    0\n\t"							      \
    : "=d" (_ret)							      \
    : "d" (_nr) ASMFMT_##nr						      \
    : "memory" );							      \
    _ret; })
# define INTERNAL_SYSCALL(name, err, nr, args...)			      \
  (((__NR_##name) < 256) ?						      \
    INTERNAL_SYSCALL_DIRECT(name, err, nr, args) :			      \
    INTERNAL_SYSCALL_SVC0(name, err,nr, args))
# define INTERNAL_SYSCALL_ERROR_P(val, err)				      \
  ((unsigned long) (val) >= -4095UL)
# define DECLARGS_0()
# define DECLARGS_1(arg1) \
	register unsigned long gpr2 asm ("2") = (unsigned long)(arg1);
# define DECLARGS_2(arg1, arg2) \
	DECLARGS_1(arg1) \
	register unsigned long gpr3 asm ("3") = (unsigned long)(arg2);
# define DECLARGS_3(arg1, arg2, arg3) \
	DECLARGS_2(arg1, arg2) \
	register unsigned long gpr4 asm ("4") = (unsigned long)(arg3);
# define ASMFMT_0
# define ASMFMT_1 , "0" (gpr2)
# define ASMFMT_2 , "0" (gpr2), "d" (gpr3)
# define ASMFMT_3 , "0" (gpr2), "d" (gpr3), "d" (gpr4)
#elif defined __sparc__
# ifndef __arch64__
#  define __INTERNAL_SYSCALL_STRING					\
	"ta	0x10;"							\
	"bcs,a	1f;"							\
	" sub	%%g0, %%o0, %%o0;"					\
	"1:"
#  define __SYSCALL_CLOBBERS "g2", "g3", "g4", "g5", "g6",		\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"cc", "memory"
# else
#  define __INTERNAL_SYSCALL_STRING					\
	"ta	0x6d;"							\
	"bcs,a,pt %%xcc, 1f;"						\
	" sub	%%g0, %%o0, %%o0;"					\
	"1:"
#  define __SYSCALL_CLOBBERS "g2", "g3", "g4", "g5", "g6",		\
	"f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7",			\
	"f8", "f9", "f10", "f11", "f12", "f13", "f14", "f15",		\
	"f16", "f17", "f18", "f19", "f20", "f21", "f22", "f23",		\
	"f24", "f25", "f26", "f27", "f28", "f29", "f30", "f31",		\
	"f32", "f34", "f36", "f38", "f40", "f42", "f44", "f46",		\
	"f48", "f50", "f52", "f54", "f56", "f58", "f60", "f62",		\
	"cc", "memory"
# endif
#define INTERNAL_SYSCALL_DECL(err) do { } while (0)
#define INTERNAL_SYSCALL(name, err, nr, args...) \
  inline_syscall##nr(__INTERNAL_SYSCALL_STRING, __NR_##name, args)
#define INTERNAL_SYSCALL_ERROR_P(val, err) \
  ((unsigned long) (val) >= -515L)
# define inline_syscall0(string,name,dummy...)				\
({									\
	register long __o0 __asm__ ("o0");				\
	register long __g1 __asm__ ("g1") = name;			\
	__asm __volatile (string : "=r" (__g1), "=r" (__o0) :		\
			  "0" (__g1) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})
# define inline_syscall1(string,name,arg1)				\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __g1 __asm__ ("g1") = name;			\
	__asm __volatile (string : "=r" (__g1), "=r" (__o0) :		\
			  "0" (__g1), "1" (__o0) :			\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})
# define inline_syscall2(string,name,arg1,arg2)				\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __g1 __asm__ ("g1") = name;			\
	__asm __volatile (string : "=r" (__g1), "=r" (__o0) :		\
			  "0" (__g1), "1" (__o0), "r" (__o1) :		\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})
# define inline_syscall3(string,name,arg1,arg2,arg3)			\
({									\
	register long __o0 __asm__ ("o0") = (long)(arg1);		\
	register long __o1 __asm__ ("o1") = (long)(arg2);		\
	register long __o2 __asm__ ("o2") = (long)(arg3);		\
	register long __g1 __asm__ ("g1") = name;			\
	__asm __volatile (string : "=r" (__g1), "=r" (__o0) :		\
			  "0" (__g1), "1" (__o0), "r" (__o1),		\
			  "r" (__o2) :					\
			  __SYSCALL_CLOBBERS);				\
	__o0;								\
})
#elif defined __alpha__
# define INTERNAL_SYSCALL(name, err_out, nr, args...) \
	INTERNAL_SYSCALL1(name, err_out, nr, args)
# define INTERNAL_SYSCALL1(name, err_out, nr, args...)	\
	INTERNAL_SYSCALL_NCS(__NR_##name, err_out, nr, args)
# define INTERNAL_SYSCALL_NCS(name, err_out, nr, args...) \
({							\
	long _sc_ret, _sc_err;				\
	inline_syscall##nr(name, args);			\
	err_out = _sc_err;				\
	_sc_ret;					\
})
# define INTERNAL_SYSCALL_DECL(err)		long int err
# define INTERNAL_SYSCALL_ERROR_P(val, err)	err
# define inline_syscall_clobbers			\
	"$1", "$2", "$3", "$4", "$5", "$6", "$7", "$8",	\
	"$22", "$23", "$24", "$25", "$27", "$28", "memory"
# define inline_syscall_r0_asm
# define inline_syscall_r0_out_constraint	"=v"
# define inline_syscall0(name, args...)				\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_0 = name;						\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2"				\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19)					\
	   : "0"(_sc_0)						\
	   : inline_syscall_clobbers,				\
	     "$16", "$17", "$18", "$20", "$21");		\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}
# define inline_syscall1(name,arg1)				\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_0 = name;						\
	_sc_16 = (long) (arg1);					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3"				\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16)				\
	   : "0"(_sc_0), "2"(_sc_16)				\
	   : inline_syscall_clobbers,				\
	     "$17", "$18", "$20", "$21");			\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}
# define inline_syscall2(name,arg1,arg2)			\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_0 = name;						\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3 %4"			\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16), "=r"(_sc_17)		\
	   : "0"(_sc_0), "2"(_sc_16), "3"(_sc_17)		\
	   : inline_syscall_clobbers,				\
	     "$18", "$20", "$21");				\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}
# define inline_syscall3(name,arg1,arg2,arg3)			\
{								\
	register long _sc_0 inline_syscall_r0_asm;		\
	register long _sc_16 __asm__("$16");			\
	register long _sc_17 __asm__("$17");			\
	register long _sc_18 __asm__("$18");			\
	register long _sc_19 __asm__("$19");			\
								\
	_sc_0 = name;						\
	_sc_16 = (long) (arg1);					\
	_sc_17 = (long) (arg2);					\
	_sc_18 = (long) (arg3);					\
	__asm__ __volatile__					\
	  ("callsys # %0 %1 <= %2 %3 %4 %5"			\
	   : inline_syscall_r0_out_constraint (_sc_0),		\
	     "=r"(_sc_19), "=r"(_sc_16), "=r"(_sc_17),		\
	     "=r"(_sc_18)					\
	   : "0"(_sc_0), "2"(_sc_16), "3"(_sc_17),		\
	     "4"(_sc_18)					\
	   : inline_syscall_clobbers, "$20", "$21");		\
	_sc_ret = _sc_0, _sc_err = _sc_19;			\
}
#endif

char buffer[32768], data[32768];
size_t datasize;
char zonename[1024];

ssize_t
readall (int fd, void *buf, size_t len)
{
  INTERNAL_SYSCALL_DECL (err);
  size_t n = len;
  ssize_t ret;
  do
    {
      ret = INTERNAL_SYSCALL (read, err, 3, fd, buf, n);
      if (INTERNAL_SYSCALL_ERROR_P (ret, err))
	{
	  ret = -1;
	  break;
	}
      else if (ret == 0)
	break;
      buf = (char *) buf + ret;
      n -= ret;
    }
  while (n > 0);
  return ret < 0 ? ret : (ssize_t) (len - n);
}

ssize_t
writeall (int fd, const void *buf, size_t len)
{
  INTERNAL_SYSCALL_DECL (err);
  size_t n = len;
  ssize_t ret;
  do
    {
      ret = INTERNAL_SYSCALL (write, err, 3, fd, buf, n);
      if (INTERNAL_SYSCALL_ERROR_P (ret, err))
	{
	  ret = -1;
	  break;
	}
      else if (ret == 0)
	break;
      buf = (const char *) buf + ret;
      n -= ret;
    }
  while (n > 0);
  return ret < 0 ? ret : (ssize_t) (len - n);
}

void
update (const char *filename)
{
  INTERNAL_SYSCALL_DECL (err);
  long int fd = INTERNAL_SYSCALL (open, err, 2, filename, O_RDONLY);
  if (INTERNAL_SYSCALL_ERROR_P (fd, err))
    return;
  ssize_t ret = readall (fd, buffer, sizeof (buffer));
  INTERNAL_SYSCALL (close, err, 1, fd);
  if (ret <= 0 || (size_t) ret == sizeof (buffer))
    return;
  /* Don't update the file unnecessarily.  */
  if ((size_t) ret == datasize && memcmp (buffer, data, datasize) == 0)
    return;
  size_t len = strlen (filename);
  char tempfilename[len + sizeof (".tzupdate")];
  memcpy (tempfilename, filename, len);
  memcpy (tempfilename + len, ".tzupdate", sizeof (".tzupdate"));

  fd = INTERNAL_SYSCALL (open, err, 3, tempfilename, O_WRONLY | O_CREAT | O_EXCL, 0600);
  if (INTERNAL_SYSCALL_ERROR_P (fd, err))
    return;
  if (writeall (fd, data, datasize) != datasize)
    {
clean_up:
      INTERNAL_SYSCALL (unlink, err, 1, tempfilename);
      INTERNAL_SYSCALL (close, err, 1, fd);
      return;
    }
  long int sret;
  sret = INTERNAL_SYSCALL (fchmod, err, 2, fd, 0644);
  if (INTERNAL_SYSCALL_ERROR_P (sret, err))
    goto clean_up;
  INTERNAL_SYSCALL (close, err, 1, fd);

  sret = INTERNAL_SYSCALL (rename, err, 2, tempfilename, filename);
  if (INTERNAL_SYSCALL_ERROR_P (sret, err))
    INTERNAL_SYSCALL (unlink, err, 1, tempfilename);
}

int
main (int argc, char **argv)
{
  INTERNAL_SYSCALL_DECL (err);
  long int fd = INTERNAL_SYSCALL (open, err, 2, "/etc/sysconfig/clock", O_RDONLY);
  if (INTERNAL_SYSCALL_ERROR_P (fd, err))
    return 0;
  ssize_t ret = readall (fd, buffer, sizeof (buffer) - 1);
  INTERNAL_SYSCALL (close, err, 1, fd);
  if (ret <= 0 || (size_t) ret == sizeof (buffer) - 1)
    return 0;
  char *p = buffer;
  while (p != NULL)
    {
      while (*p == ' ' || *p == '\t') p++;
      if (memcmp (p, "ZONE", 4) == 0)
	{
	  p += 4;
	  while (*p == ' ' || *p == '\t') p++;
	  if (*p == '=')
	    {
	      p++;
	      while (*p == ' ' || *p == '\t') p++;
	      if (*p == '"') p++;
	      char *q = p;
	      while (strchr (" \t\n\"", *p) == NULL) p++;
	      const char path[] = "/usr/share/zoneinfo/";
	      if (p - q >= sizeof (zonename) - sizeof (path))
		return 0;
	      memcpy (zonename, path, sizeof (path) - 1);
	      memcpy (zonename + sizeof (path) - 1, q, p - q);
	      break;
	    }
	}
      p = strchr (p, '\n');
      if (p) p++;
    }
  if (*zonename == '\0')
    return 0;
  fd = INTERNAL_SYSCALL (open, err, 2, zonename, O_RDONLY);
  if (INTERNAL_SYSCALL_ERROR_P (fd, err))
    return 0;
  ret = readall (fd, data, sizeof (data));
  INTERNAL_SYSCALL (close, err, 1, fd);
  if (ret <= 0 || (size_t) ret == sizeof (data))
    return 0;
  datasize = (size_t) ret;
  update ("/etc/localtime");
  update ("/var/spool/postfix/etc/localtime");
  return 0;
}

int __libc_multiple_threads __attribute__((nocommon));
int __libc_enable_asynccancel (void) { return 0; }
void __libc_disable_asynccancel (int x) { }
void __libc_csu_init (void) { }
void __libc_csu_fini (void) { }
pid_t __fork (void) { return -1; }
char thr_buf[65536];

#ifndef __powerpc__
int __libc_start_main (int (*main) (int argc, char **argv),
		       int argc, char **argv,
		       void (*init) (void), void (*fini) (void),
		       void (*rtld_fini) (void), void * stack_end)
#else
struct startup_info
{
  void *sda_base;
  int (*main) (int, char **, char **, void *);
  int (*init) (int, char **, char **, void *);
  void (*fini) (void);
};

int __libc_start_main (int argc, char **argv, char **ev,
		       void *auxvec, void (*rtld_fini) (void),
		       struct startup_info *stinfo,
		       char **stack_on_entry)
#endif
{
#if defined __ia64__ || defined __powerpc64__
  register void *r13 __asm ("r13") = thr_buf + 32768;
  __asm ("" : : "r" (r13));
#elif defined __sparc__
  register void *g6 __asm ("g6") = thr_buf + 32768;
  __thread_self = thr_buf + 32768;
  __asm ("" : : "r" (g6), "r" (__thread_self));
#elif defined __s390__ && !defined __s390x__
  __asm ("sar %%a0,%0" : : "d" (thr_buf + 32768));
#elif defined __s390x__
  __asm ("sar %%a1,%0; srlg 0,%0,32; sar %%a0,0" : : "d" (thr_buf + 32768) : "0");
#elif defined __powerpc__ && !defined __powerpc64__
  register void *r2 __asm ("r2") = thr_buf + 32768;
  __asm ("" : : "r" (r2));
#endif
#ifdef __powerpc__
  argc = (long)*stack_on_entry;
  argv = stack_on_entry + 1;
#endif
  long ret = main (argc, argv);
  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (exit, err, 1, ret);
  return 110;
}
