#! /bin/sh

common_objpfx=$1; shift
run_program_prefix=$1; shift
lang=$*

id=${PPID:-100}
here=`pwd`

# Run collation tests.
status=0
for l in $lang; do
  here=0
  cns=`echo $l | sed 's/\(.*\)[.][^.]*/\1/'`
  LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}/iconvdata \
   LC_ALL=$l ${run_program_prefix} \
   ${common_objpfx}localedata/collate-test $id < $cns.in \
   > ${common_objpfx}localedata/$cns.out || here=1
  cmp -s $cns.in ${common_objpfx}localedata/$cns.out || here=1

  LOCPATH=${common_objpfx}localedata GCONV_PATH=${common_objpfx}/iconvdata \
   LC_ALL=$l ${run_program_prefix} \
   ${common_objpfx}localedata/xfrm-test $id < $cns.in \
   > ${common_objpfx}localedata/$cns.xout || here=1
  cmp -s $cns.in ${common_objpfx}localedata/$cns.xout || here=1
  if test $here -eq 0; then
    echo "$l OK"
  else
    echo "$l FAIL"
    status=1
  fi
done

exit $status
# Local Variables:
#  mode:shell-script
# End:
