#! /bin/sh

# Usage: make-syscalls.sh ../sysdeps/unix/common
# Expects $sysdirs in environment.

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
echo "$calls" | while read file srcfile caller syscall args strong weak; do

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
  esac

  # Make sure only the first syscall rule is used, if multiple dirs
  # define the same syscall.
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
  x-,-,*) ;;
  x*,-,*[sp]* | x*,*.[sS],*[sp]*)
    echo "ifeq (,\$(filter $file,\$(bp-thunks)))"
    echo "bp-thunks += $file";
    echo "\
\$(objpfx)\$(bppfx)$file.ob: \$(common-objpfx)s-proto.d
	(echo '#include <bp-thunks.h>'; \\
	 echo 'BP_THUNK_`echo $args |tr : _` ($strong)'; \\"

    for name in $weak; do
      case $name in
	*@*) ;;
	*) echo "	 echo 'BP_ALIAS ($strong, $name)'; \\" ;;
      esac
    done

    echo '	) | $(COMPILE.c) -x c -o $@ -'
    echo endif
    ;;
  esac

done
