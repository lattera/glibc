#ifdef __STDC__
#define FUNC__(name)		\
	.align 3;		\
        .globl __##name;	\
        .ent __##name;		\
        __##name:		\
	lda sp, -16(sp);	\
	.frame sp, 16, t9, 0;	\
	.prologue 0
#else
#define FUNC__(name)		\
	.align 3;		\
        .globl __/**/name;	\
        .ent __/**/name,0;	\
        __/**/name:		\
	lda sp, -16(sp);	\
	.frame sp, 16, t9, 0;	\
	.prologue 0
#endif

#ifdef __STDC__
#define NAME__(name)	\
	__##name
#else
#define NAME__(name)	\
	__/**/name
#endif
