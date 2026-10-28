#!/bin/bash
# Builds webcam-hz, installs it, and starts a background job that keeps
# the webcam on your chosen anti-flicker setting.
# Usage: ./install.sh [50|60|auto|off]   (default: 50)
set -e
HZ="${1:-50}"
case "$HZ" in 50|60|auto|off) ;; *) echo "Usage: ./install.sh [50|60|auto|off]"; exit 2;; esac

BIN="$HOME/.local/bin"
LABEL="io.github.webcam-hz"
PLIST="$HOME/Library/LaunchAgents/$LABEL.plist"
cd "$(dirname "$0")"

make
mkdir -p "$BIN"
cp webcam-hz keep-hz.sh "$BIN/"
chmod +x "$BIN/webcam-hz" "$BIN/keep-hz.sh"

cat > "$PLIST" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key><string>$LABEL</string>
  <key>ProgramArguments</key>
  <array><string>$BIN/keep-hz.sh</string><string>$HZ</string></array>
  <key>RunAtLoad</key><true/>
  <key>StartInterval</key><integer>60</integer>
  <key>StandardOutPath</key><string>$HOME/Library/Logs/webcam-hz.log</string>
  <key>StandardErrorPath</key><string>$HOME/Library/Logs/webcam-hz.log</string>
</dict>
</plist>
PLIST

launchctl bootout "gui/$(id -u)/$LABEL" 2>/dev/null || true
launchctl bootstrap "gui/$(id -u)" "$PLIST"
echo "Installed. Webcam is kept at $HZ. Log: ~/Library/Logs/webcam-hz.log"
