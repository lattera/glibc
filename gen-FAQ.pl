#! /usr/bin/perl

=pod
This is a silly little program for generating the libc FAQ.

The input format is:
top boilerplate
^L
? section name (one line)
?? question...
...
{ID} answer...
...
^L
{ID} name <email@address>
...

which gets mapped to:

top boilerplate
^L
1. section 1...
1.1. q1.1
1.2. q1.2
...
^L
1. section 1...

1.1. q1.1

answer 1.1....


^L
Answers were provided by:
...

=cut

# We slurp the whole file into a pair of assoc arrays indexed by
# the 'section.question' number.
%questions = ();
%answers = ();
$question = 0;

# These arrays and counter keep track of the sections.
@sectcount = ();
@sections = ();
$section = 0;

# Cross reference list.
%refs = ();

# Separators.
$sepmaj = "\f\n" . ('~ ' x 36) . "\n\n";
$sepmin = "\f\n" . ('. ' x 36) . "\n\n";

# Pass through the top boilerplate.
while(<>)
{
    last if $_ eq "\f\n";
    print;
}

# Now the body.
while(<>)
{
    /\f/ && do
    {
	$sectcount[$section] = $question;
	last;
    };

    s/^\?\s+// && do
    {
	chomp;
	$sectcount[$section] = $question if $section > 0;
	$section++;
	$sections[$section] = $_;
	$question = 0;
	next;
    };
    s/^\?\?(\w*?)\s+// && do
    {
	$cur = \%questions;
	$question++;
	$questions{$section,$question} = $_;
	$refs{$1} = "$section.$question" if $1 ne "";
	next;
    };
    /^\{/ && do
    {
	$cur = \%answers;
	$answers{$section,$question} .= $_;
	next;
    };

    ${$cur}{$section,$question} .= $_;
}

# Now we have to clean up the newlines and deal with cross references.
foreach(keys %questions) { $questions{$_} =~ s/\n+$//; }
foreach(keys %answers)
{
    $answers{$_} =~ s/\n+$//;
    $answers{$_} =~ s/(\s)\?(\w+)\b/$1 . "question " . ($refs{$2} or badref($2,$_), "!!$2")/eg;
}

# Now output the formatted FAQ.
print $sepmaj;
for($i = 1; $i <= $section; $i++)
{
    print "$i. $sections[$i]\n\n";
    for($j = 1; $j <= $sectcount[$i]; $j++)
    {
	print "$i.$j.\t$questions{$i,$j}\n";
    }
    print "\n";
}

print $sepmaj;
for($i = 1; $i <= $section; $i++)
{
    print "$i. $sections[$i]\n\n";
    for($j = 1; $j <= $sectcount[$i]; $j++)
    {
	print "$i.$j.\t$questions{$i,$j}\n\n";
	print $answers{$i,$j}, "\n\n";
	print "\n" if $j < $sectcount[$i];
    }
    print $sepmin if $i < $section;
}

print $sepmaj;

# Pass through the trailer.
while(<>) { print; }

sub badref
{
    my($ref,$quest) = @_;
    $quest =~ s/$;/./;
    print STDERR "Undefined reference to $ref in answer to Q$quest\n";
}
