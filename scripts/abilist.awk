# This awk script processes the output of objdump --dynamic-syms
# into a simple format that should not change when the ABI is not changing.

# Normalize columns.
/^[0-9a-fA-F]+      / { sub(/      /, "  -   ") }

# Skip undefineds.
$4 == "*UND*" { next }

# Skip locals.
$2 == "l" { next }

$2 == "g" || $2 == "w" && NF == 7 {
  weak = $2;
  type = $3;
  size = $5;
  sub(/^0*/, "", size);
  size = " 0x" size;
  version = $6;
  symbol = $7;
  gsub(/[()]/, "", version);

  if (version == "GLIBC_PRIVATE") next;

  if (type == "D" && $4 == ".tbss") {
    type = "T";
  }
  else if (type == "D" && $4 == ".opd") {
    type = "O";
    size = "";
  }
  else if (type == "DO" && $4 == "*ABS*") {
    type = "A";
    size = "";
  }
  else if (type == "DO") {
    type = "D";
  }
  else if (type == "DF") {
    type = "F";
    size = "";
  }
  else {
    print symbol, version, weak, "?", type, $4, $5;
    next;
  }

  desc = " " symbol " " (weak == "w" ? tolower(type) : type) size;

  if (version in versions) {
    versions[version] = versions[version] "\n" desc;
  }
  else {
    versions[version] = desc;
  }
  next;
}

# Header crapola.
NF == 0 || /DYNAMIC SYMBOL TABLE/ || /file format/ { next }

{
  print "Don't grok this line:", $0
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
  }
}
