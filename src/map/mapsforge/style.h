#ifndef MAPSFORGE_STYLE_H
#define MAPSFORGE_STYLE_H

#include <QString>
#include <QList>
#include <QPen>
#include <QFont>
#include "mapdata.h"

class QXmlStreamReader;

namespace Mapsforge {

class Style
{
public:
	class Rule {
	public:
		Rule() : _type(AnyType), _closed(AnyClosed), _zooms(0, 127) {}

		bool match(const QVector<MapData::Tag> &tags) const;
		bool match(bool closed, const QVector<MapData::Tag> &tags) const;
		bool match(int zoom, bool closed,
		  const QVector<MapData::Tag> &tags) const;
		bool match(int zoom, const QVector<MapData::Tag> &tags) const;

	private:
		enum Type {
			AnyType = 0,
			NodeType = 1,
			WayType = 2,
			InvalidType = 3
		};

		enum Closed {
			AnyClosed = 0,
			YesClosed = 1,
			NoClosed = 2,
			InvalidClosed = 3
		};

		class Filter {
		public:
			Filter() : _neg(false) {}
			Filter(const MapData &data, const QList<QByteArray> &keys,
			  const QList<QByteArray> &vals);

			bool match(const QVector<MapData::Tag> &tags) const
			{
				if (_neg) {
					if (!keyMatches(tags))
						return true;
					return valueMatches(tags);
				} else
					return (keyMatches(tags) && valueMatches(tags));
			}

			bool isTautology() const
			{
				return (!_neg && _keys.contains(0u) && _vals.contains(QByteArray()));
			}

		private:
			bool keyMatches(const QVector<MapData::Tag> &tags) const
			{
				for (int i = 0; i < _keys.size(); i++) {
					for (int j = 0; j < tags.size(); j++) {
						unsigned key = _keys.at(i);
						if (!key || key == tags.at(j).key)
							return true;
					}
				}

				return false;
			}

			bool valueMatches(const QVector<MapData::Tag> &tags) const
			{
				for (int i = 0; i < _vals.size(); i++) {
					for (int j = 0; j < tags.size(); j++) {
						const QByteArray &ba = _vals.at(i);
						if (!ba.size() || ba == tags.at(j).value)
							return true;
					}
				}

				return false;
			}

			QList<unsigned> _keys;
			QList<QByteArray> _vals;
			bool _neg;
		};

		void setType(Type type)
		{
			_type = static_cast<Type>(static_cast<int>(type)
			  | static_cast<int>(_type));
		}
		void setMinZoom(int zoom) {_zooms.setMin(qMax(zoom, _zooms.min()));}
		void setMaxZoom(int zoom) {_zooms.setMax(qMin(zoom, _zooms.max()));}
		void setClosed(Closed closed)
		{
			_closed = static_cast<Closed>(static_cast<int>(closed)
			  | static_cast<int>(_closed));
		}
		void addFilter(const Filter &filter)
		{
			if (!filter.isTautology())
				_filters.append(filter);
		}
		bool match(int zoom, Type type, Closed closed,
		  const QVector<MapData::Tag> &tags) const;

		friend class Style;

		Type _type;
		Closed _closed;
		Range _zooms;
		QVector<Filter> _filters;
	};

	class Render
	{
	public:
		Render(const Rule &rule) : _rule(rule) {}

		const Rule &rule() const {return _rule;}

	private:
		Rule _rule;
	};

	class PathRender : public Render
	{
	public:
		PathRender(const Rule &rule, int zOrder) : Render(rule),
		  _zOrder(zOrder), _strokeWidth(0), _strokeCap(Qt::RoundCap),
		  _strokeJoin(Qt::RoundJoin), _area(false), _curve(false) {}

		int zOrder() const {return _zOrder;}
		QPen pen(int zoom) const;
		const QBrush &brush() const {return _brush;}
		bool area() const {return _area;}
		bool curve() const {return _curve;}

	private:
		friend class Style;

		int _zOrder;
		QColor _strokeColor;
		qreal _strokeWidth;
		QVector<qreal> _strokeDasharray;
		Qt::PenCapStyle _strokeCap;
		Qt::PenJoinStyle _strokeJoin;
		QBrush _brush;
		bool _area, _curve;
	};

	class CircleRender : public Render
	{
	public:
		CircleRender(const Rule &rule, int zOrder) : Render(rule),
		  _zOrder(zOrder), _pen(Qt::NoPen), _brush(Qt::NoBrush),
		  _scale(false) {}

		int zOrder() const {return _zOrder;}
		const QPen &pen() const {return _pen;}
		const QBrush &brush() const {return _brush;}
		qreal radius(int zoom) const;

	private:
		friend class Style;

		int _zOrder;
		QPen _pen;
		QBrush _brush;
		qreal _radius;
		bool _scale;
	};

	class TextRender : public Render
	{
	public:
		TextRender(const Rule &rule)
		  : Render(rule), _priority(0), _fillColor(Qt::black),
		  _strokeColor(Qt::black), _strokeWidth(0) {}

		const QFont &font() const {return _font;}
		const QColor &fillColor() const {return _fillColor;}
		const QColor &strokeColor() const {return _strokeColor;}
		qreal strokeWidth() const {return _strokeWidth;}
		unsigned key() const {return _key;}
		int priority() const {return _priority;}

	private:
		friend class Style;

		int _priority;
		QColor _fillColor, _strokeColor;
		qreal _strokeWidth;
		QFont _font;
		unsigned _key;
	};

	class Symbol : public Render
	{
	public:
		Symbol(const Rule &rule) : Render(rule), _priority(0) {}

		const QImage &img() const {return _img;}
		int priority() const {return _priority;}

	private:
		friend class Style;

		int _priority;
		QImage _img;
	};

	void load(const MapData &data, qreal ratio);
	void clear();

	QList<const PathRender *> paths(int zoom, bool closed,
	  const QVector<MapData::Tag> &tags) const;
	QList<const CircleRender *> circles(int zoom,
	  const QVector<MapData::Tag> &tags) const;
	QList<const TextRender*> pathLabels(int zoom) const;
	QList<const TextRender*> pointLabels(int zoom) const;
	QList<const TextRender*> areaLabels(int zoom) const;
	QList<const Symbol*> pointSymbols(int zoom) const;
	QList<const Symbol*> areaSymbols(int zoom) const;

private:
	QList<PathRender> _paths;
	QList<CircleRender> _circles;
	QList<TextRender> _pathLabels, _pointLabels, _areaLabels;
	QList<Symbol> _symbols;

	bool loadXml(const QString &path, const MapData &data, qreal ratio);
	void rendertheme(QXmlStreamReader &reader, const QString &dir,
	  const MapData &data, qreal ratio);
	void layer(QXmlStreamReader &reader, QSet<QString> &cats);
	void stylemenu(QXmlStreamReader &reader, QSet<QString> &cats);
	void cat(QXmlStreamReader &reader, QSet<QString> &cats);
	void rule(QXmlStreamReader &reader, const QString &dir, const MapData &data,
	  qreal ratio, const QSet<QString> &cats, const Rule &parent);
	void area(QXmlStreamReader &reader, const QString &dir, qreal ratio,
	  const Rule &rule);
	void line(QXmlStreamReader &reader, const Rule &rule);
	void circle(QXmlStreamReader &reader, const Rule &rule);
	void text(QXmlStreamReader &reader, const MapData &data, const Rule &rule,
	  QList<QList<TextRender> *> &lists);
	void symbol(QXmlStreamReader &reader, const QString &dir, qreal ratio,
	  const Rule &rule);
};

}

#endif // MAPSFORGE_STYLE_H
