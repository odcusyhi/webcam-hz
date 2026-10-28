#!/bin/bash
# Stops the background job and removes webcam-hz.
LABEL="io.github.webcam-hz"
launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null
rm -f "$HOME/Library/LaunchAgents/$LABEL.plist" "$HOME/.local/bin/webcam-hz" "$HOME/.local/bin/keep-hz.sh"
echo "Removed."
