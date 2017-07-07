# Peripheral Clock

## Synopsis

```
$ ./rpio clock help
arguments: MODE [help]

MODE : count   # verify clock resolution
     | set     # configure ARM counter
     | status  # display clock values

The supported clock sources are:
* arm    : ARM Free Running Counter (B00:420+4)
* bcm    : BCM System Timer Peripheral Counter (003:004+8)
* chrono : C++ chrono::steady_clock (clock_gettime(CLOCK_MONOTONIC))

The clock resolutions are (seconds):
* arm    : up to 2.5E-9
* bcm    : 1E-6
* chrono : 1E-9
```
