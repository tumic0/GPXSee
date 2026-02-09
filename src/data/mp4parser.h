#ifndef MP4PARSER_H
#define MP4PARSER_H

#include "parser.h"

class MP4Parser : public Parser
{
public:
	bool parse(QFile *file, QList<TrackData> &tracks, QList<RouteData> &routes,
	  QList<Area> &polygons, QVector<Waypoint> &waypoints);
	QString errorString() const {return _errorString;}
	int errorLine() const {return 0;}

private:
	enum Format {
		UnknownFormat,
		GPMDFormat,
		RTMDFormat,
		CAMMFormat,
		NovatekFormat,
		LigoJSONFormat
	};

	struct Table {
		Table() {}
		Table(quint32 first, quint32 samples, quint32 id)
		  : first(first), samples(samples), id(id) {}

		quint32 first;
		quint32 samples;
		quint32 id;
	};

	struct Metadata {
		Metadata() : format(UnknownFormat) {}

		Format format;
		quint32 id;
		QVector<Table> tables;
		QVector<quint32> sizes;
		QVector<quint64> chunks;
	};

	bool mp4(QFile *file, Metadata &meta, Waypoint &wpt);
	bool metadata(QFile *file, const Metadata &meta, SegmentData &segment);
	bool gpmf(QFile *file, quint64 offset, quint32 size, SegmentData &segment);
	bool rtmf(QFile *file, quint64 offset, quint32 size, SegmentData &segment);
	bool camm(QFile *file, quint64 offset, quint32 size, SegmentData &segment);
	bool novatek(QFile *file, quint64 offset, quint32 size,
	  SegmentData &segment);
	bool ligoJSON(QFile *file, quint64 offset, quint32 size, SegmentData &segment);

	static bool atoms(QDataStream &stream, Metadata &meta, Waypoint &wpt);
	static bool moov(QDataStream &stream, quint64 atomSize, Metadata &meta,
	  Waypoint &wpt);
	static bool udtaG(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool trak(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool mdia(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool minf(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool stsd(QDataStream &stream, quint64 atomSize, Format &format,
	  quint32 &id);
	static bool stbl(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool stsc(QDataStream &stream, quint64 atomSize,
	  QVector<Table> &tables);
	static bool gps(QDataStream &stream, quint64 atomSize, Metadata &meta);

	QString _errorString;
};

#endif // MP4PARSER_H
