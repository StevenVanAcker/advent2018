#!/bin/sh -e

SERVERUSER=nobody

export FLAGFILE=/opt/udpsanta/flag



cd /opt/udpsanta/userdata
echo "Creating user data"
make
cd /opt/udpsanta

while true;
do
	echo "Starting server"
	su -s /bin/sh -c ./server $SERVERUSER || true
	sleep 1;
done

