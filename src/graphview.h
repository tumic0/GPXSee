#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QVector>
#include <QList>
#include <QSet>
#include <QPointF>
#include "palette.h"
#include "units.h"
#include "graph.h"


class AxisItem;
class SliderItem;
class SliderInfoItem;
class InfoItem;
class GraphItem;
class PathItem;
class GridItem;

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

	void loadGraph(const Graph &graph, PathItem *path, int id = 0);
	int count() const {return _graphs.count();}
	void redraw();
	void clear();

	void showGraph(bool show, int id = 0);
	void setGraphType(GraphType type);
	void setUnits(Units units);
	void showGrid(bool show);

	const QString &yLabel() const {return _yLabel;}
	const QString &yUnits() const {return _yUnits;}
	qreal yScale() const {return _yScale;}
	qreal yOffset() const {return _yOffset;}
	void setYLabel(const QString &label);
	void setYUnits(const QString &units);
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
	QRectF bounds() const;
	void redraw(const QSizeF &size);
	void addInfo(const QString &key, const QString &value);
	void clearInfo();
	void skipColor() {_palette.color();}
	void resizeEvent(QResizeEvent *);

private slots:
	void emitSliderPositionChanged(const QPointF &pos);
	void newSliderPosition(const QPointF &pos);

private:
	void setXUnits();
	void createXLabel();
	void createYLabel();
	void updateSliderPosition();
	void updateSliderInfo();
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
	GridItem *_grid;

	QList<GraphItem*> _graphs;
	QList<GraphItem*> _visible;
	QSet<int> _hide;
	QRectF _bounds;
	Palette _palette;

	Units _units;
	GraphType _graphType;
};

#endif // GRAPHVIEW_H
