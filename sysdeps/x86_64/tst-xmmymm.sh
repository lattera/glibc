#! /bin/sh
objpfx="$1"

tmp=$(mktemp ${objpfx}tst-xmmymm.XXXXXX)
trap 'rm -f "$tmp"' 1 2 3 15

objdump -d "${objpfx}ld.so" |
awk 'BEGIN { last="" } /^[[:xdigit:]]* <[_[:alnum:]]*>:$/ { fct=substr($2, 2, length($2)-3) } /,%[xy]mm[[:digit:]]*$/ { if (last != fct) { print fct; last=fct} }' |
tee "$tmp"

echo "Functions which incorrectly modify xmm/ymm registers:"
err=1
egrep -vs '^_dl_runtime_profile$' "$tmp" || err=0
if test $err -eq 0; then echo "None"; fi

rm "$tmp"
exit $err
