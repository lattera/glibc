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
  if ((thislib, idx[thislib]) in firstversion) {
    # We haven't seen the stated version, but have produced
    # others pointing to it, so we synthesize it now.
    printf "  %s\n", firstversion[thislib, idx[thislib]];
    idx[thislib]++;
  }
  print;
  next;
}

/GLIBC_PRIVATE/ { print; next }

{
  if ((thislib, idx[thislib]) in firstversion) {
    # XXX relative string comparison loses if we ever have multiple digits
    # between dots in GLIBC_x.y[.z] names.
    v = firstversion[thislib, idx[thislib]];
    while ($1 >= v) {
      firstversion[thislib, idx[thislib]] = 0;
      idx[thislib]++;
      if ((thislib, idx[thislib]) in firstversion)
        v = firstversion[thislib, idx[thislib]];
      else
        break;
    }
    if ($1 >= v)
      print;
    else
      print $1, "=", v;
  }
  else
    print;
}
