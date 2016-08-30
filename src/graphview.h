#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVector>
#include <QList>
#include <QSet>
#include <QPointF>
#include "palette.h"

class AxisItem;
class SliderItem;
class SliderInfoItem;
class InfoItem;
class GraphItem;

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

	void loadData(const QVector<QPointF> &data, int id = 0);
	int count() const {return _graphs.count();}
	void redraw();
	void clear();

	void showGraph(bool show, int id = 0);

	const QString &xLabel() const {return _xLabel;}
	const QString &yLabel() const {return _yLabel;}
	const QString &xUnits() const {return _xUnits;}
	const QString &yUnits() const {return _yUnits;}
	qreal xScale() const {return _xScale;}
	qreal yScale() const {return _yScale;}
	qreal yOffset() const {return _yOffset;}

	void setXLabel(const QString &label);
	void setYLabel(const QString &label);
	void setXUnits(const QString &units);
	void setYUnits(const QString &units);
	void setXScale(qreal scale) {_xScale = scale;}
	void setYScale(qreal scale) {_yScale = scale;}
	void setYOffset(qreal offset) {_yOffset = offset;}

	void setSliderPrecision(int precision) {_precision = precision;}
	void setMinYRange(qreal range) {_minYRange = range;}

	qreal sliderPosition() const {return _sliderPos;}
	void setSliderPosition(qreal pos);

	void plot(QPainter *painter, const QRectF &target);

signals:
	void sliderPositionChanged(qreal);

protected:
	const QRectF &bounds() const {return _bounds;}
	void redraw(const QSizeF &size);
	void addInfo(const QString &key, const QString &value);
	void clearInfo();
	void skipColor() {_palette.color();}
	void resizeEvent(QResizeEvent *);

private slots:
	void emitSliderPositionChanged(const QPointF &pos);
	void newSliderPosition(const QPointF &pos);

private:
	void createXLabel();
	void createYLabel();
	void updateSliderPosition();
	void updateSliderInfo();
	void updateBounds(const QPainterPath &path);
	QRectF graphsBoundingRect() const;
	void removeItem(QGraphicsItem *item);
	void addItem(QGraphicsItem *item);

	qreal _xScale, _yScale;
	qreal _yOffset;
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

	QList<GraphItem*> _graphs;
	QList<GraphItem*> _visible;
	QSet<int> _hide;
	QRectF _bounds;
	Palette _palette;
};

#endif // GRAPHVIEW_H
