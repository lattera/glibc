BEGIN { intr=0; wantcall=0; calls=""; }
 
$1 == "/*" && $2 == "INTR" && $3 == "*/" { intr=1; }

NF == 1 && $1 == "routine"	{ wantcall=1; next; }

intr != 0 && wantcall == 0 && NF >= 2 && $1 == "routine" \
  {
    if (substr($2, length($2)-2, 1) == "(")
      calls = calls " " substr($2, 0, length($2)-1);
    else calls = calls " " $2;
    intr=0;
  }

wantcall != 0 && NF >= 1 \
  {
    if (substr($1, length($1)-2, 1) == "(")
      calls = calls " " substr($1, 0, length($1)-1);
    else calls = calls " " $1;
    intr=0;
  }

{ wantcall=0; }

END { print varname " :=" calls; }
