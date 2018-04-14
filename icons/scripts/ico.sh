#!/bin/bash

EXTENSIONS="fit:#006600 gpx:#003399 igc:#ff3300 kml:#990000 nmea:#0083d7 \
  plt:#66ff00 rte:#66ff00 tcx:#ffcc00 wpt:#66ff00"

for e in $EXTENSIONS; do
	IFS=":"; set $e

	EXT=`echo $1 | tr /a-z/ /A-Z/`
	sed -e "s/\$EXTENSION/$EXT/" -e "s/\$COLOR/$2/" icon-template.svg > $1.svg
	convert -density 400 $1.svg -define icon:auto-resize $1.ico
	rm $1.svg
done
