#! /bin/bash

objpfx="$1"
perl="$2"
cc="$3"
includes="$4"

scratch=${objpfx}scratch
mkdir -p "$scratch"

standards=("ISO" "ISO99" "ISO11" "POSIX" "XPG3" "XPG4" "UNIX98"
	   "XOPEN2K" "XOPEN2K8" "POSIX2008")

exitval=0
> ${objpfx}run-conformtest.out
for s in ${standards[*]}; do
    echo -n $s...
    e=0
    if ! $perl conformtest.pl --tmpdir="$scratch" --cc="$cc" --flags="$includes" --standard=$s > ${objpfx}conform-$s.out; then
	e=1
    fi
    printf "\n%s\n" $s >> ${objpfx}run-conformtest.out
    tail -n 4 ${objpfx}conform-$s.out >> ${objpfx}run-conformtest.out
    echo
    if [ $e -ne 0 ]; then
	tail -n 3 ${objpfx}conform-$s.out
	exitval=1
    fi
done

exit $exitval
