.text
	.align	4
	.globl	___mpn_mul_1
___mpn_mul_1:
	subo	g2,0,g2
	shlo	2,g2,g4
	subo	g4,g1,g1
	subo	g4,g0,g13
	mov	0,g0

	cmpo	1,0		# clear C bit on AC.cc

Loop:	ld	(g1)[g2*4],g5
	emul	g3,g5,g6
	ld	(g13)[g2*4],g5

	addc	g0,g6,g6	# relies on that C bit is clear
	addc	0,g7,g7
	addc	g5,g6,g6	# relies on that C bit is clear
	st	g6,(g13)[g2*4]
	addc	0,g7,g0

	addo	g2,1,g2
	cmpobne	0,g2,Loop	# when branch is taken, clears C bit

	ret
