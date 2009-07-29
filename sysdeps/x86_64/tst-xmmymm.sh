#! /bin/bash
objpfx="$1"

tmp=$(mktemp ${objpfx}tst-xmmymm.XXXXXX)
trap 'rm -f "$tmp"' 1 2 3 15

# List of object files we have to test
rtldobjs=$(readelf -W -wi ${objpfx}dl-allobjs.os |
    awk '/^ </ { if ($5 == "(DW_TAG_compile_unit)") c=1; else c=0 } $2 == "DW_AT_name" { if (c == 1) print $NF }' |
    sed 's,\(.*/\|\)\([_[:alnum:]-]*[.]\).$,\2os,')
rtldobjs="$rtldobjs $(ar t ${objpfx}rtld-libc.a)"

# OBJECT symbols can be ignored.
readelf -sW ${objpfx}dl-allobjs.os ${objpfx}rtld-libc.a |
egrep " OBJECT  *GLOBAL " |
awk '{if ($7 != "ABS") print $8 }' |
sort -u > "$tmp"
declare -a objects
objects=($(cat "$tmp"))

objs="dl-runtime.os"
tocheck="dl-runtime.os"

while test -n "$objs"; do
  this="$objs"
  objs=""

  for f in $this; do
    undef=$(nm -u "$objpfx"../*/"$f" | awk '{print $2}')
    if test -n "$undef"; then
      for s in $undef; do
	for obj in ${objects[*]} "_GLOBAL_OFFSET_TABLE_"; do
	  if test "$obj" = "$s"; then
	    continue 2
	  fi
	done
        for o in $rtldobjs; do
	  ro=$(echo "$objpfx"../*/"$o")
	  if nm -g --defined-only "$ro" | egrep -qs " $s\$"; then
	    if ! (echo "$tocheck $objs" | fgrep -qs "$o"); then
	      echo "$o needed for $s"
	      objs="$objs $o"
	    fi
	    break;
	  fi
	done
      done
    fi
  done
  tocheck="$tocheck$objs"
done

echo
echo
echo "object files needed: $tocheck"

cp /dev/null "$tmp"
for f in $tocheck; do
  objdump -d "$objpfx"../*/"$f" |
  awk 'BEGIN { last="" } /^[[:xdigit:]]* <[_[:alnum:]]*>:$/ { fct=substr($2, 2, length($2)-3) } /,%[xy]mm[[:digit:]]*$/ { if (last != fct) { print fct; last=fct} }' |
  while read fct; do
    if test "$fct" = "_dl_runtime_profile" -o "$fct" = "_dl_x86_64_restore_sse"; then
      continue;
    fi
    echo "function $fct in $f modifies xmm/ymm" >> "$tmp"
    result=1
  done
done

if test -s "$tmp"; then
  echo
  echo
  cat "$tmp"
  result=1
else
  result=0
fi

rm "$tmp"
exit $result
