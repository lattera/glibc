# awk script for shlib-versions.v -> soversions.i; see Makeconfig.

BEGIN {
  config = cpu "-" vendor "-" os;
  configs[config] = "DEFAULT";
}

{ thiscf = $1 }

$2 == "ABI" {
  if ((config ~ thiscf) && !abiname) {
    abiname = $3;
    sub(/@CPU@/, cpu, abiname);
    sub(/@VENDOR@/, vendor, abiname);
    sub(/@OS@/, os, abiname);
  }
  next;
}

# Obey the first matching DEFAULT line.
$2 == "DEFAULT" {
  $1 = $2 = "";
  default_set[++ndefault_set] = thiscf "\n" $0;
  next
}

# Collect all lib lines before emitting anything, so DEFAULT
# can be interspersed.
{
  lib = number = $2;
  sub(/=.*$/, "", lib);
  sub(/^.*=/, "", number);
  if ((thiscf FS lib) in numbers) next;
  numbers[thiscf FS lib] = number;
  order[thiscf FS lib] = ++order_n;
  if (NF > 2) {
    $1 = $2 = "";
    versions[thiscf FS lib] = $0
  }
}

END {
  for (elt in numbers) {
    split(elt, x);
    cf = x[1];
    lib = x[2];
    for (c in configs)
      if (c ~ cf) {
	if (elt in versions)
	  set = versions[elt];
	else {
	  set = (c == config) ? default_setname : "";
	  for (i = 1; i <= ndefault_set; ++i) {
	    split(default_set[i], x, "\n");
	    if (c ~ x[1]) {
	      set = x[2];
	      break;
	    }
	  }
	}
	line = set ? (lib FS numbers[elt] FS set) : (lib FS numbers[elt]);
	if (!((c FS lib) in lineorder) || order[elt] < lineorder[c FS lib]) {
	  lineorder[c FS lib] = order[elt];
	  lines[c FS lib] = configs[c] FS line;
	}
      }
  }
  if (abiname) {
    print "ABI", abiname
  }
  for (c in lines) {
    print lines[c]
  }
}
