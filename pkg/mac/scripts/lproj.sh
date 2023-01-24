#/bin/bash

for QM in ../../../lang/*.qm; do
	LANG=`echo $QM | sed s/.\*gpxsee_// | cut -d. -f1`
	DIR=../lproj/$LANG.lproj
	mkdir -p $DIR
	echo "<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple Computer//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
    <key>LprojCompatibleVersion</key>
    <string>123</string>
    <key>LprojLocale</key>
    <string>$LANG</string>
    <key>LprojRevisionLevel</key>
    <string>1</string>
    <key>LprojVersion</key>
    <string>123</string>
</dict>
</plist>" > $DIR/locversion.plist
done
