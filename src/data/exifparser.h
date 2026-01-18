#ifndef EXIFPARSER_H
#define EXIFPARSER_H

#include <QDateTime>
#include "parser.h"

class TIFFFile;

class EXIFParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	struct IFDEntry {
		IFDEntry() : type(0), count(0), offset(0) {}

		quint16 type;
		qint64 count;
		qint64 offset;
	};

	bool parseTIFF(QFile *file, QVector<Waypoint> &waypoints);
	bool readIFD(TIFFFile &file, qint64 offset, const QSet<quint16> &tags,
	  QMap<quint16, IFDEntry> &entries) const;
	bool readEntry(TIFFFile &file, const QSet<quint16> &tags,
	  QMap<quint16, IFDEntry> &entries) const;

	double coordinate(TIFFFile &file, const IFDEntry &ll) const;
	Coordinates coordinates(TIFFFile &file, const IFDEntry &lon,
	  const IFDEntry &lonRef, const IFDEntry &lat, const IFDEntry &latRef) const;
	double altitude(TIFFFile &file, const IFDEntry &alt,
	  const IFDEntry &altRef) const;
	QTime time(TIFFFile &file, const IFDEntry &ts) const;
	QString text(TIFFFile &file, const IFDEntry &e) const;

	QString _errorString;
};

#endif // EXIFPARSER_H
