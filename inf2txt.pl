#!/usr/bin/perl -w
#
#  Simple Texinfo -> plaintext converter. Texinfo (.inf) is almost
#  plaintext except for some control characters that separate sections.
#  Besides, menu lists usually hurt.
#
#  Usage: inf2txt file.inf file.inf-1 file.inf-2 >file.txt
#

use strict;

my $first = 1;
while (defined (my $header = <>)) {
    my @content;
    my $line;
    while (defined ($line = <>) && $line !~ /\x1F/) {
        push @content, $line;
    }
    if ($header =~ /^File:/) {
        # this is a texinfo node
        foreach (@content) {
            last if $_ eq "* Menu:\n" && $header !~ /\bIndex\b/;
            s/\*Note //g;
            print;
        }
        if ($first) {
            $first = 0;
            print <<EOF;
  ,-----------------------------------------------------------------.
  | This file was generated automatically from the Texinfo version, |
  | by stripping all the Texinfo magic. If you have a Texinfo       |
  | browser, you should probably use that instead.                  |
  \`-----------------------------------------------------------------\'
EOF
        }
        print "\n";
    } else {
        $header =~ /^Tag Table:/ or $header =~ /^Indirect:/ 
            or $header =~ /^This is Info file/ or $header =~ /^End Tag Table/
            or print STDERR "ignoring `$header'\n";
    }
    last if !defined $line;
}
