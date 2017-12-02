#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QGraphicsView>
#include <QList>
#include <QSet>
#include "data/graph.h"
#include "palette.h"
#include "units.h"


class AxisItem;
class SliderItem;
class SliderInfoItem;
class InfoItem;
class GraphItem;
class PathItem;
class GridItem;

class GraphView : public QGraphicsView
{
	Q_OBJECT

public:
	GraphView(QWidget *parent = 0);
	~GraphView();

	bool isEmpty() const {return _graphs.isEmpty();}
	void clear();

	void plot(QPainter *painter, const QRectF &target, qreal scale);

	void setPalette(const Palette &palette);
	void setGraphWidth(int width);
	void showGrid(bool show);
	void showSliderInfo(bool show);
	void useOpenGL(bool use);
	void useAntiAliasing(bool use);

	void setSliderPosition(qreal pos);
	void setSliderColor(const QColor &color);

signals:
	void sliderPositionChanged(qreal);

protected:
	void addGraph(GraphItem *graph, PathItem *path, int id = 0);

	void showGraph(bool show, int id = 0);
	void setGraphType(GraphType type);
	void setUnits(Units units);

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

	QRectF bounds() const;
	void redraw();
	void redraw(const QSizeF &size);
	void addInfo(const QString &key, const QString &value);
	void clearInfo();
	void skipColor() {_palette.nextColor();}

	QList<GraphItem*> _graphs;
	GraphType _graphType;

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

	void resizeEvent(QResizeEvent *);
	void mousePressEvent(QMouseEvent *);

	Units _units;
	qreal _xScale, _yScale;
	qreal _yOffset;
	QString _xUnits, _yUnits;
	QString _xLabel, _yLabel;
	int _precision;
	qreal _minYRange;
	qreal _sliderPos;

	QGraphicsScene *_scene;

	AxisItem *_xAxis, *_yAxis;
	SliderItem *_slider;
	SliderInfoItem *_sliderInfo;
	InfoItem *_info;
	GridItem *_grid;

	QList<GraphItem*> _visible;
	QSet<int> _hide;
	QRectF _bounds;
	Palette _palette;
	int _width;
};

#endif // GRAPHVIEW_H
