#! /usr/bin/awk -f
# Generate sorted list of directories.  The sorting is stable but with
# dependencies between directories resolved by moving dependees in front.
# (C) Copyright 1998 Free Software Foundation, Inc.
# Written by Ulrich Drepper <drepper@cygnus.com>, 1998.

BEGIN {
  cnt = 0
  dnt = 0
}
{
  if ($1 ~ /depend/) {
    from[dnt] = $2
    to[dnt] = $3
    ++dnt
  } else {
    all[cnt++] = $1
  }
}
END {
  do {
    moved = 0
    for (i = 0; i < dnt; ++i) {
      for (j = 0; j < cnt; ++j) {
	if (all[j] == from[i]) {
	  for (k = j + 1; k < cnt; ++k) {
	    if (all[k] == to[i]) {
	      break;
	    }
	  }
	  if (k < cnt) {
	    for (l = k - 1; l >= j; --l) {
	      all[l + 1] = all[l]
	    }
	    all[j] = to[i]
	    break;
	  }
	}
      }
      if (j < cnt) {
	moved = 1
	break
      }
    }
  } while (moved)

  printf "sorted-subdirs = "
  for (i = 0; i < cnt; ++i) {
    printf "%s ", all[i];
  }
  printf "\n"
}
