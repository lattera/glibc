# Combine version map fragments into version scripts for our shared objects.
# Copyright (C) 1998,99,2000,2002,2005 Free Software Foundation, Inc.
# Written by Ulrich Drepper <drepper@cygnus.com>, 1998.

# This script expects the following variables to be defined:
# defsfile		name of Versions.def file
# buildroot		name of build directory with trailing slash
# move_if_change	move-if-change command

# Read definitions for the versions.
BEGIN {
  lossage = 0;

  nlibs=0;
  while (getline < defsfile) {
    if (/^[a-zA-Z0-9_.]+ \{/) {
      libs[$1] = 1;
      curlib = $1;
      while (getline < defsfile && ! /^}/) {
	if ($2 == "=") {
	  renamed[curlib "::" $1] = $3;
	}
	else
	  versions[curlib "::" $1] = 1;
      }
    }
  }
  close(defsfile);

  tmpfile = buildroot "Versions.tmp";
  # POSIX sort needed.
  sort = "sort -t. -k 1,1 -k 2n,2n -k 3 > " tmpfile;
}

# Remove comment lines.
/^ *#/ {
  next;
}

# This matches the beginning of the version information for a new library.
/^[a-zA-Z0-9_.]+/ {
  actlib = $1;
  if (!libs[$1]) {
    printf("no versions defined for %s\n", $1) > "/dev/stderr";
    ++lossage;
  }
  next;
}

# This matches the beginning of a new version for the current library.
/^  [A-Za-z_]/ {
  if (renamed[actlib "::" $1])
    actver = renamed[actlib "::" $1];
  else if (!versions[actlib "::" $1] && $1 != "GLIBC_PRIVATE") {
    printf("version %s not defined for %s\n", $1, actlib) > "/dev/stderr";
    ++lossage;
  }
  else
    actver = $1;
  next;
}

# This matches lines with names to be added to the current version in the
# current library.  This is the only place where we print something to
# the intermediate file.
/^   / {
  sortver=actver
  # Ensure GLIBC_ versions come always first
  sub(/^GLIBC_/," GLIBC_",sortver)
  printf("%s %s %s\n", actlib, sortver, $0) | sort;
}


function closeversion(name, oldname) {
  if (firstinfile) {
    printf("  local:\n    *;\n") > outfile;
    firstinfile = 0;
  }
  # This version inherits from the last one only if they
  # have the same nonnumeric prefix, i.e. GLIBC_x.y and GLIBC_x.z
  # or FOO_x and FOO_y but not GLIBC_x and FOO_y.
  pfx = oldname;
  sub(/[0-9.]+/,".+",pfx);
  if (oldname == "" || name !~ pfx) print "};" > outfile;
  else printf("} %s;\n", oldname) > outfile;
}

function close_and_move(name, real_name) {
  close(name);
  system(move_if_change " " name " " real_name " >&2");
}

# Now print the accumulated information.
END {
  close(sort);

  if (lossage) {
    system("rm -f " tmpfile);
    exit 1;
  }

  oldlib = "";
  oldver = "";
  printf("version-maps =");
  while (getline < tmpfile) {
    if ($1 != oldlib) {
      if (oldlib != "") {
	closeversion(oldver, veryoldver);
	oldver = "";
	close_and_move(outfile, real_outfile);
      }
      oldlib = $1;
      real_outfile = buildroot oldlib ".map";
      outfile = real_outfile "T";
      firstinfile = 1;
      veryoldver = "";
      printf(" %s.map", oldlib);
    }
    if ($2 != oldver) {
      if (oldver != "") {
	closeversion(oldver, veryoldver);
	veryoldver = oldver;
      }
      printf("%s {\n  global:\n", $2) > outfile;
      oldver = $2;
    }
    printf("   ") > outfile;
    for (n = 3; n <= NF; ++n) {
      printf(" %s", $n) > outfile;
    }
    printf("\n") > outfile;
  }
  printf("\n");
  closeversion(oldver, veryoldver);
  close_and_move(outfile, real_outfile);
  #system("rm -f " tmpfile);
}
