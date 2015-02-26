#!/bin/sh

if [ "X$1" = "X" ]; then
	echo "Usage: $0 dstaddr"
	exit -1
fi

sudo insmod wtun.ko dst_addr=$1

sudo iwconfig wlan0 mode ad-hoc
sudo iwconfig wlan0 essid test
sudo ifconfig wlan0 up
