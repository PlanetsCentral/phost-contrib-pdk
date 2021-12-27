#
#  Building PDK with the Accidental Build System
#

load_module('Compiler.pl');

my $IN = get_variable('IN');
my $OUT = get_variable('OUT');

add_variable(LM => '-lm');
add_to_variable(CFLAGS => "-I$IN");
add_to_variable(CFLAGS => "-Wall -W -Wno-type-limits -Wno-format-zero-length");

# Sources
my @SRCS = qw(auxdata.c pconfig.c crc.c enum.c hostio.c main.c portable.c
        vgapdata.c vgapmsg.c vgaputil.c worm.c doscvt.c vgapdat1.c
        vgapdat2.c vgapdat3.c vgapdat4.c vgapdat5.c msgtmp.c
        rstmsg.c pgetopt.c points.c alldata.c vgapdat6.c vgapdat7.c vgapdat8.c vgapdat9.c
        vgapdatb.c vgapdatc.c battle.c hullfunc.c listmat.c utildata.c fcode.c commands.c
        exp.c tons.c ion.c);

# Public headers (just for export)
my @public_headers = to_prefix_list($IN, qw(phostpdk.h version.h));

# Libraries
my @libs = (
    compile_static_library('pdk', [to_prefix_list($IN, @SRCS)], [get_variable('LM')])
    );

# Utilities
my @utils = (
    compile_executable('sendmess', ["$IN/sendmess.c"], ['pdk']),
    compile_executable('killrace', ["$IN/killrace.c"], ['pdk']),
    compile_executable('crack',    ["$IN/crack.c"],    ['pdk']),
    compile_executable('ptscore',  ["$IN/ptscore.c"],  ['pdk']),
    compile_executable('pmaster',  ["$IN/pmaster.c"],  ['pdk']),
    compile_executable('pally',    ["$IN/pally.c"],    ['pdk']),
    );

# Examples
my @examples = map {
    my ($dir, $base, $ext) = split_filename($_);
    compile_executable($base, $_, 'pdk')
} glob("$IN/ex[0-9]*.c");

# Documentation
my @doc = (
    generate("$OUT/pdk.inf",     "$IN/pdk.texi", 'makeinfo --no-split -o $@ $<'),
    generate("$OUT/pdk-api.txt", "$IN/pdk.texi", 'makeinfo --no-split --no-headers -o $@ $<'),
);
generate(doc => [@doc]);
rule_set_phony('doc');

# Installation
my $result = add_variable(prefix => 'result');
generate(install => [
             generate('install-bin', [
                          generate_copy_to_dir("$result/bin", @utils),
                          generate_copy_to_dir("$result/lib", @libs),
                          generate_copy_to_dir("$result/include", @public_headers),
                          generate_copy_to_dir("$result/legacy", @public_headers, @libs),
                      ]),
             generate('install-doc', [
                          generate_copy_to_dir("$result/doc", @doc),
                      ])
             ]);
rule_set_phony('install', 'install-bin', 'install-doc');

# Distribution
my @dist_files = (@public_headers, @libs, @doc);
generate(dist => generate('pdk.zip', [@dist_files],
                          'zip -j $@ '.join(' ', @dist_files)));
rule_set_phony('dist');
