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
		LigoJSONFormat,
		PittasoftFormat,
		DJIFormat
	};

	struct STSCEntry {
		STSCEntry() {}
		STSCEntry(quint32 first, quint32 samples, quint32 id)
		  : first(first), samples(samples), id(id) {}

		quint32 first;
		quint32 samples;
		quint32 id;
	};

	struct STTSEntry {
		STTSEntry() {}
		STTSEntry(quint32 samples, quint32 duration)
		  : samples(samples), duration(duration) {}

		quint32 samples;
		quint32 duration;
	};

	struct Metadata {
		Metadata() : format(UnknownFormat), scale(0), id(0) {}

		Format format;
		quint32 scale;
		quint32 id;
		QVector<STSCEntry> stscTable;
		QVector<STTSEntry> sttsTable;
		QVector<quint32> sizes;
		QVector<quint64> chunks;
	};

	bool mp4(QFile *file, Metadata &meta, Waypoint &wpt);
	bool metadata(QFile *file, const Metadata &meta, const QDateTime &start,
	  SegmentData &segment);
	bool gpmf(QFile *file, quint64 offset, quint32 size, SegmentData &segment,
	  quint64 &timeShift);
	bool rtmf(QFile *file, quint64 offset, quint32 size, SegmentData &segment);
	bool camm(QFile *file, quint64 offset, quint32 size,
	  const QDateTime &timestamp, SegmentData &segment);
	bool novatek(QFile *file, quint64 offset, quint32 size,
	  SegmentData &segment);
	bool ligoJSON(QFile *file, quint64 offset, quint32 size,
	  SegmentData &segment);
	bool pittasoft(QFile *file, quint64 offset, quint32 size,
	  SegmentData &segment);
	bool dji(QFile *file, quint64 offset, quint32 size,
	  const QDateTime &timestamp, SegmentData &segment);

	static bool atoms(QDataStream &stream, Metadata &meta, Waypoint &wpt);
	static bool moov(QDataStream &stream, quint64 atomSize, Metadata &meta,
	  Waypoint &wpt);
	static bool udta(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool free(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool free2(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool trak(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool mdia(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool minf(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool stsd(QDataStream &stream, quint64 atomSize, Format &format,
	  quint32 &id);
	static bool stbl(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool stsc(QDataStream &stream, quint64 atomSize,
	  QVector<STSCEntry> &table);
	static bool stts(QDataStream &stream, quint64 atomSize,
	  QVector<STTSEntry> &table);
	static bool gps(QDataStream &stream, quint64 atomSize, Metadata &meta);
	static bool gpsf(QDataStream &stream, quint64 atomSize, Metadata &meta);

	QString _errorString;
};

#endif // MP4PARSER_H
