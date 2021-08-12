#!/bin/sh
WATCHDIR="${WATCHDIR-/dev/input/by-path}"
WATCHDIR="$(echo "$WATCHDIR" | sed 's#/*$##')"

[ -z "$WATCHDIR" ] && {
	echo WATCHDIR cannot be empty >&2
	exit 1
}

DEPS=
DEPS="$DEPS capsctrl"
DEPS="$DEPS inotifywait"

for DEP in $DEPS
do command -v $DEP >/dev/null || { echo "$DEP not found" >&2; exit 1; }
done

inotifywait_args=
inotifywait_args="$inotifywait_args -q"
inotifywait_args="$inotifywait_args -e create"
inotifywait_args="$inotifywait_args --include '.*-event-kbd'"
inotifywait_args="$inotifywait_args --format %f"
inotifywait_args="$inotifywait_args --no-newline"

while true
do capsctrl "$WATCHDIR/$(inotifywait $inotifywait_args $WATCHDIR)" &
done

# vim: set nowrap ft=sh:
