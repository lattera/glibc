.text
	.align 4
	.globl ___mpn_sub_n
___mpn_sub_n:
	mov	1,g6		# set carry-save register
	cmpo	1,0		# clear cy

Loop:	subo	1,g3,g3		# update loop counter
	ld	(g1),g5		# load from s1_ptr
	addo	4,g1,g1		# s1_ptr++
	ld	(g2),g4		# load from s2_ptr
	addo	4,g2,g2		# s2_ptr++
	cmpo	g6,1		# restore cy from g6, relies on cy being 0
	subc	g4,g5,g4	# main subtract
	subc	0,0,g6		# save cy in g6
	st	g4,(g0)		# store result to res_ptr
	addo	4,g0,g0		# res_ptr++
	cmpobne	0,g3,Loop	# when branch is taken, cy will be 0

	mov	g6,g0
	ret
