/* On Linux we have stat64 available.  */
#define elf_stat	stat64
#define elf_fxstat	__fxstat64
#define elf_xstat	__xstat64
#define elf_ino_t	ino64_t
