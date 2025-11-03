#ifndef MVT_VECTORTILE_H
#define MVT_VECTORTILE_H

#include <QVariant>
#include <QVector>
#include <QHash>
#include <QPainterPath>
#include "pbf.h"

typedef QHash<QByteArray, quint32> KeyHash;

namespace MVT {

class VectorTile
{
public:
	class Layer;

	class Feature
	{
	public:
		Feature() : _data(0), _layer(0) {}
		Feature(const PBF::Feature *data, const Layer *layer)
		  : _data(data), _layer(layer) {}

		const QVariant *value(const QByteArray &key) const;
		PBF::GeomType type() const {return _data->type;}
		const QPainterPath &path(int tileSize);

		bool operator<(const Feature &other) const
		  {return _data->id < other._data->id;}

	private:
		const PBF::Feature *_data;
		const Layer *_layer;
		QPainterPath _path;
	};

	class Layer
	{
	public:
		Layer(const PBF::Layer *layer);

		QVector<Feature> &features() {return _features;}
		const QVector<QVariant> &values() const {return _data->values;}
		const KeyHash &keys() const {return _keys;}
		const PBF::Layer *data() const {return _data;}

	private:
		const PBF::Layer *_data;
		QVector<Feature> _features;
		KeyHash _keys;
	};

	VectorTile(const PBF &data);
	~VectorTile();

	const QHash<QByteArray, Layer*> &layers() const {return _layers;}

private:
	QHash<QByteArray, Layer*> _layers;
};

}

#endif // MVT_VECTORTILE_H
