# @(#)count.awk	10.1 (Sleepycat) 11/1/98
#
# Print out the number of log records for transactions that we
# encountered.

/^\[/{
	if ($5 != 0)
		print $5
}
