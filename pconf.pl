#!/usr/bin/perl -wT

# pconf.pl -- PHOST configuration utility
#
# Copyright (C) 2002 PHOST (Contrib) Team
#
# $Id$
# Author: PHOST (Contrib) Team
# Status: quick & dirty
#
#     This program is free software; you can redistribute it and/or modify
#     it under the terms of the GNU General Public License as published by
#     the Free Software Foundation; either version 2 of the License, or
#     (at your option) any later version.
#
#     This program is distributed in the hope that it will be useful,
#     but WITHOUT ANY WARRANTY; without even the implied warranty of
#     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#     GNU General Public License for more details.
#
#     You should have received a copy of the GNU General Public License
#     along with this program; if not, write to the Free Software
#     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


# Command line options:
#  -d    Print default parameter settings ("% PHOST" section).
#  -n    Print non-default parameter settings only ("% PHOST" section).
#  -h    Show help.
#  -o    Force to use "old" (pre-3.4) config definitions


use strict;
use vars qw($opt_h $opt_d $opt_n $opt_o);
use Getopt::Std;

my $configfilename     = "pconfig.src";
my %defconfigdata      = ();
my %configdata         = ();
my %configparamnames   = ();
my @defconfigfilenames = ();
my @defconfiglist      = ();

$| = 1;  # activate autoflush


if (@ARGV < 1) {
    usage( $0 );
}

getopts( 'hdno' ) || usage( $0 );
if (defined $opt_h) {
    usage ( $0 );
}
if (defined $opt_d && defined $opt_n) {
    usage( $0 );
}
push @defconfigfilenames, "config.hi4" unless defined $opt_o;
push @defconfigfilenames, "config.hi";
if (@ARGV) {
    $configfilename = shift @ARGV;
}
if (@ARGV) {
    usage ($0);
}
if (defined $opt_d) {
    printdefaults( @defconfigfilenames );
    exit 0;
}
if (defined $opt_n) {
    getdefaults( @defconfigfilenames );
    getconfig( $configfilename );
    &printnondefaults;
    exit 0;
}

usage( $0 );



sub getdefaults {
    foreach (@_) {
        open( DEFCONFIG, "< $_" ) or next;
        while (<DEFCONFIG>) {
            next unless /^CFDefine/;
            chomp;
            s/\s//g;
            s/([A-Z])/\l$1/g;  # tolower
            s/\btrue\b/yes/g;
            s/\bfalse\b/no/g;
            if (my ($defconfigparam, $defconfigvalue) = /^cfdefine\(([^,]+)[^\"]+"(.+?)".+\)$/) {
                $defconfigdata{$defconfigparam} = $defconfigvalue;
                push @defconfiglist, $defconfigparam;
            } elsif (($defconfigparam, $defconfigvalue) = /^cfdefine4\((\w+),[^\"]+"(.+?)".*\)$/) {
                if ($defconfigvalue =~ /^\$(\w+)/) {
                    die "Error: backwards reference `$defconfigvalue' in config definition\n"
                        if not defined $defconfigdata{$1};
                    $defconfigvalue = $defconfigdata{$1};
                }
                $defconfigdata{$defconfigparam} = $defconfigvalue;
                push @defconfiglist, $defconfigparam;
            }
        }
        close( DEFCONFIG );
        return;
    }
    die "Cannot open any of " . join(", ", @_) . "\n";
}

sub getconfig {
    my $configfile = shift;

    open( CONFIG, $configfile ) or die "Cannot open \"$configfile\": $!";
    while (<CONFIG>) {
	last if /^%\s*PCONTROL/;
	next if /^%/;
	next if /^\s*#/;
	next if /^\s*$/;
	chomp;
	s/\s//g;
	my ($configparamname) = split( /=/, $_, 2 );  # save original parameter name
	s/([A-Z])/\l$1/g;  # tolower
        s/\btrue\b/yes/g;
        s/\bfalse\b/no/g;
	if (my ($configparam, $configvalue) = split( /=/, $_, 2 )) {
            if ($configvalue =~ /^\$(.*)/) {
                die "Configuration Error: backwards reference `$configvalue'\n"
                    if not defined $configdata{$1};
                $configvalue = $configdata{$1};
            }
	    $configdata{$configparam} = $configvalue;
	    $configparamnames{$configparam} = $configparamname;
	}
    }
    close( CONFIG );
}


sub printdefaults {
#    my $defconfigfile = shift;
#
#    open( DEFCONFIG, $defconfigfile ) or die "Cannot open \"$defconfigfile\": $!";
#    print "% PHOST\n";
#    while (<DEFCONFIG>) {
#	next unless /^CFDefine/;
#	chomp;
#	s/\s//g;
#	if (my ($defconfigparam, $defconfigvalue) = /^CFDefine\(([^,]+)[^"]+"(.+?)".+\)$/) {
#	    printf( "%-29s = %s\n", $defconfigparam, $defconfigvalue);
#	}
#    }
#    close( DEFCONFIG );
    getdefaults (@_);
    print "% PHOST\n";
    foreach (@defconfiglist) {
        printf( "%-29s = %s\n", $_, $defconfigdata{$_});
    }
}

sub printnondefaults {
    my $param = "";

    foreach $param (sort keys %configdata) {
	if (exists $defconfigdata{$param}) {  # Parameter known by default?
	    if ($configdata{$param} ne $defconfigdata{$param}) {
		printf( "%-29s = %s\n", $configparamnames{$param}, $configdata{$param} );
	    }
	} else {
	    printf( "%-29s = %s\n", $configparamnames{$param}, $configdata{$param} );
	}
    }
}


sub usage {
    my $commandname = shift;

    $commandname =~ s#.*/##g;
    die "usage: $commandname [-h (help)] [-d (defaults)] [-n (non-defaults)] [-o (old config.hi only)] [pconfig.src]\n";
}
