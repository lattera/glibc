.text
	.align 4
	.globl ___mpn_add_n
___mpn_add_n:
	mov	0,g6		# clear carry-save register
	cmpo	1,0		# clear cy

Loop:	subo	1,g3,g3		# update loop counter
	ld	(g1),g5		# load from s1_ptr
	addo	4,g1,g1		# s1_ptr++
	ld	(g2),g4		# load from s2_ptr
	addo	4,g2,g2		# s2_ptr++
	cmpo	g6,1		# restore cy from g6, relies on cy being 0
	addc	g4,g5,g4	# main add
	subc	0,0,g6		# save cy in g6
	st	g4,(g0)		# store result to res_ptr
	addo	4,g0,g0		# res_ptr++
	cmpobne	0,g3,Loop	# when branch is taken, clears C bit

	mov	g6,g0
	ret
