dnl NOTE: The $1 below is the argument to EXTEND, not register $1.
define(EXTEND,
`ifelse(SIZE, `l',
`ifelse(SIGNED, `true',
`	sextl $1, $1
',dnl
`	zapnot $1, 0xf, $1
')')')dnl

dnl FULLEXTEND -- extend the register named in the first argument
define(FULLEXTEND,
`ifelse(SIZE, `l',
`	sextl $1, $1
')')dnl

dnl This is used by divqu.
define(ADJQU,
`ifelse(MODE, `qu',
`	ldit	$f26, 18446744073709551616.0
	addt	$f26, $1, $f26
	fcmovlt	$1, $f26, $1
')')dnl

define(DOREM,
`ifelse(BASEOP, `rem',
`	! Compute the remainder.
ifelse(SIZE, `l',
`	mull t11, t12, t11
	subl t10, t11, t12
',dnl Note mulq/subq were only really used in remq, but we will find out
dnl   if assuming they apply to remqu as well is wrong or not.
`	mulq t11, t12, t11
	subq t10, t11, t12
')')')dnl
