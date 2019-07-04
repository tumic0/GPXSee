#ifndef IMG_H
#define IMG_H

#include <QRect>
#include <QFile>
#include <QByteArray>
#include <QCache>
#include <QDebug>
#include "common/rtree.h"
#include "common/rectc.h"
#include "common/range.h"
#include "style.h"
#include "label.h"

class VectorTile;
class SubFile;

class IMG
{
public:
	struct Poly {
		/* QPointF insted of Coordinates for performance reasons (no need to
		   duplicate all the vectors for drawing). Note, that we do not want to
		   ll2xy() the points in the IMG class as this can not be done in
		   parallel. */
		QVector<QPointF> points;
		Label label;
		quint32 type;

		bool operator<(const Poly &other) const
		  {return type > other.type;}
	};

	struct Point {
		Point() : id(0) {}

		Coordinates coordinates;
		Label label;
		quint32 type;
		bool poi;
		quint64 id;

		bool operator<(const Point &other) const
		  {return id < other.id;}
	};


	IMG(const QString &fileName);
	~IMG();

	void load();
	void clear();

	QString fileName() const {return _file.fileName();}
	const QString &name() const {return _name;}
	const RectC &bounds() const {return _bounds;}

	void objects(const RectC &rect, int bits, QList<Poly> *polygons,
	  QList<Poly> *lines, QList<Point> *points);
	const Style *style() const {return _style;}

	bool isValid() const {return _valid;}
	const QString &errorString() const {return _errorString;}

private:
	friend class SubFile;

	typedef RTree<VectorTile*, double, 2> TileTree;

	int blockSize() const {return _blockSize;}
	bool readBlock(int blockNum, QByteArray &data);
	qint64 read(char *data, qint64 maxSize);
	template<class T> bool readValue(T &val);
	bool init();

	QFile _file;
	quint8 _key;
	int _blockSize;
	QCache<int, QByteArray> _blockCache;

	QString _name;
	RectC _bounds;
	TileTree _tileTree;
	SubFile *_typ;
	Style *_style;

	bool _valid;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const IMG::Point &point);
QDebug operator<<(QDebug dbg, const IMG::Poly &poly);
#endif // QT_NO_DEBUG

#endif // IMG_H
