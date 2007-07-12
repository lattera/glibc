#!/bin/sh
#
# List all the files under version control in the source tree.
#

case $# in
0) ;;
1) cd "$1" ;;
*) echo >&2 "Usage: $0 [top_srcdir]"; exit 2 ;;
esac

if [ -r CVS/Entries ]; then

  ${CVS:-cvs} status 2>&1 | ${AWK:-awk} '
NF >= 2 && $(NF - 1) == "Examining" { dir = $NF }
$1 == "File:" { print (dir == ".") ? $2 : (dir "/" $2) }'
  exit $?

elif [ -r .svn/entries ]; then

  ${SVN:-svn} ls -R | sed '/\/$/d'
  exit $?

elif [ -r MT/options ]; then

  exec ${MONOTONE:-monotone} list known

elif [ -r .git/HEAD ]; then

  exec ${GIT:-git} ls-files

fi

echo >&2 'Cannot list sources without some version control system in use.'
exit 1
