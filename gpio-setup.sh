#! /bin/bash

# BUTTONS
pigs modes 9  r     # green btn
pigs pud   9  u     # green pull-up
pigs modes 11 r     # blue btn
pigs pud   11 u     # blue pull-up

# LEDS
pigs modes 16 w     # red led
pigs modes 20 w     # green led
pigs modes 21 w     # blue led

# MOTOR STANDBY
pigs modes 25 w     # /stby
pigs w     25 0     # set standby

# LEFT MOTOR CONTROL
pigs modes 12 w     # pwma
pigs modes 10 w     # ain1
pigs modes 24 w     # ain2

# RIGHT MOTOR CONTROL
pigs modes 13 w     # pwmb
pigs modes 4  w     # bin1
pigs modes 27 w     # bin2

# LEFT MOTOR SENSE
pigs modes 23 r     # left_a
pigs modes 22 r     # left_b

# LEFT MOTOR SENSE
pigs modes 17 r     # right_a
pigs modes 18 r     # right_b

