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


use strict;
use vars qw($opt_h $opt_d $opt_n);
use Getopt::Std;

my $defconfigfilename  = "config.hi";
my $configfilename     = "pconfig.src";
my %defconfigdata      = ();
my %configdata         = ();
my %configparamnames   = ();

$| = 1;  # activate autoflush


if (@ARGV < 1) {
    usage( $0 );
}

getopts( 'hdn' ) || usage( $0 );
if (defined $opt_h) {
    usage ( $0 );
}
if (defined $opt_d && defined $opt_n) {
    usage( $0 );
}
if (defined $opt_d) {
    printdefaults( $defconfigfilename );
    exit 0;
}
if (defined $opt_n) {
    getdefaults( $defconfigfilename );
    getconfig( $configfilename );
    &printnondefaults;
    exit 0;
}

usage( $0 );



sub getdefaults {
    my $defconfigfile = shift;

    open( DEFCONFIG, $defconfigfile ) or die "Cannot open \"$defconfigfile\": $!";
    while (<DEFCONFIG>) {
	next unless /^CFDefine/;
	chomp;
	s/\s//g;
	s/([A-Z])/\l$1/g;  # tolower
	if (my ($defconfigparam, $defconfigvalue) = /^cfdefine\(([^,]+)[^"]+"(.+?)".+\)$/) {
	    $defconfigdata{$defconfigparam} = $defconfigvalue;
	}
    }
    close( DEFCONFIG );
}


sub getconfig {
    my $configfile = shift;

    open( CONFIG, $configfile ) or die "Cannot open \"$configfile\": $!";
    while (<CONFIG>) {
	last if /^%\s*PCONTROL/;
	next if /^%/;
	next if /^#/;
	next if /^\s*$/;
	chomp;
	s/\s//g;
	my ($configparamname) = split( /=/, $_, 2 );  # save original parameter name
	s/([A-Z])/\l$1/g;  # tolower
	if (my ($configparam, $configvalue) = split( /=/, $_, 2 )) {
	    $configdata{$configparam} = $configvalue;
	    $configparamnames{$configparam} = $configparamname;
	}
    }
    close( CONFIG );
}


sub printdefaults {
    my $defconfigfile = shift;

    open( DEFCONFIG, $defconfigfile ) or die "Cannot open \"$defconfigfile\": $!";
    print "% PHOST\n";
    while (<DEFCONFIG>) {
	next unless /^CFDefine/;
	chomp;
	s/\s//g;
	if (my ($defconfigparam, $defconfigvalue) = /^CFDefine\(([^,]+)[^"]+"(.+?)".+\)$/) {
	    printf( "%-29s = %s\n", $defconfigparam, $defconfigvalue);
	}
    }
    close( DEFCONFIG );
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
    die "usage: $commandname [-h (help)] [-d (defaults)] [-n (non-defaults)]\n";
}
