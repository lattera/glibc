# awk script to merge a config-specific .symlist file with others.
# The input files should be an existing .abilist file, and a .symlist file.
# This must be passed run with awk -v config=REGEXP to specify a regexp
# matching configuration tuples for which the .symlist input defines an ABI.
# The result merges all duplicate occurrences of any symbol in a version set
# into a stanza listing the regexps matching configurations that contain it.

/^[^ ]/ {
  if (NF < 2 && config == "") {
    print "BAD LINE:", $0 > "/dev/stderr";
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

{
  if (current == "") next;

  if ($0 in seen) {
    seen[$0] = seen[$0] "\n" current;
  }
  else {
    seen[$0] = current;
  }

  next;
}

END {
  for (line in seen) {
    split(seen[line], setlist, "\n");
    for (i in setlist) {
      split(setlist[i], configs, ",");
      for (j in configs) {
	split(configs[j], temp, ":");
	version = temp[1];
	conf = temp[2];

	if ((version,conf) in have) continue;
	have[version,conf] = 1;

	if (version in confs) {
	  split(confs[version], c, " ");
	  if (conf < c[1]) {
	    confs[version] = conf;
	    for (k = 1; k <= nconfs[version]; ++k) {
	      confs[version] = confs[version] " " c[k];
	    }
	  }
	  else {
	    confs[version] = c[1];
	    for (k = 2; k <= nconfs[version]; ++k) {
	      if (conf < c[k]) break;
	      confs[version] = confs[version] " " c[k];
	    }
	    confs[version] = confs[version] " " conf;
	    for (; k <= nconfs[version]; ++k) {
	      confs[version] = confs[version] " " c[k];
	    }
	  }
	  ++nconfs[version];
	}
	else {
	  confs[version] = conf;
	  nconfs[version] = 1;
	}
      }
    }
    for (idx in have) delete have[idx];

    for (version in confs) {

      # Hack: if an element is foo.*/bar and there is also a foo.*,
      # then we can omit the foo.*/bar since foo.* matches already.
      nc = split(confs[version], c, " ");
      for (i = 1; i <= nc; ++i) {
	slash = index(c[i], ".*/");
	if (slash > 0) {
	  beforeslash = substr(c[i], 1, slash + 2 - 1);
	  for (j = 1; j <= nc; ++j)
	    if (j != i && c[j] == beforeslash) {
	      c[i] = c[nc--];
	      break;
	    }
	}
      }

      idx = version;
      for (i = 1; i <= nc; ++i)
	idx = idx " " c[i];

      if (idx in final) {
	final[idx] = final[idx] "\n" line;
      }
      else {
	final[idx] = line;
      }
      delete confs[version];
      delete nconfs[version];
    }
  }

  nstanzas = 0;
  for (stanza in final) {
    if (nstanzas == 0) {
      stanzas = stanza;
      nstanzas = 1;
      continue;
    }
    split(stanzas, s, "\n");
    if (stanza < s[1]) {
      stanzas = stanza;
      for (i = 1; i <= nstanzas; ++i) {
	stanzas = stanzas "\n" s[i];
      }
    }
    else {
      stanzas = s[1];
      for (i = 2; i <= nstanzas; ++i) {
	if (stanza < s[i]) break;
	stanzas = stanzas "\n" s[i];
      }
      stanzas = stanzas "\n" stanza;
      for (; i <= nstanzas; ++i) {
	stanzas = stanzas "\n" s[i];
      }
    }
    ++nstanzas;
  }

  split(stanzas, order, "\n");
  for (i = 1; i <= nstanzas; ++i) {
    stanza = order[i];
    print stanza;
    outpipe = "sort";
    print final[stanza] | outpipe;
    close(outpipe);
  }
}
