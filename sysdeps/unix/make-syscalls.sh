#! /bin/sh

# Usage: make-syscalls.sh ../sysdeps/unix/common
# Expects $sysdirs in environment.

##############################################################################

# Syscall Signature Key Letters for BP Thunks:
#
# a: unchecked address (e.g., 1st arg to mmap)
# b: non-NULL buffer (e.g., 2nd arg to read)
# B: optionally-NULL buffer (e.g., 4th arg to getsockopt)
# f: buffer of 2 ints (e.g., 4th arg to socketpair)
# i: scalar (any signedness & size: int, long, long long, enum, whatever)
# n: scalar buffer length (e.g., 3rd arg to read)
# N: pointer to value/return scalar buffer length (e.g., 6th arg to recvfrom)
# p: pointer to typed object (e.g., any non-void* arg)
# P: pointer return value (e.g., return value from mmap)
# s: string (e.g., 1st arg to open)
# v: vararg scalar (e.g., optional 3rd arg to open)
# V: vararg pointer (e.g., 3rd arg to fcntl & ioctl)

ptrlet='[abBfNpPs]'
argdig='[1-9]'
fixarg='[^vV]'$argdig	# fixed args (declare extern)
strarg=s$argdig		# string arg (check with CHECK_STRING)
twoarg=f$argdig		# fd pair arg (check with CHECK_N (..., 2)
objarg=p$argdig		# object arg (check with CHECK_1)
ptrarg=$ptrlet$argdig	# pointer arg (toss bounds)
rtnarg='P'$argdig	# pointer return value (add bounds)
bufarg='[bB]'$argdig	# buffer arg (check with CHECK_N)
intarg='[inv]'$argdig	# scalar arg
borarg='[iv]'$argdig	# boring arg (just pass it through)

##############################################################################

thisdir=$1; shift

echo ''
echo \#### DIRECTORY = $thisdir
# Check each sysdep dir with higher priority than this one,
# and remove from $calls all the functions found in other dirs.
# Punt when we reach the directory defining these syscalls.
sysdirs=`for dir in $sysdirs; do
	 test $dir = $thisdir && break; echo $dir; done`
echo \#### SYSDIRS = $sysdirs

# Get the list of system calls for this directory.
calls=`sed 's/#.*$//
/^[ 	]*$/d' $thisdir/syscalls.list`

calls=`echo "$calls" |
while read file caller rest; do
  # Remove each syscall that is implemented by a file in $dir.
  # If a syscall specified a "caller", then only compile that syscall
  # if the caller function is also implemented in this directory.
  srcfile=-;
  for dir in $sysdirs; do
     { test -f $dir/$file.c && srcfile=$dir/$file.c; } ||
     { test -f $dir/$file.S && srcfile=$dir/$file.S; } ||
     { test -f $dir/$file.s && srcfile=$dir/$file.s; } ||
     { test x$caller != x- &&
	{ { test -f $dir/$caller.c && srcfile=$dir/$caller.c; } ||
	  { test -f $dir/$caller.S && srcfile=$dir/$caller.S; } ||
	  { test -f $dir/$caller.s && srcfile=$dir/$caller.s; }; }; } && break;
  done;
  echo $file $srcfile $caller $rest;
done`

# Any calls left?
test -n "$calls" || exit 0

# Emit rules to compile the syscalls remaining in $calls.
echo "$calls" |
while read file srcfile caller syscall args strong weak; do

  # Figure out if $syscall is defined with a number in syscall.h.
  callnum=-
  eval `{ echo "#include <sysdep.h>";
	echo "callnum=SYS_ify ($syscall)"; } |
	  $asm_CPP - |sed -n -e "/^callnum=.*$syscall/d" \
			     -e "/^\(callnum=\)[ 	]*\(.*\)/s//\1'\2'/p"`

  # Derive the number of arguments from the argument signature
  case $args in
  [0-9]) nargs=$args;;
  ?:) nargs=0;;
  ?:?) nargs=1;;
  ?:??) nargs=2;;
  ?:???) nargs=3;;
  ?:????) nargs=4;;
  ?:?????) nargs=5;;
  ?:??????) nargs=6;;
  ?:???????) nargs=7;;
  ?:????????) nargs=8;;
  ?:?????????) nargs=9;;
  esac

  # Make sure only the first syscall rule is used, if multiple dirs
  # define the same syscall.
  echo ''
  echo "#### CALL=$file NUMBER=$callnum ARGS=$args SOURCE=$srcfile"

 case x$srcfile"$callnum" in
 x*-) ;; ### Do nothing for undefined callnum
 x-*)
  echo "ifeq (,\$(filter $file,\$(unix-syscalls)))"

  case $weak in
  *@*)
    # The versioned symbols are only in the shared library.
    echo "ifneq (,\$(filter .os,\$(object-suffixes)))"
    ;;
  esac
  # Accumulate the list of syscall files for this directory.
  echo "unix-syscalls += $file"
  test x$caller = x- || echo "unix-extra-syscalls += $file"

  # Emit a compilation rule for this syscall.
  case $weak in
  *@*)
    # The versioned symbols are only in the shared library.
    echo "\
shared-only-routines += $file
\$(objpfx)${file}.os: \\"
    ;;
  *)
    echo "\
\$(foreach o,\$(object-suffixes),\$(objpfx)$file\$o): \\"
    ;;
  esac
  echo "		\$(common-objpfx)s-proto.d
	(echo '#include <sysdep.h>'; \\
	 echo 'PSEUDO ($strong, $syscall, $nargs)'; \\
	 echo '	ret'; \\
	 echo 'PSEUDO_END($strong)'; \\"

  # Append any weak aliases or versions defined for this syscall function.

  # A shortcoming in the current gas is that it will only allow one
  # version-alias per symbol.  So we create new strong aliases as needed.
  vcount=""

  for name in $weak; do
    case $name in
      *@@*)
	base=`echo $name | sed 's/@@.*//'`
	ver=`echo $name | sed 's/.*@@//'`
	if test -z "$vcount" ; then
	  source=$strong
	  vcount=1
	else
	  source="${strong}_${vcount}"
	  vcount=`expr $vcount + 1`
	  echo "	 echo 'strong_alias ($strong, $source)'; \\"
	fi
	echo "	 echo 'default_symbol_version($source, $base, $ver)'; \\"
	;;
      *@*)
	base=`echo $name | sed 's/@.*//'`
	ver=`echo $name | sed 's/.*@//'`
	if test -z "$vcount" ; then
	  source=$strong
	  vcount=1
	else
	  source="${strong}_${vcount}"
	  vcount=`expr $vcount + 1`
	  echo "	 echo 'strong_alias ($strong, $source)'; \\"
	fi
	echo "	 echo 'symbol_version($source, $base, $ver)'; \\"
	;;
      *)
	echo "	 echo 'weak_alias ($strong, $name)'; \\"
	;;
    esac
  done

  # And finally, pipe this all into the compiler.
  echo '	) | $(COMPILE.S) -x assembler-with-cpp -o $@ -'

  case $weak in
  *@*)
    # The versioned symbols are only in the shared library.
    echo endif
    ;;
  esac

  echo endif
 ;;
 esac

  case x"$callnum",$srcfile,$args in
  x-,-,* | x*,*.[sS],*V*) ;;
  x*,-,*$ptrlet* | x*,*.[sS],*$ptrlet*)

    # choose the name with the fewest leading underscores, preferably none
    set `echo $strong $weak |tr ' \t' '\n' |sort -r`
    callname=$1

    # convert signature string to individual numbered arg names
    # e.g., i:ipbN -> i0 i1 p2 b3 N4
    set `echo $args |
	sed -e 's/^\(.\):\(.*\)/\2 \10/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \11/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \12/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \13/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \14/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \15/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \16/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \17/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \18/' \
	    -e 's/^\([^ ]\)\(.*\)/\2 \19/'`
    rtn=$1; shift
    args=$*
    arglist=`echo $* |sed 's/ /, /g'`

    # The best way to understand what's going on here is to examine
    # the output in BUILDDIR/sysd-syscalls.

    # generate makefile envelope & rule head
    echo "ifeq (,\$(filter $file,\$(bp-thunks)))"
    echo "bp-thunks += $file"
    echo "\$(objpfx)\$(bppfx)$file.ob: \$(common-objpfx)s-proto.d"

    # generate macro head & thunk prologue
    echo "\
	(echo '#define $callname($arglist) r0, $rtn; \\'; \\
	 echo '`echo $args | \
		    sed -e 's/\('$fixarg'\)/extern \1, \1v;/g' \
			-e 's/\(v'$argdig'\)/extern int \1v;/g'` \\'; \\
	 echo '__typeof (r0) BP_SYM ($strong) (`echo $args | \
		    sed -e 's/ /, /g' \
			-e 's/\('$ptrarg'\)/__typeof (\1v) *__bounded \1a/g' \
			-e 's/\('$intarg'\)/__typeof (\1v) \1a/g'`) { \\'; \\
	 echo '  extern __typeof (r0) ($callname) (`echo $args | \
		    sed -e 's/ /, /g' \
			-e 's/\('$ptrarg'\)/__typeof (\1v) *__unbounded/g' \
			-e 's/\('$intarg'\)/__typeof (\1v)/g'`); \\'; \\"

    # generate thunk bounds checks
    for arg; do
      next=$2; shift
      case $arg in
      B$argdig) echo "	 echo '  __ptrvalue (${arg}a) && \\'; \\" ;;
      esac
      case $arg in
      n$argdig) len=$arg ;; ### save for possible use with return value.
      $strarg) echo "	 echo '  CHECK_STRING (${arg}a); \\'; \\" ;;
      $objarg) echo "	 echo '  CHECK_1 (${arg}a); \\'; \\" ;;
      $twoarg) echo "	 echo '  CHECK_N (${arg}a, 2); \\'; \\" ;;
      $bufarg)
	case $next in
	n$argdig) echo "	 echo '  CHECK_N (${arg}a, ${next}a); \\'; \\" ;;
	N$argdig) echo "	 echo '  CHECK_N (${arg}a, *CHECK_1 (${next}a)); \\'; \\" ;;
	*) echo "### BP Thunk Error: Expected length after buffer ###" ;;
	esac ;;
      esac
    done

    # generate thunk epilogue
    funcall="($callname) (`echo $args | \
		    sed -e 's/ /, /g' \
			-e 's/\('$ptrarg'\)/__ptrvalue (\1a)/g' \
			-e 's/\('$intarg'\)/\1a/g'`)"
    case $rtn in
    P*) echo "	 echo '{ __typeof ($rtn) *__bounded rtn; \\'; \\
	 echo '  __ptrlow (rtn) = __ptrvalue (rtn) = $funcall; \\'; \\
	 echo '  __ptrhigh (rtn) = __ptrlow (rtn) + ${len}a; return rtn; } \\'; \\" ;;
    *) echo "	 echo '  return $funcall; \\'; \\" ;;
    esac
    echo "	 echo '} \\'; \\"

    # generate thunk aliases
    for name in $weak; do
      case $name in
	*@*) ;;
	*) echo "	 echo 'weak_alias (BP_SYM ($strong), BP_SYM ($name)) \\'; \\" ;;
      esac
    done
    # wrap up
    echo "\
	 echo ''; \\
	 echo '#include <bp-thunks.h>'; \\
	) | \$(COMPILE.c) -x c -o \$@ -"
### Use this for debugging intermediate output:
### echo '	) >$(@:.ob=.c)
###	$(subst -c,-E,$(COMPILE.c)) -o $(@:.ob=.ib) $(@:.ob=.c)
###	$(COMPILE.c) -x cpp-output -o $@ $(@:.ob=.ib)'
    echo endif
    ;;
  esac

done
