#! /bin/sh

common_objpfx=$1; shift
lang=$*

here=`pwd`

# Generate data files.
for l in $lang; do
  cns=`echo $l | sed 's/\(.*\)[.][^.]*/\1/'`
  cn=locales/$cns
  fn=charmaps/`echo $l | sed 's/.*[.]\([^.]*\)/\1/'`
  LD_LIBRARY_PATH=$common_objpfx I18NPATH=./locales ${common_objpfx}elf/ld.so \
   ${common_objpfx}locale/localedef --quiet -i $cn -f $fn \
   ${common_objpfx}localedata/$cns
done

# Run the tests.
for l in $lang; do
  cns=`echo $l | sed 's/\(.*\)[.][^.]*/\1/'`

  LOCPATH=$common_objpfx/localedata LC_ALL=$cns \
   LD_LIBRARY_PATH=$common_objpfx $common_objpfx/elf/ld.so \
   $common_objpfx/localedata/tst-fmon \
   > $common_objpfx/localedata/fmon-$cns.out || status=1
  cmp -s fmon-$cns.exp $common_objpfx/localedata/fmon-$cns.out || status=1
done

exit $status
# Local Variables:
#  mode:shell-script
# End:
