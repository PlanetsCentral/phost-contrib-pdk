#!/usr/bin/perl -w
#
#  C Module Splitter
#
#  Usage: splitmod file.c           (generates file_1.c .. file_N.c)
#         splitmod file.c out%file  (generates out1file .. outNfile)
#
#  Special cases: splitmod -        (read stdin, write stdout)
#                 splitmod foo -    (read foo, write stdout)
#
#  Each function/definition goes in one file. Things that must go
#  together (static functions, maybe) must be put into groups using
#  special comments: /* GROUP name */ The name can be a word, or
#  a list of words separated by comma (no whitespace!). Groups of
#  many functions can be made with /* GROUP name { */ (start) and
#  /* GROUP } */ (end).
#
#  Declarations / includes are automatically repeated in all output
#  files; a declaration (local struct definition, maybe) can be forced
#  into a particular file by preceding it with a /* GROUP name */ comment.
#
#  To prevent a file from being split, just start it with an open brace
#  /* GROUP anything { */ with no close brace.
#
#  Results:
#  + normal compilation (libpdk.a):
#     total memory allocated to sections: 152340
#     file sizes (unstripped):
#       -rw-------   1 stefan   users      295982 Mar 20 19:01 libpdk.a
#       -rwx------   1 stefan   users      158746 Mar 20 19:01 crack
#       -rwx------   1 stefan   users      160805 Mar 20 19:01 killrace
#       -rwx------   1 stefan   users      176763 Mar 20 19:01 pmaster
#     stripped:
#       -rwx------   1 stefan   users      132980 Mar 20 19:07 crack
#       -rwx------   1 stefan   users      134868 Mar 20 19:07 killrace
#       -rwx------   1 stefan   users      150004 Mar 20 19:07 pmaster
#  + compilation with split modules (libpdks.a):
#     total memory allocated to sections: 128741
#     file sizes (unstripped):
#       -rw-------   1 stefan   users      507712 Mar 20 18:59 libpdks.a
#       -rwx------   1 stefan   users      116617 Mar 20 19:06 cracks
#       -rwx------   1 stefan   users      118644 Mar 20 19:06 killraces
#       -rwx------   1 stefan   users      114272 Mar 20 19:06 pmasters
#     stripped:
#       -rwx------   1 stefan   users       92696 Mar 20 19:07 cracks
#       -rwx------   1 stefan   users       94552 Mar 20 19:07 killraces
#       -rwx------   1 stefan   users       87968 Mar 20 19:07 pmasters
#
#  "total memory allocated to sections":
#    size $LIB | awk '{sum+=$4}END{print sum}'
#  libpdks has smaller values here because splitting effectively disables
#  inlining for many functions. Hence the resulting programs are slower.
#  + 100 runs of pmaster (normal):
#       real    0m16.421s
#       user    0m12.450s
#       sys     0m3.600s
#  + 100 runs of pmasters (small version):
#       real    0m18.236s
#       user    0m14.300s
#       sys     0m3.600s
#  (standard configuration, everything at "random rich", "...high" etc.)
#
#  Public Domain 2002 by Stefan Reuther <Streu@gmx.de>
#

use strict;
use less time;                  # never give up hope :-)

########################## Command Line Handling ##########################

if ((@ARGV != 1 && @ARGV != 2) || $ARGV[0] =~ /^-./) {
    print STDERR "usage: $0 input-file [output-template]\n";
    exit 1;
}

my $arg_input = $ARGV[0];
my $arg_output;
if (@ARGV > 1) {
    $arg_output = $ARGV[1];
    if ($arg_output !~ /%/ && $arg_output ne '-') {
        print STDERR "$0: output template must contain `%'\n";
        exit 1;
    }
} else {
    $arg_output = $arg_input;
    if ($arg_output ne '-') {
        $arg_output =~ s/\.(?=[^.]$ )/_\%./x or $arg_output .= '%';
    }
}

############################# Group Overrides #############################

my %group_override;
my $keep_override = 0;
if (open OVER, "< splitmod.def") {
    while (defined(my $line = <OVER>)) {
        chomp $line;
        next if $line =~ /^\s*;/ || $line =~ /^\s*$/;
        if ($line =~ /^\s*(\w+)\s*:\s*(.*)/) {
            my ($group, $syms) = ($1, $2);
            foreach (split /\s*,\s*/, $syms) {
                print STDERR "warning: symbol $_ put in more than one group\n"
                    if exists $group_override{$_};
                $group_override{$_} = $group;
            }
        } elsif ($line =~ /^\s*keep\s+([^:]+)$/) {
            foreach (split /\s*,\s*/, $1) {
                if ($arg_input eq $_ || $arg_input =~ m|/$_$|) {
                    $keep_override = 1;
                    last;
                }
            }
        } else {
            print STDERR "warning: didn't understand line `$_' in splitmod.def\n";
        }
    }
    close OVER;
}

############################# Input Splitting #############################

# Split input into parts, one declaration/definition/include/comment
# per piece.
open INPUT, "< $arg_input" or die "$arg_input: $!\n";
$_ = join "", <INPUT>;
close INPUT;
my @module;
if ($keep_override) {
    push @module, $_;
} else {
    while (1) {
        if (m{\G\s+}sgc) {
            # nothing
        } elsif (   m{\G//.*?\n}sgc 
                 || m{\G/\*.*?\*/}sgc) {
            push @module, $&;
        } elsif (m|\G^\s*\#([^\"\'](?!/[/\*]))*?|mgc) {
            # Preprocessor command. This one is hairy. The above regexp
            # matches anything up to a string or comment.
            my $here = $&;
            while (   m{\G//.*?\n}sgc            #  // comment
                   || m{\G/\*.*?\*/}sgc          #  /* comment */
                   || m{\G"([^\"\\]|\\.)*"}sgc   #  string
                   || m{\G'([^\'\\]|\\.)*'}sgc   #  character
                   || m{\G[^/\"\'\n]*}sgc        #  anything but these four
                   || m{\G\\\n}sgc               #  continuation
                   || m{\G[^\n]}sgc) {           #  anything else
                $here .= $&;
            }
            push @module, $here;
        } else {
            # very simplicistic C definition "parser". This gets confused by
            # unbalanced braces, as in
            #    void bar() {
            #    #if foo
            #         if (test1) {
            #    #else
            #         if (test2) {
            #    #endif
            #            dosomething();
            #         }
            #    }
            # You can help it out by placing /* END BLOCK */ after the
            # last brace.
            my $item = "";
            my $brace = 0;
            my $is_typedef = 0;
            if (m{\G\s*(struct|union|enum|class|typedef)[\s\w]*\{}sgc) {
                # it is a type definition. For these, the last closing brace
                # does NOT end this part:
                #   typedef struct { ... } foo, *pfoo;
                $item .= $&;
                $is_typedef = 1;
                $brace = 1;
            }
            while (1) {
                m{\G[\w\s]+}sgc and do { $item .= $&; next; };
                m{\G\{}sgc and do { $item .= $&; ++$brace; next; };
                m{\G\}\;?}sgc and do { $item .= $&; next if --$brace>0 || ($is_typedef && $& eq '}'); last; };
                m{\G"([^\"\\]|\\.)*"}sgc and do { $item .= $&; next; };
                m{\G'([^\'\\]|\\.)*'}sgc and do { $item .= $&; next; };
                m{\G/\*(.*?)\*/}sgc and do { $item .= $&; last if $1 =~ /\bEND\s?BLOCK\b/; next; };
                m{\G//(.*?)\n}sgc and do { $item .= $&; last if $1 =~ /\bEND\s?BLOCK\b/; next; };
                m{\G;}sgc and do { $item .= $&; next if $brace; last; };
                m{\G.}sgc and do { $item .= $&; next; };
                last;
            }
            push @module, $item
                if $item =~ /\W/;
            last if pos >= length;
        }
    }
}

######################### Distributing into groups ########################

my $nextgroup;                  # the group we're in
my $nextgroup2;
my $counter = 0;
my @groups;

foreach (@module) {
    if ($keep_override) {
        push @groups, "EVERYTHING";
    } elsif (m{^//} || m{^/\*}) {
        # comment
        if (/\bGROUP\s+([\w,]+)\s*(\{?)/) {
            print STDERR "warning: group `$nextgroup2' not closed\n"
                if defined $nextgroup2;
            $nextgroup = $1;
            $nextgroup2 = ($2 ne "") ? $1 : undef;
        } elsif (/\bGROUP\s*\}/) {
            print STDERR "warning: `GROUP }' while we are not in a group\n"
                if !defined $nextgroup2;
            $nextgroup = $nextgroup2 = undef;
        }
        push @groups, "";
    } elsif (m{^\s*\#}s || /^(extern|using|typedef)\s[^\{]*$/s) {
        print STDERR "warning: GROUP comment before declaration/header element\n"
            if defined $nextgroup && !defined $nextgroup2;
        push @groups, "*header";
    } elsif (/^(typedef|struct|class|union|enum)([\s\w]*)\{/s) {
        # type definitions are header by default, but can be forced
        # into a group.
        if (!defined $nextgroup) {
            foreach (split /\W+/, $2) {
                if (exists $group_override{$_}) {
                    $nextgroup = $group_override{$_};
                    last;
                }
            }
        }
        push @groups, defined($nextgroup) ? $nextgroup : "*header";
        $nextgroup = $nextgroup2;
    } else {
        if (!defined $nextgroup and /^([^\{]+)/) {
            foreach (split /\W+/, $1) {
                if (exists $group_override{$_}) {
                    $nextgroup = $group_override{$_};
                    last;
                }
            }
        }
        my $thisgroup = defined($nextgroup) ? $nextgroup : "item ".++$counter;
        push @groups, $thisgroup;
        $nextgroup = $nextgroup2;
    }
}

# Now make up the group list
my @grouplist;
foreach (@groups) {
    next if $_ eq "" || $_ eq "*header";
    foreach my $g (split /,/) {
        push @grouplist, $g unless grep { $g eq $_ } @grouplist;
    }
}

################################## Output #################################

# Pretty simple, slow algorithm. But fast enough ;-)

my $file_counter = 0;
foreach my $thisgroup (@grouplist) {
    my @output;
    foreach (0 .. $#module) {
        next if $groups[$_] eq "";       # ignore comments
        if ($groups[$_] eq "*header") {
            # header processing. Be a bit smart about #ifs. Should we
            # be smart about #define/#undef (to prevent the DOSCVT_Name's
            # from piling up, e.g.)? Probably it's not worth the trouble.
            if ($module[$_] =~ /^\#\s*endif/s) {
                if (@output && $output[-1] =~ /^\#\s*if/s) {
                    # if / endif
                    pop @output;
                    next;
                }
                if (@output > 1 && $output[-1] =~ /^\#\s*else/s
                    && $output[-2] =~ /^\#\s*if/s) 
                {
                    # if / else / endif
                    pop @output;
                    pop @output;
                    next;
                }
            }
            push @output, $module[$_];
        } elsif (grep { $thisgroup eq $_ } split /,/, $groups[$_]) {
            push @output, $module[$_];
        }
    }
    ++$file_counter;
    if ($arg_output eq '-') {
        print "======= file #$file_counter ($thisgroup) =======\n";
        print join("\n", @output), "\n";
    } else {
        my $outfn = $arg_output;
        $outfn =~ s/%/$file_counter/;
        open OUTPUT, "> $outfn" or die "can't create $outfn: $!\n";
        print OUTPUT join("\n", @output), "\n";
        close OUTPUT;
    }
}

exit 0;
