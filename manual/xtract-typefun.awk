#! /usr/local/bin/gawk -f
BEGIN {
  last_node="";
}

/^@node/ {
  last_node = gensub (/@node +([^@,]+).*/, "\\1", 1);
}

/^@deftypefun/ {
  printf ("* %s: (libc)%s.\n",
	  gensub (/@deftypefun +([^{ ]+|\{[^}]+\}) +([[:alpha:]_][[:alnum:]_]+).*/, "\\2", 1),
    last_node);
}

/^@deftypevr/ {
  printf ("* %s: (libc)%s.\n",
	  gensub (/@deftypevr +([^{ ]+|\{[^}]+\}) +([^{ ]+|\{[^}]+\}) +([[:alpha:]_][[:alnum:]_]+).*/, "\\3", 1),
    last_node);
}

/^@deftypefn/ {
  printf ("* %s: (libc)%s.\n",
	  gensub (/@deftypefn +([^{ ]+|\{[^}]+\}) +[^{ ]*(\{[^}]+\})? +([[:alpha:]_][[:alnum:]_]+).*/, "\\3", 1),
    last_node);
}
