#!/bin/sh

common_objpfx=$1; shift
elf_objpfx=$1; shift
rtld_installed_name=$1; shift
logfile=$common_objpfx/linuxthreads/tst-tls2.out

# We have to find libc and linuxthreads
library_path=${common_objpfx}:${common_objpfx}linuxthreads
tst_tls1="${elf_objpfx}${rtld_installed_name} --library-path ${library_path} \
	  ${common_objpfx}/linuxthreads/tst-tls1"

LC_ALL=C
export LC_ALL
LANG=C
export LANG

> $logfile
fail=0

for aligned in a e f; do
  echo "preload tst-tls1mod{$aligned,b,c,d}.so" >> $logfile
  echo "===============" >> $logfile
  LD_PRELOAD=`echo ${common_objpfx}linuxthreads/tst-tls1mod{$aligned,b,c,d}.so \
	      | sed 's/:$//;s/: /:/g'` ${tst_tls1} >> $logfile || fail=1
  echo >> $logfile

  echo "preload tst-tls1mod{b,$aligned,c,d}.so" >> $logfile
  echo "===============" >> $logfile
  LD_PRELOAD=`echo ${common_objpfx}linuxthreads/tst-tls1mod{b,$aligned,c,d}.so \
	      | sed 's/:$//;s/: /:/g'` ${tst_tls1} >> $logfile || fail=1
  echo >> $logfile

  echo "preload tst-tls1mod{b,c,d,$aligned}.so" >> $logfile
  echo "===============" >> $logfile
  LD_PRELOAD=`echo ${common_objpfx}linuxthreads/tst-tls1mod{b,c,d,$aligned}.so \
	      | sed 's/:$//;s/: /:/g'` ${tst_tls1} >> $logfile || fail=1
  echo >> $logfile
done

echo "preload tst-tls1mod{d,a,b,c,e}" >> $logfile
echo "===============" >> $logfile
LD_PRELOAD=`echo ${common_objpfx}linuxthreads/tst-tls1mod{d,a,b,c,e}.so \
	    | sed 's/:$//;s/: /:/g'` ${tst_tls1} >> $logfile || fail=1
echo >> $logfile

echo "preload tst-tls1mod{d,a,b,e,f}" >> $logfile
echo "===============" >> $logfile
LD_PRELOAD=`echo ${common_objpfx}linuxthreads/tst-tls1mod{d,a,b,e,f}.so \
	    | sed 's/:$//;s/: /:/g'` ${tst_tls1} >> $logfile || fail=1
echo >> $logfile

exit $fail
