# awk script to extract a config-specific .symlist file from a merged file.
# This must be passed run with awk -v config=TUPLE to specify the configuration
# tuple we will match.  The merged file contains stanzas in the form:
#	GLIBC_x.y regexp...
#	 function F
#	 variable D 0x4
# Each regexp is matched against TUPLE, and only matching stanzas go
# into the output, with the regexp list removed.  The result matches the
# original .symlist file from abilist.awk that was fed into merge-abilist.awk.

BEGIN {
  outpipe = "";
}

/^ / { if (!ignore) print | outpipe; next; }

{
  for (i = 2; i <= NF; ++i) {
    regex = "^" $i "$";
    if (match(config, regex) != 0) {
      if ($1 != version) {
	if (outpipe != "") {
	  close(outpipe);
	}
	version = $1;
	print version;
	outpipe = "sort";
      }
      ignore = 0;
      next;
    }
  }
  ignore = 1;
  next;
}

END {
  if (outpipe == "") {
    print "No stanza matched", config > "/dev/stderr";
    exit 2;
  }
  else
    close(outpipe);
}
