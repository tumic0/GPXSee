#include <QFile>
#include <QLineF>
#include "ll.h"
#include "gpx.h"


bool GPX::loadFile(const QString &fileName)
{
	QFile file(fileName);
	bool ret;

	_tracks.clear();
	_error.clear();
	_errorLine = 0;

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		_error = qPrintable(file.errorString());
		return false;
	}

	if (!(ret = _parser.loadFile(&file))) {
		_error = _parser.errorString();
		_errorLine = _parser.errorLine();
	}
	file.close();

	return ret;
}
