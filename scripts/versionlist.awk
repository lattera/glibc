# Extract ordered list of version sets from Versions files.
# Copyright (C) 2014 Free Software Foundation, Inc.

BEGIN { in_lib = ""; in_version = 0 }

!in_lib && NF == 2 && $2 == "{" { in_lib = $1; next }
!in_lib { next }

NF == 2 && $2 == "{" {
  in_version = 1;
  libs[in_lib] = libs[in_lib] "  " $1 "\n";
  lib_versions[in_lib, $1] = 1;
  all_versions[$1] = 1;
  next
}

in_version && $1 == "}" { in_version = 0; next }
in_version { next }

$1 == "}" { in_lib = ""; next }

END {
  nlibs = asorti(libs, libs_order);
  for (i = 1; i <= nlibs; ++i) {
    lib = libs_order[i];

    for (v in all_versions) {
      if (!((in_lib, v) in lib_versions)) {
        libs[lib] = libs[lib] "  " v "\n";
      }
    }

    print lib, "{";
    sort = "sort -u -t. -k 1,1 -k 2n,2n -k 3";
    printf "%s", libs[lib] | sort;
    close(sort);
    print "}";
  }
}
