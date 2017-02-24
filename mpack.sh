#!/bin/bash

if [ "$1" != "" ]; then
	echo Compressing...
	rm $1.zip
	zip $1.zip energy_*
	zip -u $1.zip *.pcap
	zip -u $1.zip coap_trace.tr
	zip -u $1.zip coap_animation.xml
	zip -u $1.zip state_*
	echo Done
	echo Removing...
	rm energy_*
	rm *.pcap
	rm coap_trace.tr
	rm coap_animation.xml
	rm state_*
else
    echo "You did not write any name for the zip file"
fi

