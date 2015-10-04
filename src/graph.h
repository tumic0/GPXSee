#ifndef GRAPH_H
#define GRAPH_H

#include <QGraphicsView>
#include <QVector>
#include <QList>
#include <QPointF>
#include "axisitem.h"
#include "slideritem.h"
#include "colorshop.h"


class Graph : public QGraphicsView
{
	Q_OBJECT

public:
	Graph(QWidget *parent = 0);
	~Graph();

	void loadData(const QVector<QPointF> &data);
	void setXLabel(const QString &label) {_xAxis->setLabel(label);}
	void setYLabel(const QString &label) {_yAxis->setLabel(label);}
	void setXScale(qreal scale) {_xScale = scale;}
	void setYScale(qreal scale) {_yScale = scale;}

	void plot(QPainter *painter, const QRectF &target);
	void clear();

	qreal sliderPosition() const;
	void setSliderPosition(qreal pos);

signals:
	void sliderPositionChanged(qreal);

protected:
	void resizeEvent(QResizeEvent *);

private slots:
	void emitSliderPositionChanged(const QPointF &pos);

private:
	void updateBounds(const QPointF &point);
	void resize(const QSizeF &size);


	QGraphicsScene *_scene;
	AxisItem *_xAxis, *_yAxis;
	SliderItem *_slider;
	qreal _xMin, _xMax, _yMin, _yMax;
	QList<QGraphicsPathItem*> _graphs;
	qreal _xScale, _yScale;
	ColorShop _colorShop;
};

#endif // GRAPH_H
