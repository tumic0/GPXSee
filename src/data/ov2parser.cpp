#include <QDataStream>
#include "common/textcodec.h"
#include "ov2parser.h"

bool OV2Parser::parse(QFile *file, QList<TrackData> &tracks,
  QList<RouteData> &routes, QList<Area> &polygons, QVector<Waypoint> &waypoints)
{
	Q_UNUSED(tracks);
	Q_UNUSED(routes);
	Q_UNUSED(polygons);
	QDataStream stream(file);
	quint8 type;
	quint32 len;
	qint32 lon, lat;
	QByteArray ba;
	TextCodec codec(1252);

	stream.setByteOrder(QDataStream::LittleEndian);

	while (!stream.atEnd()) {
		stream >> type;
		switch (type) {
			case 0:
				stream >> len;
				if (stream.status() != QDataStream::Ok || len < 5
				  || stream.skipRawData(len - 5) < (int)len - 5) {
					_errorString = "Corrupted deleted record";
					return false;
				}
				break;
			case 1:
				if (stream.skipRawData(20) < 20) {
					_errorString = "Corrupted skipper record";
					return false;
				}
				break;
			case 2:
			case 3:
				{stream >> len >> lon >> lat;
				if (stream.status() != QDataStream::Ok || len < 13) {
					_errorString = "Corrupted POI record";
					return false;
				}
				ba.resize(len - 13);
				if (stream.readRawData(ba.data(), ba.size()) != ba.size()) {
					_errorString = "Corrupted POI record";
					return false;
				}
				if (lon < -18000000 || lon > 18000000
				  || lat < -9000000 || lat > 9000000) {
					_errorString = "Invalid POI coordinates";
					return false;
				}
				Waypoint wp(Coordinates(lon/1e5, lat/1e5));
				QList<QByteArray> parts(ba.split('\0'));
				int pp = parts.first().indexOf('>');
				if (pp >= 0) {
					wp.setName(codec.toString(parts.first().left(pp)));
					wp.setPhone(parts.first().mid(pp+1));
				} else
					wp.setName(codec.toString(parts.first()));
				waypoints.append(wp);}
				break;
			default:
				_errorString = QString("%1: invalid/unknown record type")
				  .arg(type);
				return false;
		}
	}

	return true;
}
