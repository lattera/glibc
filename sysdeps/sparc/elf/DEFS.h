#define	FUNC(name)		\
	.global name;		\
	.type name,@function;	\
	.align 4;		\
	name:
