#!/bin/bash

while read e; do
	IFS=":"; set $e

	EXT=`echo $1 | tr /a-z/ /A-Z/`
	sed -e "s/\$EXTENSION/$EXT/" -e "s/\$COLOR/$2/" icon-template.svg > $1.svg
	convert -density 400 $1.svg -define icon:auto-resize $1.ico
	rm $1.svg
done < extensions
