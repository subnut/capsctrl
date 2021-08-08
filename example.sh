#!/bin/sh
if command -v inotifywait >/dev/null && command -v capsctrl >/dev/null
then while true
do capsctrl "/dev/input/by-path/$(inotifywait -q -e create --include '.*-event-kbd' --format '%f' --no-newline /dev/input/by-path)"
done fi &
# vim: set nowrap ft=sh:
