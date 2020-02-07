#ifndef IMG_H
#define IMG_H

#include <QFile>
#include <QByteArray>
#include <QCache>
#include <QDebug>
#include "common/rtree.h"
#include "common/rectc.h"
#include "style.h"
#include "label.h"

class VectorTile;
class SubFile;
class SubDiv;

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
		RectC boundingRect;

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

	struct Polys {
		Polys() {}
		Polys(const QList<Poly> &polygons, const QList<Poly> &lines)
			: polygons(polygons), lines(lines) {}

		QList<Poly> polygons;
		QList<Poly> lines;
	};

	IMG(const QString &fileName);
	~IMG();

	void load();
	void clear();

	QString fileName() const {return _file.fileName();}
	const QString &name() const {return _name;}
	const RectC &bounds() const {return _bounds;}

	void polys(const RectC &rect, int bits, QList<Poly> *polygons,
	  QList<Poly> *lines);
	void points(const RectC &rect, int bits, QList<Point> *points);
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

	QFile _file;
	quint8 _key;
	int _blockSize;

	QString _name;
	RectC _bounds;
	TileTree _tileTree;
	SubFile *_typ;
	Style *_style;

	QCache<const SubDiv*, Polys> _polyCache;
	QCache<const SubDiv*, QList<Point> > _pointCache;

	bool _valid;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const IMG::Point &point);
QDebug operator<<(QDebug dbg, const IMG::Poly &poly);
#endif // QT_NO_DEBUG

#endif // IMG_H
