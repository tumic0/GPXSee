#ifndef MVT_TILE_H
#define MVT_TILE_H

#include <QVariant>
#include <QVector>
#include <QHash>
#include <QPainterPath>
#include "data.h"

typedef QHash<QByteArray, quint32> KeyHash;

namespace MVT {

class Tile
{
public:
	class Layer;

	class Feature
	{
	public:
		Feature() : _data(0), _layer(0) {}
		Feature(const Data::Feature *data, const Layer *layer)
		  : _data(data), _layer(layer) {}

		const QVariant *value(const QByteArray &key) const;
		Data::GeomType type() const {return _data->type;}
		const QPainterPath &path(int tileSize);

		friend bool operator<(const Feature &f1, const Feature &f2);

	private:
		const Data::Feature *_data;
		const Layer *_layer;
		QPainterPath _path;
	};

	class Layer
	{
	public:
		Layer(const Data::Layer *layer);

		QVector<Feature> &features() {return _features;}
		const QVector<QVariant> &values() const {return _data->values;}
		const KeyHash &keys() const {return _keys;}
		const Data::Layer *data() const {return _data;}

	private:
		const Data::Layer *_data;
		QVector<Feature> _features;
		KeyHash _keys;
	};

	Tile(const Data &data);
	~Tile();

	const QHash<QByteArray, Layer*> &layers() const {return _layers;}

private:
	QHash<QByteArray, Layer*> _layers;
};

inline bool operator<(const Tile::Feature &f1, const Tile::Feature &f2)
  {return f1._data->id < f2._data->id;}

}

#endif // MVT_TILE_H
