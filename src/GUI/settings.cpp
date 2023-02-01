#include <QGeoPositionInfoSource>
#include "settings.h"

static QString defaultPlugin()
{
	QString source;

	QGeoPositionInfoSource *ps = QGeoPositionInfoSource::createDefaultSource(0);
	if (ps) {
		source = ps->sourceName();
		delete ps;
	}

	return source;
}

const QString &Settings::positionPlugin()
{
	static QString plugin(defaultPlugin());
	return plugin;
}
