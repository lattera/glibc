# awk script for shlib-versions.v.i -> soversions.i; see Makeconfig.

# Only lines matching `config' (set with -v) are relevant to us.
config !~ $1 { next }

# Obey the first matching DEFAULT line.
$2 == "DEFAULT" {
  if (!matched_default) {
    matched_default = 1;
    $1 = $2 = "";
    default_setname = $0;
  }
  next
}

# Collect all lib lines before emitting anything, so DEFAULT
# can be interspersed.
{
  lib = number = $2;
  sub(/=.*$/, "", lib);
  sub(/^.*=/, "", number);
  if (lib in numbers) next;
  numbers[lib] = number;
  if (NF > 2) {
    $1 = $2 = "";
    versions[lib] = $0
  }
}

END {
  for (lib in numbers) {
    set = (lib in versions) ? versions[lib] : default_setname;
    if (set)
      print lib, numbers[lib], set;
    else
      print lib, numbers[lib];
  }
}
