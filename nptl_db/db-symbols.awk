# This script processes the output of 'readelf -W -s' on the libpthread.so
# we've just built.  It checks for all the symbols used in td_symbol_list.

BEGIN {
%define DB_LOOKUP_NAME(idx, name)		required[STRINGIFY (name)] = 1;
%define DB_LOOKUP_NAME_TH_UNIQUE(idx, name)	th_unique[STRINGIFY (name)] = 1;
%include "db-symbols.h"

   in_symtab = 0;
}

/Symbol table '.symtab'/ { in_symtab=1; next }
NF == 0 { in_symtab=0; next }

!in_symtab { next }

NF >= 8 && $7 != "UND" { seen[$NF] = 1 }

END {
  status = 0;

  for (s in required) {
    if (s in seen) print s, "ok";
    else {
      status = 1;
      print s, "***MISSING***";
    }
  }

  any = "";
  for (s in th_unique) {
    if (s in seen) {
      any = s;
      break;
    }
  }
  if (any)
    print "th_unique:", any;
  else {
    status = 1;
    print "th_unique:", "***MISSING***";
  }

  exit(status);
}
