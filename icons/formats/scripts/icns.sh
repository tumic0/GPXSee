#!/bin/bash

while read e; do
	IFS=":"; set $e

	EXT=`echo $1 | tr /a-z/ /A-Z/`
	sed -e "s/\$EXTENSION/$EXT/" -e "s/\$COLOR/$2/" icon-template.svg > $1.svg

	ICONSET=$1.iconset
	mkdir $ICONSET

	rsvg-convert -w 16 -h 16 -o "$ICONSET/icon_16x16.png" "$1.svg"
	rsvg-convert -w 32 -h 32 -o "$ICONSET/icon_16x16@2x.png" "$1.svg"
	cp "$ICONSET/icon_16x16@2x.png" "$ICONSET/icon_32x32.png"
	rsvg-convert -w 64 -h 64 -o "$ICONSET/icon_32x32@2x.png" "$1.svg"
	rsvg-convert -w 128 -h 128 -o "$ICONSET/icon_128x128.png" "$1.svg"
	rsvg-convert -w 256 -h 256 -o "$ICONSET/icon_128x128@2x.png" "$1.svg"
	cp "$ICONSET/icon_128x128@2x.png" "$ICONSET/icon_256x256.png"
	rsvg-convert -w 512 -h 512 -o "$ICONSET/icon_256x256@2x.png" "$1.svg"
	cp "$ICONSET/icon_256x256@2x.png" "$ICONSET/icon_512x512.png"
	rsvg-convert -w 1024 -h 1024 -o "$ICONSET/icon_512x512@2x.png" "$1.svg"

	iconutil -c icns -o $1.icns "$ICONSET"
	rm -R "$ICONSET" $1.svg
done < extensions
