# Script to preprocess Versions.all lists based on "earliest version"
# specifications in the shlib-versions file.

NF == 3 && $2 == ":" { firstversion[$1] = $3; next }

NF == 2 && $2 == "{" { thislib = $1; print; next }

$1 == "}" {
  if (firstversion[thislib]) {
    # We haven't seen the stated version, but have produced
    # others pointing to it, so we synthesize it now.
    printf "  %s\n", firstversion[thislib];
  }
  print;
  next;
}

{
  if (! firstversion[thislib])
    print;
  else if ($1 == firstversion[thislib]) {
    print;
    firstversion[thislib] = 0;
  }
  else
    print $1, "=", firstversion[thislib];
}
