# awk script to extract a config-specific .symlist file from a merged file.
# This must be passed run with awk -v config=TUPLE to specify the configuration
# tuple we will match.  The merged file contains stanzas in the form:
#	GLIBC_x.y regexp...
#	| GLIBC_x.y.z regexp...
#	| GLIBC_m.n regexp...
#	 function F
#	 variable D 0x4
# Each regexp is matched against TUPLE, and only matching stanzas go
# into the output, with the regexp list removed.  Multiple version lines
# can match with the same regexp, meaning the stanza is duplicated in
# multiple version sets.  The result matches the original .symlist file
# from abilist.awk that was fed into merge-abilist.awk.

BEGIN {
  inside = 0;
}

/^ / {
  inside = 1;
  if (!ignore) {
    for (version in current) {
      if (version in versions)
	versions[version] = versions[version] "\n" $0;
      else
	versions[version] = $0;
    }
  }
  next;
}

{
  second = ($1 == "|");
  if (second && inside) {
    printf "%s:%d: bad input line inside stanza: %s\n", FILENAME, FNR, $0;
    exit 1;
  }
  inside = 0;

  for (i = second ? 3 : 2; i <= NF; ++i) {
    regex = "^" $i "$";
    if (match(config, regex) != 0) {
      if (!second || ignore)
        # Clear old array.
        split("", current);
      current[second ? $2 : $1] = 1;
      ignore = 0;
      next;
    }
  }

  if (!second)
    ignore = 1;
  next;
}

END {
  nverlist = 0;
  for (version in versions) {
    if (nverslist == 0) {
      verslist = version;
      nverslist = 1;
      continue;
    }
    split(verslist, s, "\n");
    if (version < s[1]) {
      verslist = version;
      for (i = 1; i <= nverslist; ++i) {
	verslist = verslist "\n" s[i];
      }
    }
    else {
      verslist = s[1];
      for (i = 2; i <= nverslist; ++i) {
	if (version < s[i]) break;
	verslist = verslist "\n" s[i];
      }
      verslist = verslist "\n" version;
      for (; i <= nverslist; ++i) {
	verslist = verslist "\n" s[i];
      }
    }
    ++nverslist;
  }

  split(verslist, order, "\n");
  for (i = 1; i <= nverslist; ++i) {
    version = order[i];

    print version;
    outpipe = "sort";
    print versions[version] | outpipe;
    close(outpipe);

    if (version == lastversion)
      break;
  }
}
