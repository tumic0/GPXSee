#ifndef GRAPH_H
#define GRAPH_H

#include <QGraphicsView>
#include <QVector>
#include <QList>
#include <QPointF>
#include "axisitem.h"
#include "colorshop.h"


class SliderItem;
class InfoItem;

class Graph : public QGraphicsView
{
	Q_OBJECT

public:
	Graph(QWidget *parent = 0);
	~Graph();

	void loadData(const QVector<QPointF> &data);
	void setXLabel(const QString &label);
	void setYLabel(const QString &label);
	void setXUnits(const QString &units);
	void setYUnits(const QString &units);
	void setXScale(qreal scale) {_xScale = scale;}
	void setYScale(qreal scale) {_yScale = scale;}

	void plot(QPainter *painter, const QRectF &target);
	void clear();

	qreal sliderPosition() const;
	void setSliderPosition(qreal pos);

	void addInfo(const QString &key, const QString &value);

signals:
	void sliderPositionChanged(qreal);

protected:
	void resizeEvent(QResizeEvent *);

	qreal _xScale, _yScale;
	QString _xUnits, _yUnits;
	QString _xLabel, _yLabel;

private slots:
	void emitSliderPositionChanged(const QPointF &pos);

private:
	void createXLabel();
	void createYLabel();
	void updateBounds(const QPointF &point);
	void resize(const QSizeF &size);


	QGraphicsScene *_scene;
	AxisItem *_xAxis, *_yAxis;
	SliderItem *_slider;
	InfoItem *_info;
	QList<QGraphicsPathItem*> _graphs;

	qreal _xMin, _xMax, _yMin, _yMax;

	ColorShop _colorShop;
};

#endif // GRAPH_H
