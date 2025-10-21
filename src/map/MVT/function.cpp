#include <cmath>
#include <QJsonObject>
#include <QJsonArray>
#include "color.h"
#include "function.h"

#define f(y0, y1, ratio) (y0 * (1.0 - ratio)) + (y1 * ratio)

using namespace MVT;

static qreal interpolate(const QPointF &p0, const QPointF &p1, qreal base,
  qreal x)
{
	qreal difference = p1.x() - p0.x();
	if (difference < 1e-6)
		return p0.y();

	qreal progress = x - p0.x();
	qreal ratio = (base == 1.0)
	  ? progress / difference
	  : (pow(base, progress) - 1) / (pow(base, difference) - 1);

	return f(p0.y(), p1.y(), ratio);
}

static QColor interpolate(const QPair<qreal, QColor> &p0,
  const QPair<qreal, QColor> &p1, qreal base, qreal x)
{
	qreal difference = p1.first - p0.first;
	if (difference < 1e-6)
		return p0.second;

	qreal progress = x - p0.first;
	qreal ratio = (base == 1.0)
	  ? progress / difference
	  : (pow(base, progress) - 1) / (pow(base, difference) - 1);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	qreal p0h, p0s, p0l, p0a;
	qreal p1h, p1s, p1l, p1a;
#else // QT6
	float p0h, p0s, p0l, p0a;
	float p1h, p1s, p1l, p1a;
#endif // QT6
	p0.second.getHslF(&p0h, &p0s, &p0l, &p0a);
	p1.second.getHslF(&p1h, &p1s, &p1l, &p1a);

	/* Qt returns a hue of -1 for achromatic colors. We convert it to a hue of 1
	   using qAbs() to enable interpolation of grayscale colors */
	return QColor::fromHslF(f(qAbs(p0h), qAbs(p1h), ratio), f(p0s, p1s, ratio),
	  f(p0l, p1l, ratio), f(p0a, p1a, ratio));
}

FunctionF::FunctionF(const QJsonValue &json, qreal dflt)
  : _default(dflt), _base(1.0)
{
	if (json.isDouble())
		_default = json.toDouble();
	else if (json.isObject()) {
		QJsonObject obj(json.toObject());

		if (!(obj.contains("stops") && obj["stops"].isArray()))
			return;

		QJsonArray stops = obj["stops"].toArray();
		for (int i = 0; i < stops.size(); i++) {
			if (!stops.at(i).isArray())
				return;
			QJsonArray stop = stops.at(i).toArray();
			if (stop.size() != 2)
				return;
			_stops.append(QPointF(stop.at(0).toDouble(), stop.at(1).toDouble()));
		}

		if (obj.contains("base") && obj["base"].isDouble())
			_base = obj["base"].toDouble();
	}
}

qreal FunctionF::value(qreal x) const
{
	if (_stops.isEmpty())
		return _default;

	QPointF v0(_stops.first());
	for (int i = 0; i < _stops.size(); i++) {
		if (x < _stops.at(i).x())
			return interpolate(v0, _stops.at(i), _base, x);
		else
			v0 = _stops.at(i);
	}

	return _stops.last().y();
}

FunctionC::FunctionC(const QJsonValue &json, const QColor &dflt)
  : _default(dflt), _base(1.0)
{
	if (json.isString())
		_default = Color::fromJsonString(json.toString());
	else if (json.isObject()) {
		QJsonObject obj(json.toObject());

		if (!(obj.contains("stops") && obj["stops"].isArray()))
			return;

		QJsonArray stops = obj["stops"].toArray();
		for (int i = 0; i < stops.size(); i++) {
			if (!stops.at(i).isArray())
				return;
			QJsonArray stop = stops.at(i).toArray();
			if (stop.size() != 2)
				return;
			_stops.append(QPair<qreal, QColor>(stop.at(0).toDouble(),
			  Color::fromJsonString(stop.at(1).toString())));
		}

		if (obj.contains("base") && obj["base"].isDouble())
			_base = obj["base"].toDouble();
	}
}

QColor FunctionC::value(qreal x) const
{
	if (_stops.isEmpty())
		return _default;

	QPair<qreal, QColor> v0(_stops.first());
	for (int i = 0; i < _stops.size(); i++) {
		if (x < _stops.at(i).first)
			return interpolate(v0, _stops.at(i), _base, x);
		else
			v0 = _stops.at(i);
	}

	return _stops.last().second;
}

FunctionB::FunctionB(const QJsonValue &json, bool dflt) : _default(dflt)
{
	if (json.isBool())
		_default = json.toBool();
	else if (json.isObject()) {
		QJsonObject obj(json.toObject());

		if (!(obj.contains("stops") && obj["stops"].isArray()))
			return;

		QJsonArray stops = obj["stops"].toArray();
		for (int i = 0; i < stops.size(); i++) {
			if (!stops.at(i).isArray())
				return;
			QJsonArray stop = stops.at(i).toArray();
			if (stop.size() != 2)
				return;
			_stops.append(QPair<qreal, bool>(stop.at(0).toDouble(),
			  stop.at(1).toBool()));
		}
	}
}

bool FunctionB::value(qreal x) const
{
	if (_stops.isEmpty())
		return _default;

	QPair<qreal, bool> v0(_stops.first());
	for (int i = 0; i < _stops.size(); i++) {
		if (x < _stops.at(i).first)
			return v0.second;
		else
			v0 = _stops.at(i);
	}

	return _stops.last().second;
}

FunctionS::FunctionS(const QJsonValue &json, const QString &dflt)
  : _default(dflt)
{
	if (json.isString())
		_default = json.toString();
	else if (json.isObject()) {
		QJsonObject obj(json.toObject());

		if (!(obj.contains("stops") && obj["stops"].isArray()))
			return;

		QJsonArray stops = obj["stops"].toArray();
		for (int i = 0; i < stops.size(); i++) {
			if (!stops.at(i).isArray())
				return;
			QJsonArray stop = stops.at(i).toArray();
			if (stop.size() != 2)
				return;
			_stops.append(QPair<qreal, QString>(stop.at(0).toDouble(),
			  stop.at(1).toString()));
		}
	}
}

const QString FunctionS::value(qreal x) const
{
	if (_stops.isEmpty())
		return _default;

	QPair<qreal, QString> v0(_stops.first());
	for (int i = 0; i < _stops.size(); i++) {
		if (x < _stops.at(i).first)
			return v0.second;
		else
			v0 = _stops.at(i);
	}

	return _stops.last().second;
}
