BEGIN { special = 0 }

/REQUESTS/ { nreqs = split(requests, reqs)
	     for (i = 1; i <= nreqs; ++i)
	       printf "#ifdef\t%s\n  DEFINE(\"%s\", %s);\n#endif\n", \
		      reqs[i], reqs[i], reqs[i]
	     special = 1 }


{ if (special == 0) print $0; special = 0 }
