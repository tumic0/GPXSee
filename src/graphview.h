#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVector>
#include <QList>
#include <QPointF>
#include "palette.h"


class AxisItem;
class SliderItem;
class SliderInfoItem;
class InfoItem;

class Scene : public QGraphicsScene
{
	Q_OBJECT

public:
	Scene(QObject *parent = 0) : QGraphicsScene(parent) {}
	void mousePressEvent(QGraphicsSceneMouseEvent *e);

signals:
	void mouseClicked(const QPointF &pos);
};

class GraphView : public QGraphicsView
{
	Q_OBJECT

public:
	GraphView(QWidget *parent = 0);
	~GraphView();

	void loadData(const QVector<QPointF> &data);

	void redraw();
	void clear();

	int count() const {return _graphs.count();}

	const QString &xLabel() const {return _xLabel;}
	const QString &yLabel() const {return _yLabel;}
	const QString &xUnits() const {return _xUnits;}
	const QString &yUnits() const {return _yUnits;}
	qreal xScale() const {return _xScale;}
	qreal yScale() const {return _yScale;}

	void setXLabel(const QString &label);
	void setYLabel(const QString &label);
	void setXUnits(const QString &units);
	void setYUnits(const QString &units);
	void setXScale(qreal scale);
	void setYScale(qreal scale);

	void setSliderPrecision(int precision) {_precision = precision;}
	void setMinYRange(qreal range) {_minYRange = range;}

	qreal sliderPosition() const {return _sliderPos;}
	void setSliderPosition(qreal pos);

	void plot(QPainter *painter, const QRectF &target);

signals:
	void sliderPositionChanged(qreal);

protected:
	const QRectF &bounds() const {return _bounds;}
	void resizeEvent(QResizeEvent *);
	void redraw(const QSizeF &size);
	void addInfo(const QString &key, const QString &value);
	void clearInfo();
	void skipColor() {_palette.color();}

private slots:
	void emitSliderPositionChanged(const QPointF &pos);
	void newSliderPosition(const QPointF &pos);

private:
	void createXLabel();
	void createYLabel();
	void updateBounds(const QPointF &point);
	void updateSliderInfo();

	qreal _xScale, _yScale;
	QString _xUnits, _yUnits;
	QString _xLabel, _yLabel;
	int _precision;
	qreal _minYRange;
	qreal _sliderPos;

	Scene *_scene;

	AxisItem *_xAxis, *_yAxis;
	SliderItem *_slider;
	SliderInfoItem *_sliderInfo;
	InfoItem *_info;

	QList<QGraphicsPathItem*> _graphs;
	QRectF _bounds;
	Palette _palette;
};

#endif // GRAPHVIEW_H
