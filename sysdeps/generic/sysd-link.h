/* In general we don't have stat64 available.  */
#define elf_stat	stat
#define elf_fxstat	__fxstat
#define elf_xstat	__xstat
#define elf_ino_t	ino_t
