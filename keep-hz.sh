#!/bin/bash
# Keeps the webcam anti-flicker at the given setting. Run by launchd every minute.
# Does nothing when no camera is plugged in or the setting is already right.
WANT="${1:-50}"
case "$WANT" in 50|60) LABEL="$WANT Hz";; *) LABEL="$WANT";; esac
TOOL="$(dirname "$0")/webcam-hz"
now=$("$TOOL" 2>/dev/null)
[ -z "$now" ] && exit 0
echo "$now" | grep -q "= $LABEL\$" && exit 0
"$TOOL" "$WANT" && echo "$(date '+%F %T') reset to $LABEL"
