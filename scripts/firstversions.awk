# Script to preprocess Versions.all lists based on "earliest version"
# specifications in the shlib-versions file.

NF > 2 && $2 == ":" {
  for (i = 0; i <= NF - 3; ++i)
    firstversion[$1, i] = $(3 + i);
  idx[$1] = 0;
  next;
}

NF == 2 && $2 == "{" { thislib = $1; print; next }

$1 == "}" {
  if (firstversion[thislib, idx[thislib]]) {
    # We haven't seen the stated version, but have produced
    # others pointing to it, so we synthesize it now.
    printf "  %s\n", firstversion[thislib, idx[thislib]];
    idx[thislib]++;
  }
  print;
  next;
}

{
  v = firstversion[thislib, idx[thislib]];

  if (! v)
    print;
  else if ($1 == v) {
    print;
    firstversion[thislib, idx[thislib]] = 0;
    idx[thislib]++;
  }
  else
    print $1, "=", v;
}
