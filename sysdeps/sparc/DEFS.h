#ifdef HAVE_ELF
# define FUNC(name)		\
	.global name;		\
	.type name,@function;	\
	.align 4;		\
	name:
#else
# define FUNC(name)	\
	.global name;	\
	.align 4;	\
	name:
#endif
