#!/usr/bin/perl

use strict;
use warnings;

# Check __BEGIN_NAMESPACE ... __END_NAMESPACE pairing in an include file.

my $code = 0;
for my $path (@ARGV) {
    my $localcode = 0;
    my @stack;

    open my $in, '<', $path
        or die "open $path failed: $!";

    while (<$in>) {
        if ( /^\s*__BEGIN_(.*)\b/ ) {
            push @stack, $1;
        }
        elsif ( /^\s*__END_(.*)\b/ ) {
            if (@stack) {
                my $tag = pop @stack;
		if ($1 ne $tag) {
                    print "$path:$.: BEGIN $tag paired with END $1\n";
		    $localcode = 1;
		}
            }
            else {
                print "$path:$.: END $1 does not match a begin\n";
		$localcode = 1;
            }
        }
    }

    if (@stack) {
	print "$path: Unmatched begin tags " . join (' ', @stack) ."\n";
	$localcode = 1;
    }

    if ($localcode == 0) {
	print "$path: OK\n";
    } else {
	$code = $localcode;
    }
}

exit $code;
