#!/bin/bash

echo 1016 > /sys/class/gpio/unexport
echo 1016 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio1016/direction
echo 0 > /sys/class/gpio/gpio1016/value
sleep 1
echo 1 > /sys/class/gpio/gpio1016/value
./ssd1306_demo /dev/i2c-2
