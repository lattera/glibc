#
#  Awk program to analyze mtrace.c output.
#
{
  if ($1 == "@") {
    where = " (" $2 ")"
    n = 3
  } else {
    where = ""
    n = 1
  }
  if ($n == "+") {
    if (allocated[$(n+1)] != "")
      print "+", $(n+1), "Alloc", NR, "duplicate:", allocated[$(n+1)], wherewas[$(n+1)], where;
    else {
      wherewas[$(n+1)] = where;
      allocated[$(n+1)] = $(n+2);
    }
  } else if ($n == "-") {
    if (allocated[$(n+1)] != "") {
      wherewas[$(n+1)] = "";
      allocated[$(n+1)] = "";
      if (allocated[$(n+1)] != "")
	print "DELETE FAILED", $(n+1), allocated[$(n+1)];
    } else
      print "-", $(n+1), "Free", NR, "was never alloc'd", where;
  } else if ($n == "<")	{
    if (allocated[$(n+1)] != "") {
      wherewas[$(n+1)] = "";
      allocated[$(n+1)] = "";
    } else
      print "-", $(n+1), "Realloc", NR, "was never alloc'd", where;
  } else if ($n == ">") {
    if (allocated[$(n+1)] != "")
      print "+", $(n+1), "Realloc", NR, "duplicate:", allocated[$(n+1)], where;
    else {
      wherewas[$(n+1)] = $(n+2);
      allocated[$(n+1)] = $(n+2);
    }
  } else if ($n == "=") {
    # Ignore "= Start"
  } else if ($n == "!") {
    # Ignore failed realloc attempts for now
  }
}
END {
  for (x in allocated) 
    if (allocated[x] != "")
      print "+", x, allocated[x], wherewas[x];
}
