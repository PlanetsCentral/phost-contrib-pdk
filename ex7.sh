#!/bin/sh
#
#  Shell script for example 7
#

# path to program (relative to $ROOT)
SENDFILE=./ex7

# game root
ROOT=.

ALL="1 2 3 4 5 6 7 8 9 10 11"
ERRORS=0

# send_file <file> <pl1> <pl2> <pl3...>
#   Send file to player(s) unconditionally
send_file() {
  file="$1"
  shift
  for i in $@; do
    { cd "$ROOT" && $SENDFILE $i "$file"; } || ERRORS=1
  done
}

# send_file <file> <pl1> <pl2> <pl3...>
#   Send file to player(s) if it has changed. Uses a directory
#   `timestamps' to keep track of times.
send_file_if_newer() {
  file="$1"
  if test "$ROOT/$file" -nt "$ROOT/timestamps/$1" || test ! -f "$ROOT/timestamps/$1"; then
    test -d "$ROOT/timestamps" || mkdir "$ROOT/timestamps"
    touch "$ROOT/timestamps/$1"
    send_file $@
  fi
}

### okay, here comes da fun.

# all players receive race.nm each turn
send_file race.nm $ALL

# player 5 is the developer of an addon called `crack', and gets
# sent its logfile each turn ;-)
send_file crack.log 5

# all players get the pconfig if it changed
send_file_if_newer pconfig.src $ALL

if test $ERRORS = 1; then
  echo "Warning: there were errors sending the files" >&2
  echo "Warning: there were errors sending the files" >>host.log
  exit 1;
fi
