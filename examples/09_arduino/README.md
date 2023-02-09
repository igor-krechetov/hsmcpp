How to build PlatformIO based project
=====================================

1. [Install PlatformIO Core or PlatformIO IDE](https://docs.platformio.org/en/latest/core/installation/index.html)
2. Run these commands:

```shell
# Change directory to example
$ cd 01_blink

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Clean build files
$ pio run --target clean
```