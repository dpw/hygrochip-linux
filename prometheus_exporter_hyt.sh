#!/bin/sh

HYT_BIN="/usr/local/sbin/hyt-read"
I2C_DEVICE="/dev/i2c-1"

$HYT_BIN -d $I2C_DEVICE | awk '{ print "hyt_humidity " $1; print "hyt_temperature " $2; }'
