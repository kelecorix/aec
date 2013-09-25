#!/bin/bash
while true
do
	echo '1=1' > /dev/pi-blaster
	sleep 2
	echo '1=0' > /dev/pi-blaster
	sleep 120
done
