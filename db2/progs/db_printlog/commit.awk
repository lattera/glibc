# @(#)commit.awk	10.1 (Sleepycat) 11/1/98
#
# Output tid of committed transactions.

/txn_regop/ {
	print $5
}
