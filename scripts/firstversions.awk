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
    f = v = firstversion[thislib, idx[thislib]];
    while ($1 >= v) {
      delete firstversion[thislib, idx[thislib]];
      idx[thislib]++;
      if ((thislib, idx[thislib]) in firstversion)
        v = firstversion[thislib, idx[thislib]];
      else
        break;
    }
    if ($1 == v || $1 == f)
      # This version was the specified earliest version itself.
      print;
    else if ($1 < v) {
      # This version is older than the specified earliest version.
      print "  " $1, "=", v;
      # Record that V has been referred to, so we will be sure to emit it
      # if we hit a later one without hitting V itself.
      usedversion[thislib, v] = 1;
    }
    else {
      # This version is newer than the specified earliest version.
      # We haven't seen that version itself or else we wouldn't be here
      # because we would have removed it from the firstversion array.
      # If there were any earlier versions that used that one, emit it now.
      if ((thislib, v) in usedversion) {
        print "  " v;
      }
      print "  " $1;
    }
  }
  else
    print;
}
