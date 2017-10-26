# Peripheral Clock Manager

See [CM](../../Rpi/Cm.md) description in the library section.

## Synopsis

```
$ rpio peripheral cm help
arguments: MODE [help]

MODE : ctl     # configure Control register
     | div     # configure Divider register
     | status  # display the status of all clock generators

There are seven clock generators specified by ALIAS:
ALIAS: gp0 | gp1 | gp2 | pcm | pwm | slim | uart
```

### Setup: Control Register (CTL)

```
$ rpio peripheral cm ctl help
arguments: ALIAS OPTION+

 [-+]enab  : enable the clock
 [-+]flip  : invert output signal
 [-+]kill  : reset clock
 mash 0..3 : audio-band noise-shaping
 src 0..15 : the clock source; for example:
             1 = ocillator @  19.2 Mhz
             6 = PLLD      @ 500.0 MHz

Each option writes consecutively to the register
```

### Setup: Divider Register (DIV)

```
$ rpio peripheral cm div help
arguments: ALIAS OPTION+

fract 0..4095 : fractional part of the divider
intgr 0..4095 : integer part of the divider
```

### Status

There are no command line arguments.

Example (after booting a Pi-0):

```
$ rpio peripheral cm status

alias on busy flip kill src div:i div:f mash
--------------------------------------------
gp0    0    0    0    0   0     0     0    1
gp1    0    0    0    0   0    40     0    1
gp2    0    0    0    0   0     0     0    0
pcm    0    0    0    0   0     0     0    1
pwm    0    0    0    0   0     0     0    1
slim   0    0    0    0   0     0     0    1
uart   1    1    0    0   6    10  1707    1
```
