# Combine version map fragments into version files for the generated
# shared object.
# (C) Copyright 1998 Free Software Foundation, Inc.
# Written by Ulrich Drepper <drepper@cygnus.com>, 1998.

# This script expects the following variables to be defined:
# defsfile		name of Versions.def file
# buildroot		name of build directory with trailing slash
# move_if_change	move-if-change command

# Read definitions for the versions.
BEGIN {
  nlibs=0;
  while (getline < defsfile) {
    if (/^[a-zA-Z_]+ {/) {
      libs[$1] = 1;
      curlib = $1;
      while (getline < defsfile && ! /^}/) {
	versions[$1] = 1;
	if (NF > 1) {
	  derived[curlib, $1] = " " $2;
	  for (n = 3; n <= NF; ++n) {
	    derived[curlib, $1] = derived[curlib, $1] ", " $n;
	  }
	}
      }
    }
  }
  close(defsfile);

  tmpfile = buildroot "Versions.tmp";
  sort = "sort -n > " tmpfile;
}

# Remove comment lines.
/^ *#/ {
  next;
}

# This matches the beginning of the version information for a new library.
/^[a-zA-Z_]+/ {
  actlib = $1;
  if (!libs[$1]) {
    printf("no versions defined for %s\n", $1) > "/dev/stderr";
    exit 1;
  }
  next;
}

# This matches the beginning of a new version for the current library.
/^  [A-Za-z_]/ {
  actver = $1;
  if (!versions[$1]) {
    printf("version %s not defined\n", $1) > "/dev/stderr";
    exit 1;
  }
  next;
}

# This matches lines with names to be added to the current version in the
# current library.  This is the only place where we print something to
# the intermediate file.
/^   / {
  printf("%s %s %s\n", actlib, actver, $0) | sort;
}


function closeversion(name) {
  if (firstinfile) {
    printf("  local:\n    *;\n") > outfile;
    firstinfile = 0;
  }
  printf("}%s;\n", derived[oldlib, name]) > outfile;
}

function close_and_move(name, real_name) {
  close(name);
  system(move_if_change " " name " " real_name " >&2");
}

# Now print the accumulated information.
END {
  close(sort);
  oldlib = "";
  oldver = "";
  printf("version-maps =");
  while(getline < tmpfile) {
    if ($1 != oldlib) {
      if (oldlib != "") {
	closeversion(oldver);
	oldver = "";
	close_and_move(outfile, real_outfile);
      }
      oldlib = $1;
      real_outfile = buildroot oldlib ".map";
      outfile = real_outfile "T";
      firstinfile = 1;
      printf(" %s.map", oldlib);
    }
    if ($2 != oldver) {
      if (oldver != "") {
	closeversion(oldver);
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
  closeversion(oldver);
  close_and_move(outfile, real_outfile);
  system("rm -f " tmpfile);
}
