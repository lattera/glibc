BEGIN { special = 0 }

/EXTERNS/ { ndirs = split(subdirs, dirs)
	    for (i = 1; i <= ndirs; ++i)
	    	printf "extern void __init_%s (int argc, char **argv, char **envp);\n", dirs[i]
	    special = 1 }
/CALLS/ { ndirs = split(subdirs, dirs)
	  for (i = 1; i <= ndirs; ++i) printf "  __init_%s (argc, argv, envp);\n", dirs[i]
	  special = 1 }

{ if (special == 0) print $0; special = 0 }
