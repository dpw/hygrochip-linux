#!/bin/sh

HYT_BIN="/usr/local/sbin/hyt-read"

if [ "$1" = "autoconf" ]; then
	echo yes
	exit 0
fi

if [ "$1" = "config" ]; then
	echo 'graph_title HYT Humidity and Temperature'
	echo 'graph_args --base 1000 -l 0'
	echo 'graph_vlabel Humidity/Temperature'
	echo 'graph_scale yes'
	echo 'graph_category other'
	echo 'hyt_hum.label Humidity'
	echo 'hyt_temp.label Temperature'
	echo 'graph_info HYT Humidity and Temperature'
	echo 'hyt_hum.info Humidity'
	echo 'hyt_temp.info Temperature'
	exit 0
fi

$HYT_BIN -b bcm2708_i2c.1 | awk '{ print "hyt_hum.value " $1; print "hyt_temp.value " $2; }'
