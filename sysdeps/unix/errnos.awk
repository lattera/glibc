BEGIN { special = 0 }

/ERRNOS/ { nerrnos = split(errnos, errs)
	     for (i = 1; i <= nerrnos; ++i)
	       # Some systems define errno codes inside undefined #ifdefs,
	       # and then never actually use them.
	       printf "#ifdef %s\n  DO(\"%s\", %s);\n#endif\n", \
		 errs[i], errs[i], errs[i]
	     special = 1 }


{ if (special == 0) print $0; special = 0 }
