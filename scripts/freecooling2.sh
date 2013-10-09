#!/bin/bash
while true
do
cd /opt/owfs/freecooling/
/opt/owfs/freecooling/freecooling2 -d 4 -p 0
echo "sigfolt" >> /var/log/freecooling2_sh.log
sleep 5
done

