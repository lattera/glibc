.section ".ctors",#alloc,#write

	.align 8
__CTOR_LIST__:
	.xword	-1

.section ".dtors",#alloc,#write

	.align 8
__DTOR_LIST__:
	.xword	-1

.section ".fini",#alloc,#execinstr

	call	__do_global_dtors_aux
	 nop

.text

	.align 4
	.type __do_global_dtors_aux,#function
__do_global_dtors_aux:
	save	%sp,-160,%sp

#ifdef PIC
1:	rd	%pc, %g1
	sethi	%hi(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
	or	%l7, %lo(_GLOBAL_OFFSET_TABLE_-(1b-.)), %l7
	add	%l7, %g1, %l7
	sethi	%hi(__DTOR_LIST__), %l0
	or	%l0, %lo(__DTOR_LIST__), %l0
	ldx	[%l7+%l0], %l0
#else
	sethi	%hi(__DTOR_LIST__), %l0
	or	%l0, %lo(__DTOR_LIST__), %l0
	add	%l0, %g4, %l0
#endif

	ba	3f
	 ldx	[%l0+8], %l1
2:	jmpl	%l1, %o7
	 ldx	[%l0+8], %l1
3:	brnz,pt	%l1, 2b
	 add	%l0, 8, %l0

	ret
	 restore

	.size __do_global_dtors_aux,.-__do_global_dtors_aux
