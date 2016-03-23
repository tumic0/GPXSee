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
	void setXLabel(const QString &label);
	void setYLabel(const QString &label);
	void setXUnits(const QString &units);
	void setYUnits(const QString &units);
	void setXScale(qreal scale);
	void setYScale(qreal scale);
	void setPrecision(int precision) {_precision = precision;}
	void setMinRange(qreal range) {_minRange = range;}

	void plot(QPainter *painter, const QRectF &target);
	void clear();

	qreal sliderPosition() const;
	void setSliderPosition(qreal pos);

	int count() const {return _graphs.count();}

signals:
	void sliderPositionChanged(qreal);

protected:
	void resizeEvent(QResizeEvent *);
	void redraw();
	void addInfo(const QString &key, const QString &value);
	void clearInfo();
	void skipColor() {_palette.color();}

	qreal _xScale, _yScale;
	QString _xUnits, _yUnits;
	QString _xLabel, _yLabel;
	int _precision;
	qreal _minRange;

private slots:
	void emitSliderPositionChanged(const QPointF &pos);
	void newSliderPosition(const QPointF &pos);

private:
	void createXLabel();
	void createYLabel();
	void updateBounds(const QPointF &point);
	void redraw(const QSizeF &size);

	Scene *_scene;

	AxisItem *_xAxis, *_yAxis;
	SliderItem *_slider;
	SliderInfoItem *_sliderInfo;
	InfoItem *_info;

	QList<QGraphicsPathItem*> _graphs;
	qreal _xMin, _xMax, _yMin, _yMax;
	Palette _palette;
};

#endif // GRAPHVIEW_H
