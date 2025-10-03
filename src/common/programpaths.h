#ifndef PROGRAMPATHS_H
#define PROGRAMPATHS_H

#include <QString>

namespace ProgramPaths
{
	QString mapDir(bool writable = false);
	QString poiDir(bool writable = false);
	QString crsDir(bool writable = false);
	QString demDir(bool writable = false);
	QString styleDir(bool writable = false);
	QString symbolsDir(bool writable = false);
	QString tilesDir();
	QString translationsDir();

	QString ellipsoidsFile();
	QString gcsFile();
	QString projectionsFile();
	QString pcsFile();
}

#endif // PROGRAMPATHS_H
