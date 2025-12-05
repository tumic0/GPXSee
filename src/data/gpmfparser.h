#ifndef GPMFPARSER_H
#define GPMFPARSER_H

#include "parser.h"

class GPMFParser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	bool mp4(QFile *file, QVector<quint32> &sizes, QVector<quint64> &chunks);
	bool moov(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
	  QVector<quint64> &chunks);
	bool trak(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
	  QVector<quint64> &chunks);
	bool mdia(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
	  QVector<quint64> &chunks);
	bool hdlr(QDataStream &stream, quint64 atomSize, bool &mhlr);
	bool minf(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
	  QVector<quint64> &chunks);
	bool stbl(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes,
	  QVector<quint64> &chunks);
	bool stsd(QDataStream &stream, quint64 atomSize, bool &gpmd);
	bool stsz(QDataStream &stream, quint64 atomSize, QVector<quint32> &sizes);
	bool stco(QDataStream &stream, quint64 atomSize, QVector<quint64> &chunks);
	bool co64(QDataStream &stream, quint64 atomSize, QVector<quint64> &chunks);

	bool gpmf(QFile *file, quint64 offset, quint32 size, SegmentData &segment);

	QString _errorString;
};

#endif // GPMFPARSER_H
