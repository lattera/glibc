#!/bin/sh
#
# List all the files under version control in the source tree.
#

case $# in
0) ;;
1) cd "$1" ;;
*) echo >&2 "Usage: $0 [top_srcdir]"; exit 2 ;;
esac

if [ -r .git/HEAD ]; then

  # List files for glibc core.
  ${GIT:-git} ls-files
  # List files for glibc ports.
  ports="ports"
  if [ -d "$PWD/$ports" ]; then
    cd "$PWD/$ports"
    ${GIT:-git} ls-files | sed -e "s,^,$ports/,g"
  else
    # We expect the glibc-ports directory to be symlinked as PORTS.
    # The glibc release manager will run this script as part of libc.pot
    # regeneration and should ensure the symlink to PORTS is setup.
    echo >&2 "WARNING: No \"$ports\" directory found. Expected glibc-ports"\
	     "source directory to be symlinked as \"$ports\" directory."
  fi
  exit 0
fi

echo >&2 'Cannot list sources without some version control system in use.'
exit 1
