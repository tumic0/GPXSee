#ifndef MVT_DATA_H
#define MVT_DATA_H

#include <QByteArray>
#include <QVector>
#include <QVariant>

namespace MVT {

class Data
{
public:
	enum GeomType {
		UNKNOWN = 0,
		POINT = 1,
		LINESTRING = 2,
		POLYGON = 3
	};

	struct Feature
	{
		Feature() : id(0), type(UNKNOWN) {}

		quint64 id;
		QVector<quint32> tags;
		GeomType type;
		QVector<quint32> geometry;
	};

	struct Layer
	{
		Layer() : version(1), extent(4096) {}

		quint32 version;
		QByteArray name;
		QVector<Feature> features;
		QVector<QByteArray> keys;
		QVector<QVariant> values;
		quint32 extent;
	};

	Data(const QByteArray &ba);
	const QVector<Layer> &layers() const {return _layers;}

private:
	QVector<Layer> _layers;
};

}

#endif // MVT_DATA_H
