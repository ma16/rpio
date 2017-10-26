# Peripheral GPIO

## Synopsis

```
$ ./rpio peripheral gpio help
arguments: MODE [help]

MODE : enable  # enable event detection
     | mode    # set mode
     | output  # set level
     | pull    # switch resistor
     | reset   # reset detected events
     | status  # display status

Arguments may require a PINS value:

PINS : NO             # a single pin number
     | -l NO[,NO]...  # a set of pin numbers as list
     | -m MASK        # a set of pin number as bit mask
     | all            # all pins
```
