# Combine version map fragments into version files for the generated
# shared object.
# (C) Copyright 1998 Free Software Foundation, Inc.
# Written by Ulrich Drepper <drepper@cygnus.com>, 1998.

# Read definitions for the versions.
BEGIN {
  nlibs=0;
  while (getline < "Versions.def") {
    if (/^[a-zA-Z_]+ {/) {
      libs[$1] = 1;
      curlib = $1;
      while (getline < "Versions.def" && ! /^}/) {
	if (NF > 1) {
	  versions[$1] = 1;
	  derived[curlib, $1] = (" " $2);
	  for (n = 3; n <= NF; ++n) {
	    derived[curlib, $1] = sprintf("%s, %s", derived[curlib, $1], $n);
	  }
	} else {
	  versions[$1] = 1;
	}
      }
    }
  }
  close("Versions.def");

  tmpfile = (buildroot "/Versions.tmp");
  sort = ("sort -n >" tmpfile);
}

# Remove comment lines.
/^ *#/ {
  next;
}

# This matches the beginning of the version information for a new library.
/^[a-zA-Z_]+/ {
  actlib = $1;
  if (libs[$1] != 1) {
    printf("no versions defined for %s\n", $1);
    exit 1;
  }
  next;
}

# This matches the beginning of a new version for the current library.
/^  [A-Za-z_]/ {
  actver = $1;
  if (versions[$1] != 1) {
    printf("version %s not defined\n", $1);
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

# Now print the accumulated information.
END {
  close(sort);
  oldlib="";
  oldver="";
  while(getline < tmpfile) {
    if ($1 != oldlib) {
      if (oldlib != "") {
	closeversion(oldver);
	oldver = "";
	close(outfile);
      }
      oldlib = $1;
      outfile = (buildroot oldlib ".map");
      firstinfile = 1;
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
  closeversion(oldver);
  close(outfile);
  rm tmpfile;
}
