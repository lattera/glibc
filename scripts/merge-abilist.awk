# awk script to merge a config-specific .symlist file with others.
# The input files should be existing .abilist files, and a .symlist
# file.  This must be run with awk -v config=REGEXP to specify a
# regexp matching configuration tuples for which the .symlist input
# defines an ABI.  The result merges all duplicate occurrences of any
# symbol into a stanza listing the regexps matching configurations
# that contain it and giving associated versions.
# The merged file contains stanzas in the form:
#	GLIBC_x.y regexp...
#	| GLIBC_x.y.z regexp...
#	| GLIBC_m.n regexp...
#	 function F
#	 variable D 0x4

BEGIN { current = "UNSET" }

/^[^| ]/ {
  if (NF < 2 && config == "") {
    print FILENAME ":" FNR ": BAD SET LINE:", $0 > "/dev/stderr";
    exit 2;
  }

  if (NF < 2) {
    current = $1 ":" config;
  }
  else {
    # Filter out the old stanzas from the config we are merging in.
    # That way, if a set disappears from the .symlist file for this
    # config, the old stanza doesn't stay in the merged output tagged
    # for this config.  (Disappearing sets might happen during development,
    # and between releases could happen on a soname change).
    nc = 0;
    for (i = 2; i <= NF; ++i)
      if ($i != config)
        c[nc++] = $i;
    if (nc == 0)
      current = "";
    else {
      current = $1 ":" c[0];
      for (i = 1; i < nc; ++i)
        current = current "," $1 ":" c[i];
    }
  }

  next;
}

/^\| / {
  if (NF < 3 || current == "UNSET") {
    print FILENAME ":" FNR ": BAD | LINE:", $0 > "/dev/stderr";
    exit 2;
  }

  nc = 0;
  for (i = 3; i <= NF; ++i)
    if ($i != config)
      c[nc++] = $i;
  for (i = 0; i < nc; ++i)
    current = current "," $2 ":" c[i];

  next;
}

{
  if (current == "") next;
  if (current == "UNSET") {
    print FILENAME ":" FNR ": IGNORED LINE:", $0 > "/dev/stderr";
    next;
  }

  ns = split(seen[$0], s, ",");
  nc = split(current, c, ",");
  for (i = 1; i <= nc; ++i) {
    if (c[i] == "")
      continue;
    # Sorted insert.
    for (j = 1; j <= ns; ++j) {
      if (c[i] == s[j])
        break;
      if (c[i] < s[j]) {
	for (k = ns; k >= j; --k)
	  s[k + 1] = s[k];
	s[j] = c[i];
	++ns;
	break;
      }
    }
    if (j > ns)
      s[++ns] = c[i];
  }

  seen[$0] = s[1];
  for (i = 2; i <= ns; ++i)
    seen[$0] = seen[$0] "," s[i];

  next;
}

END {
  for (line in seen) {
    if (seen[line] in stanzas)
      stanzas[seen[line]] = stanzas[seen[line]] "\n" line;
    else
      stanzas[seen[line]] = line;
  }

  ns = split("", s);
  for (configs in stanzas) {
    # Sorted insert.
    for (j = 1; j <= ns; ++j) {
      if (configs == s[j])
        break;
      if (configs < s[j]) {
	for (k = ns; k >= j; --k)
	  s[k + 1] = s[k];
	s[j] = configs;
	++ns;
	break;
      }
    }
    if (j > ns)
      s[++ns] = configs;
  }

  # S[1..NS] is now a sorted list of stanza identifiers.
  # STANZAS[ID] contains the lines for that stanza.
  # All we have to do is pretty-print the stanza ID,
  # and then print the sorted list.

  for (i = 1; i <= ns; ++i) {
    # S[I] is a sorted, comma-separated list of SET:CONFIG pairs.
    # All we have to do is pretty-print them.
    nc = split(s[i], c, ",");
    lastvers = lastconf = "";
    for (j = 1; j <= nc; ++j) {
      split(c[j], temp, ":");
      version = temp[1];
      conf = temp[2];
      if (version != lastvers)
	printf "%s%s", (lastvers != "" ? "\n| " : ""), version;
      # Hack: if CONF is foo.*/bar and LASTCONF was foo.*,
      # then we can omit the foo.*/bar since foo.* matches already.
      # Note we don't update LASTCONF, so foo.*/baz next time will match too.
      else if ((slash = index(conf, ".*/")) > 0 && \
	       substr(conf, 1, slash + 2 - 1) == lastconf)
        continue;
      printf " %s", conf;
      lastvers = version;
      lastconf = conf;
    }
    print "";
    outpipe = "sort";
    print stanzas[s[i]] | outpipe;
    close(outpipe);
  }
}
