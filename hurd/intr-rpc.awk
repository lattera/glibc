# Icky intimate knowledge of MiG output.

BEGIN \
  {
    nprotolines=0; proto=0;
    args=""; echo=1; isintr=0;
    intrcall = "__hurd_intr_rpc_" call;
    print "#include <hurd/signal.h>";
  }

$NF == intrcall { isintr=1; }

NF == 1 && $1 == ")" { proto=0; }
proto \
  {
    protolines[nprotolines++] = $0;
    arg = $NF;
    if (substr(arg, 1, 1) == "*")
      arg = substr(arg, 2, length(arg)-1);
    args = args arg;
  }
NF == 1 && $1 == "(" { proto=1; }

NF == 3 && $1 == "InP->Head.msgh_request_port" \
  { portarg = substr($3, 1, length($3)-1); }

{ print $0; }

END \
  {
    if (isintr)
      {
	print "\n\n/* User-callable interrupt-handling stub.  */";
	print "kern_return_t __" call;
	print "(";
	for (i = 0; i < nprotolines; ++i)
	  print protolines[i];
	print ")";
	print "{";
	print "  return HURD_EINTR_RPC (" portarg ", " \
	  intrcall "(" args "));";
	print "}";
      }
    print "weak_alias (__" call ", " call ")"
  }
