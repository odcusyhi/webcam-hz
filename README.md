# webcam-hz

Stop USB webcam flicker on macOS.

LED lights and fluorescent tubes pulse at the frequency of the mains power:
50 Hz in Europe and most of the world, 60 Hz in North America and parts of
Asia. If the webcam's anti-flicker setting doesn't match, the picture shows
bands or flickers. macOS has no setting for this, and many webcams (for example
the Logitech C920) reset to 60 Hz when they are unplugged or the Mac restarts.

`webcam-hz` sets the UVC "power line frequency" control directly over USB.
It needs no drivers, no kernel extension and no vendor software.

## Use

```sh
make
./webcam-hz          # show the current setting
./webcam-hz 50       # set 50 Hz (Europe, most of the world)
./webcam-hz 60       # set 60 Hz (North America)
./webcam-hz auto     # let the camera decide (not all cameras support it)
./webcam-hz off
```

## Keep the setting permanently

```sh
./install.sh 50      # or 60
```

This builds the tool, copies it to `~/.local/bin`, and installs a small
LaunchAgent. Every minute, and at login, it checks the camera and puts the
setting back if it changed. If no camera is plugged in, or the setting is
already right, it does nothing. Changes are logged to
`~/Library/Logs/webcam-hz.log`.

Remove it with `./uninstall.sh`.

## Requirements

- macOS (tested on macOS 26, Apple Silicon)
- Xcode Command Line Tools (`xcode-select --install`)
- A UVC webcam that exposes the power line frequency control (many do)

Tested with a Logitech HD Pro Webcam C920. Built-in Mac cameras are not supported.

## License

MIT
