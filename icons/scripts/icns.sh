#!/bin/bash

EXTENSIONS="fit:#006600 gpx:#003399 igc:#ff3300 kml:#990000 nmea:#0083d7 \
  plt:#66ff00 rte:#66ff00 tcx:#ffcc00 wpt:#66ff00"

for e in $EXTENSIONS; do
	IFS=":"; set $e

	EXT=`echo $1 | tr /a-z/ /A-Z/`
	sed -e "s/\$EXTENSION/$EXT/" -e "s/\$COLOR/$2/" icon-template.svg > $1.svg

	ICONSET=$1.iconset
	mkdir $ICONSET

	convert -density 400 -background none -resize '16x16' "$1.svg" "$ICONSET/icon_16x16.png"
	convert -density 400 -background none -resize '32x32' "$1.svg" "$ICONSET/icon_16x16@2x.png"
	cp "$ICONSET/icon_16x16@2x.png" "$ICONSET/icon_32x32.png"
	convert -density 400 -background none -resize '64x64' "$1.svg" "$ICONSET/icon_32x32@2x.png"
	convert -density 400 -background none -resize '128x128' "$1.svg" "$ICONSET/icon_128x128.png"
	convert -density 400 -background none -resize '256x256' "$1.svg" "$ICONSET/icon_128x128@2x.png"
	cp "$ICONSET/icon_128x128@2x.png" "$ICONSET/icon_256x256.png"
	convert -density 400 -background none -resize '512x512' "$1.svg" "$ICONSET/icon_256x256@2x.png"
	cp "$ICONSET/icon_256x256@2x.png" "$ICONSET/icon_512x512.png"
	convert -density 400 -background none -resize '1024x1024' "$1.svg" "$ICONSET/icon_512x512@2x.png"

	iconutil -c icns -o $1.icns "$ICONSET"
	rm -R "$ICONSET" $1.svg
done
