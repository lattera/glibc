#! /bin/sh

# Usage: make-syscalls.sh ../sysdeps/unix/common
# Expects $sysdirs in environment.

thisdir=$1; shift

# Get the list of system calls for this directory.
calls=`sed 's/#.*$//
/^[ 	]*$/d' $thisdir/syscalls.list`

# Check each sysdep dir with higher priority than this one,
# and remove from $calls all the functions found in other dirs.
for dir in $sysdirs; do

  # Punt when we reach the directory defining these syscalls.
  test $dir = $thisdir && break

  # Remove each syscall that is implemented by a file in $dir.
  # If a syscall specified a "caller", then only compile that syscall
  # if the caller function is also implemented in this directory.
  calls=`echo "$calls" | while read file caller rest; do
	   test -f $dir/$file.c && continue
	   test -f $dir/$file.S && continue
	   test -f $dir/$file.s && continue
	   if test x$caller != x-; then
	     test -f $dir/$caller.c && continue
	     test -f $dir/$caller.S && continue
	     test -f $dir/$caller.s && continue
	   fi
	   echo $file $caller $rest
         done`

done

# Any calls left?
test -n "$calls" || exit 0

files=

# Emit rules to compile the syscalls remaining in $calls.
echo "$calls" | while read file caller syscall nargs strong weak; do

  # Figure out if $syscall is defined with a number in syscall.h.
  $asm_CPP - << EOF | grep "^@@@ .*$syscall" >/dev/null && continue
#include <sysdep.h>
@@@ SYS_ify ($syscall)
EOF

  case $weak in
  *@*)
    # The versioned symbols are only in the shared library.
    echo "ifneq (,\$(filter .os,\$(object-suffixes)))"
    ;;
  esac

  # Make sure only the first syscall rule is used, if multiple dirs
  # define the same syscall.
  echo "ifeq (,\$(filter $file,\$(unix-syscalls)))"

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
  echo "\$(common-objpfx)s-proto.d
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

  echo endif

  case $weak in
  *@*)
    # The versioned symbols are only in the shared library.
    echo endif
    ;;
  esac

done
