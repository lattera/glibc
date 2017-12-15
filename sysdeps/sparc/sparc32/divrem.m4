/*
 * Division and remainder, from Appendix E of the Sparc Version 8
 * Architecture Manual, with fixes from Gordon Irlam.
 */

/*
 * Input: dividend and divisor in %o0 and %o1 respectively.
 *
 * m4 parameters:
 *  NAME	name of function to generate
 *  OP		OP=div => %o0 / %o1; OP=rem => %o0 % %o1
 *  S		S=true => signed; S=false => unsigned
 *
 * Algorithm parameters:
 *  N		how many bits per iteration we try to get (4)
 *  WORDSIZE	total number of bits (32)
 *
 * Derived constants:
 *  TOPBITS	number of bits in the top `decade' of a number
 *
 * Important variables:
 *  Q		the partial quotient under development (initially 0)
 *  R		the remainder so far, initially the dividend
 *  ITER	number of main division loop iterations required;
 *		equal to ceil(log2(quotient) / N).  Note that this
 *		is the log base (2^N) of the quotient.
 *  V		the current comparand, initially divisor*2^(ITER*N-1)
 *
 * Cost:
 *  Current estimate for non-large dividend is
 *	ceil(log2(quotient) / N) * (10 + 7N/2) + C
 *  A large dividend is one greater than 2^(31-TOPBITS) and takes a
 *  different path, as the upper bits of the quotient must be developed
 *  one bit at a time.
 */

define(N, `4')dnl
define(WORDSIZE, `32')dnl
define(TOPBITS, eval(WORDSIZE - N*((WORDSIZE-1)/N)))dnl
dnl
define(dividend, `%o0')dnl
define(divisor, `%o1')dnl
define(Q, `%o2')dnl
define(R, `%o3')dnl
define(ITER, `%o4')dnl
define(V, `%o5')dnl
dnl
dnl m4 reminder: ifelse(a,b,c,d) => if a is b, then c, else d
define(T, `%g1')dnl
define(SC, `%g2')dnl
ifelse(S, `true', `define(SIGN, `%g3')')dnl

dnl
dnl This is the recursive definition for developing quotient digits.
dnl
dnl Parameters:
dnl  $1	the current depth, 1 <= $1 <= N
dnl  $2	the current accumulation of quotient bits
dnl  N	max depth
dnl
dnl We add a new bit to $2 and either recurse or insert the bits in
dnl the quotient.  R, Q, and V are inputs and outputs as defined above;
dnl the condition codes are expected to reflect the input R, and are
dnl modified to reflect the output R.
dnl
define(DEVELOP_QUOTIENT_BITS,
`	! depth $1, accumulated bits $2
	bl	LOC($1.eval(2**N+$2))
	srl	V,1,V
	! remainder is positive
	subcc	R,V,R
	ifelse($1, N,
	`	b	9f
		add	Q, ($2*2+1), Q
', `	DEVELOP_QUOTIENT_BITS(incr($1), `eval(2*$2+1)')')
LOC($1.eval(2**N+$2)):
	! remainder is negative
	addcc	R,V,R
	ifelse($1, N,
	`	b	9f
		add	Q, ($2*2-1), Q
', `	DEVELOP_QUOTIENT_BITS(incr($1), `eval(2*$2-1)')')
ifelse($1, 1, `9:')')dnl

#include <sysdep.h>
#include <sys/trap.h>

ENTRY(NAME)
ifelse(S, `true',
`	! compute sign of result; if neither is negative, no problem
	orcc	divisor, dividend, %g0	! either negative?
	bge	2f			! no, go do the divide
ifelse(OP, `div',
`	xor	divisor, dividend, SIGN	! compute sign in any case',
`	mov	dividend, SIGN		! sign of remainder matches dividend')
	tst	divisor
	bge	1f
	tst	dividend
	! divisor is definitely negative; dividend might also be negative
	bge	2f			! if dividend not negative...
	sub	%g0, divisor, divisor	! in any case, make divisor nonneg
1:	! dividend is negative, divisor is nonnegative
	sub	%g0, dividend, dividend	! make dividend nonnegative
2:
')
	! Ready to divide.  Compute size of quotient; scale comparand.
	orcc	divisor, %g0, V
	bne	1f
	mov	dividend, R

		! Divide by zero trap.  If it returns, return 0 (about as
		! wrong as possible, but that is what SunOS does...).
		ta	ST_DIV0
		retl
		clr	%o0

1:
	cmp	R, V			! if divisor exceeds dividend, done
	blu	LOC(got_result)		! (and algorithm fails otherwise)
	clr	Q
	sethi	%hi(1 << (WORDSIZE - TOPBITS - 1)), T
	cmp	R, T
	blu	LOC(not_really_big)
	clr	ITER

	! `Here the dividend is >= 2**(31-N) or so.  We must be careful here,
	! as our usual N-at-a-shot divide step will cause overflow and havoc.
	! The number of bits in the result here is N*ITER+SC, where SC <= N.
	! Compute ITER in an unorthodox manner: know we need to shift V into
	! the top decade: so do not even bother to compare to R.'
	1:
		cmp	V, T
		bgeu	3f
		mov	1, SC
		sll	V, N, V
		b	1b
		add	ITER, 1, ITER

	! Now compute SC.
	2:	addcc	V, V, V
		bcc	LOC(not_too_big)
		add	SC, 1, SC

		! We get here if the divisor overflowed while shifting.
		! This means that R has the high-order bit set.
		! Restore V and subtract from R.
		sll	T, TOPBITS, T	! high order bit
		srl	V, 1, V		! rest of V
		add	V, T, V
		b	LOC(do_single_div)
		sub	SC, 1, SC

	LOC(not_too_big):
	3:	cmp	V, R
		blu	2b
		nop
		be	LOC(do_single_div)
		nop
	/* NB: these are commented out in the V8-Sparc manual as well */
	/* (I do not understand this) */
	! V > R: went too far: back up 1 step
	!	srl	V, 1, V
	!	dec	SC
	! do single-bit divide steps
	!
	! We have to be careful here.  We know that R >= V, so we can do the
	! first divide step without thinking.  BUT, the others are conditional,
	! and are only done if R >= 0.  Because both R and V may have the high-
	! order bit set in the first step, just falling into the regular
	! division loop will mess up the first time around.
	! So we unroll slightly...
	LOC(do_single_div):
		subcc	SC, 1, SC
		bl	LOC(end_regular_divide)
		nop
		sub	R, V, R
		mov	1, Q
		b	LOC(end_single_divloop)
		nop
	LOC(single_divloop):
		sll	Q, 1, Q
		bl	1f
		srl	V, 1, V
		! R >= 0
		sub	R, V, R
		b	2f
		add	Q, 1, Q
	1:	! R < 0
		add	R, V, R
		sub	Q, 1, Q
	2:
	LOC(end_single_divloop):
		subcc	SC, 1, SC
		bge	LOC(single_divloop)
		tst	R
		b,a	LOC(end_regular_divide)

LOC(not_really_big):
1:
	sll	V, N, V
	cmp	V, R
	bleu	1b
	addcc	ITER, 1, ITER
	be	LOC(got_result)
	sub	ITER, 1, ITER

	tst	R	! set up for initial iteration
LOC(divloop):
	sll	Q, N, Q
	DEVELOP_QUOTIENT_BITS(1, 0)
LOC(end_regular_divide):
	subcc	ITER, 1, ITER
	bge	LOC(divloop)
	tst	R
	bl,a	LOC(got_result)
	! non-restoring fixup here (one instruction only!)
ifelse(OP, `div',
`	sub	Q, 1, Q
', `	add	R, divisor, R
')

LOC(got_result):
ifelse(S, `true',
`	! check to see if answer should be < 0
	tst	SIGN
	bl,a	1f
	ifelse(OP, `div', `sub %g0, Q, Q', `sub %g0, R, R')
1:')
	retl
	ifelse(OP, `div', `mov Q, %o0', `mov R, %o0')

END(NAME)
ifelse(OP, `div', ifelse(S, `false', `strong_alias (.udiv, __wrap_.udiv)
'))dnl
