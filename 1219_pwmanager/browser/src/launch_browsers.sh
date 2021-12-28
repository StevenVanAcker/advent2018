#!/bin/bash

addir=$(readlink -f $1)
scriptdir=$(dirname $(readlink -f $0))
maxjobs=${2:-10}

cd $scriptdir
while true;
do
	find $addir -type f -exec basename {} \; | \
		parallel --will-cite --max-args 1 --jobs $maxjobs timeout 20 phantomjs ./browser.js {1}
	sleep 3
done

