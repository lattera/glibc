# This awk script processes the output of objdump --dynamic-syms
# into a simple format that should not change when the ABI is not changing.

BEGIN { outpipe = "sort" }

# Normalize columns.
/^[0-9a-fA-F]+      / { sub(/      /, "  -   ") }

# Skip undefineds.
$4 == "*UND*" { next }

# Skip locals.
$2 == "l" { next }

$2 == "g" || $2 == "w" && NF == 7 {
  weak = ($2 == "w") ? "weak" : "strong";
  type = $3;
  size = strtonum("0x" $5);
  version = $6;
  symbol = $7;
  gsub(/[()]/, "", version);

  if (version == "GLIBC_PRIVATE") next;

  if (type == "D" && $4 == ".tbss") {
    print symbol, version, weak, "TLS", size | outpipe;
  }
  else if (type == "D" && $4 == ".opd") {
    print symbol, version, weak, "FDESC" | outpipe;
  }
  else if (type == "DO" && $4 == "*ABS*") {
    print symbol, version, weak, "ABS" | outpipe;
  }
  else if (type == "DO") {
    print symbol, version, weak, "DATA", size | outpipe;
  }
  else if (type == "DF") {
    print symbol, version, weak, "FUNC" | outpipe;
  }
  else {
    print symbol, version, weak, "UNEXPECTED", type, $4, $5;
  }

  next;
}

# Header crapola.
NF == 0 || /DYNAMIC SYMBOL TABLE/ || /file format/ { next }

{
  print "Don't grok this line:", $0
}
