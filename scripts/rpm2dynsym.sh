#!/bin/sh

# This script takes rpm package files, finds *.so.N files in them,
# and runs objdump --dynamic-syms on them.  The arguments are rpm file
# names.  For each rpm, it creates an output file with the name
# "NAME-VERSION-RELEASE.ARCH.dynsym", the variable parts being extracted
# from the rpm's headers (not its file name).  Each file contains the
# collected objdump output for all the *.so.N files in the corresponding rpm.
# This can be processed with abilist.awk or sent to someone who will do that.
# This does not do a lot of error-checking, so you should always watch stderr
# and sanity-check the resulting output files.

RPM=${RPM:-rpm}
RPM2CPIO=${RPM2CPIO:-rpm2cpio}
CPIO=${CPIO:-cpio}
OBJDUMP=${OBJDUMP:-objdump}

unpackdir=/tmp/rpm2dynsym$$
trap 'rm -rf $unpackdir' 0 1 2 15

for rpm; do
  name=`$RPM -qp $rpm --queryformat '%{NAME}-%{VERSION}-%{RELEASE}.%{ARCH}\n'`
  mkdir $unpackdir || exit
  $RPM2CPIO "$rpm" | {
    cd $unpackdir
    $CPIO -i -d --no-absolute-filenames -uv '*.so.*' '*.so' 2>&1 |
    while read file b; do
      test x"$b" = x || break
      case "$file" in
      *.so.[0-9]*) $OBJDUMP --dynamic-syms $file ;;
      esac
    done
  } > $name.dynsym
  echo wrote $name.dynsym for $rpm
  rm -rf $unpackdir
done
