# Free Running ARM Counter

## Synopsis

```
$ rpio clock help
arguments: MODE [help]

MODE : set     # switch on/off and set prescaler
     | sleep   # show counter increment when sleeping
     | status  # display status
```

```
$ rpio counter set help
arguments: MODE | DIV | MODE DIV

MODE | off  # disable ARM counter
     | on   #  enable ARM counter

DIV = pre-scaler (0..255)
```

```
$ rpio counter sleep help
argument: [-b] TIME

  -b: do busy-wait instead of sleeping
TIME: time to sleep in (fractions of) seconds
```

```
$ rpio counter status help
no arguments
```

## Examples

The counter is disabled after booting up:
```
$ rpio counter status
enabled.....off
prescaler...62
ticks.......0
```

Enable the counter and set the highest resolution:
```
$ rpio counter set on 0
```
The resolution is (DIV+1) / 250 MHz = 4ns. Note that the driving clock is fixed to 250 MHz on the Pi-2. However, on the Pi-0 and on the Pi-3, the clock varies between 250 MHz and 400 MHz (depending on the load).

Example (on a Pi-3):
```
$ rpio counter sleep 1
ticks=250085421 elapsed=1.000e+00s ticks/s=2.500e+08
$ rpio counter sleep -b 1
ticks=390975035 elapsed=1.000e+00s ticks/s=3.910e+08
```

You may want to fix the clock to 250 MHz by setting /boot/config.txt:core_freq=250.
