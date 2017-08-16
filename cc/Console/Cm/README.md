# Peripheral Clock Manager

See [CM](../../Rpi/Cm.md) description in the library section.

## Synopsis

```
$ rpio cm help
arguments: MODE [help]

MODE : config  # configure clock generator
     | status  # display the status of all clock generators
     | switch  # switch clock generator on/off

There are seven clock generators specified by ALIAS:
ALIAS: gp0 | gp1 | gp2 | pcm | pwm | slim | uart
```

### Config

```
$ rpio cm config help
arguments: ALIAS [-f FRACT] [-i INT] [-m MASH] [-s SOURCE]

 FRACT : fractional part of the pre-scaler (0..FFF)
   INT :    integer part of the pre-scaler (0..FFF)
  MASH : apply 'mash' filter (0..3, where 0 means off)

SOURCE : the clock source (0..15); for example:
           1 = ocillator =    19.2 Mhz
           6 = PLLD      =   500.0 MHz
```

### Status

There are no command line arguments.

Example:

```
$ rpio cm status
alias on bsy src div:i div:f mash
---------------------------------
gp0    0   0   0     0     0    1
gp1    0   0   0    40     0    1
gp2    0   0   0     0     0    0
pcm    0   0   0     0     0    1
pwm    1   1   6    10     0    0
slim   0   0   0     0     0    1
uart   1   1   6    10  1707    1
```

### Switch

```
$ rpio cm switch help
arguments: ALIAS MODE
MODE: kill | off | on | toggle
```
