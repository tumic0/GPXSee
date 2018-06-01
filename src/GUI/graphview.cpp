#include <QGraphicsScene>
#include <QEvent>
#include <QMouseEvent>
#include <QPaintEngine>
#include <QPaintDevice>
#include "data/graph.h"
#include "opengl.h"
#include "config.h"
#include "axisitem.h"
#include "slideritem.h"
#include "sliderinfoitem.h"
#include "infoitem.h"
#include "griditem.h"
#include "graphitem.h"
#include "pathitem.h"
#include "format.h"
#include "graphview.h"


#define MARGIN 10.0

GraphView::GraphView(QWidget *parent)
	: QGraphicsView(parent)
{
	_scene = new QGraphicsScene(this);
	setScene(_scene);

	setBackgroundBrush(QBrush(Qt::white));
	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setRenderHint(QPainter::Antialiasing, true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	_xAxis = new AxisItem(AxisItem::X);
	_xAxis->setZValue(2.0);
	_yAxis = new AxisItem(AxisItem::Y);
	_yAxis->setZValue(2.0);
	_slider = new SliderItem();
	_slider->setZValue(3.0);
	_sliderInfo = new SliderInfoItem(_slider);
	_sliderInfo->setZValue(3.0);
	_info = new InfoItem();
	_grid = new GridItem();

	connect(_slider, SIGNAL(positionChanged(const QPointF&)), this,
	  SLOT(emitSliderPositionChanged(const QPointF&)));

	_width = 1;

	_xScale = 1;
	_yScale = 1;
	_yOffset = 0;

	_precision = 0;
	_minYRange = 0.01;

	_sliderPos = 0;

	_units = Metric;
	_graphType = Distance;
	_xLabel = tr("Distance");
}

GraphView::~GraphView()
{
	delete _xAxis;
	delete _yAxis;
	delete _slider;
	delete _info;
	delete _grid;

	for (int i = 0; i < _graphs.count(); i++)
		delete _graphs[i];
}

void GraphView::createXLabel()
{
	_xAxis->setLabel(QString("%1 [%2]").arg(_xLabel, _xUnits));
}

void GraphView::createYLabel()
{
	_yAxis->setLabel(QString("%1 [%2]").arg(_yLabel, _yUnits));
}

void GraphView::setYLabel(const QString &label)
{
	_yLabel = label;
	createYLabel();
}

void GraphView::setYUnits(const QString &units)
{
	_yUnits = units;
	createYLabel();
}

void GraphView::setXUnits()
{
	if (_graphType == Distance) {
		if (_units == Imperial) {
			if (bounds().width() < MIINM) {
				_xUnits = tr("ft");
				_xScale = M2FT;
			} else {
				_xUnits = tr("mi");
				_xScale = M2MI;
			}
		} else if (_units == Nautical) {
			if (bounds().width() < NMIINM) {
				_xUnits = tr("ft");
				_xScale = M2FT;
			} else {
				_xUnits = tr("nmi");
				_xScale = M2NMI;
			}
		} else {
			if (bounds().width() < KMINM) {
				_xUnits = tr("m");
				_xScale = 1;
			} else {
				_xUnits = tr("km");
				_xScale = M2KM;
			}
		}
	} else {
		    if (bounds().width() < MININS) {
				_xUnits = tr("s");
				_xScale = 1;
			} else if (bounds().width() < HINS) {
				_xUnits = tr("min");
				_xScale = MIN2S;
			} else {
				_xUnits = tr("h");
				_xScale = H2S;
			}
	}

	createXLabel();
}

void GraphView::setUnits(Units units)
{
	_units = units;

	for (int i = 0; i < _graphs.count(); i++)
		_graphs.at(i)->setUnits(units);

	setXUnits();

	redraw();
}

void GraphView::setGraphType(GraphType type)
{
	_graphType = type;
	_bounds = QRectF();

	for (int i = 0; i < _graphs.count(); i++) {
		_graphs.at(i)->setGraphType(type);
		if (_graphs.at(i)->scene() == _scene)
			_bounds |= _graphs.at(i)->bounds();
	}

	if (type == Distance)
		_xLabel = tr("Distance");
	else
		_xLabel = tr("Time");
	setXUnits();

	redraw();
}

void GraphView::showGrid(bool show)
{
	_grid->setVisible(show);
}

void GraphView::showSliderInfo(bool show)
{
	_sliderInfo->setVisible(show);
}

void GraphView::addGraph(GraphItem *graph, int id)
{
	QColor color(_palette.nextColor());
	color.setAlpha(255);

	graph->setUnits(_units);
	graph->setId(id);
	graph->setColor(color);
	graph->setWidth(_width);

	connect(this, SIGNAL(sliderPositionChanged(qreal)), graph,
	  SLOT(emitSliderPositionChanged(qreal)));

	_graphs.append(graph);

	if (!_hide.contains(id)) {
		_visible.append(graph);
		_scene->addItem(graph);
		_bounds |= graph->bounds();
		setXUnits();
	}
}

void GraphView::removeItem(QGraphicsItem *item)
{
	if (item->scene() == _scene)
		_scene->removeItem(item);
}

void GraphView::addItem(QGraphicsItem *item)
{
	if (item->scene() != _scene)
		_scene->addItem(item);
}

void GraphView::showGraph(bool show, int id)
{
	if (show)
		_hide.remove(id);
	else
		_hide.insert(id);

	_visible.clear();
	_bounds = QRectF();
	for (int i = 0; i < _graphs.count(); i++) {
		GraphItem* gi = _graphs.at(i);
		if (_hide.contains(gi->id()))
			removeItem(gi);
		else {
			addItem(gi);
			_visible.append(gi);
			_bounds |= gi->bounds();
		}
	}
}

QRectF GraphView::bounds() const
{
	QRectF br(_bounds);
	br.moveTopLeft(QPointF(br.left(), -br.top() - br.height()));
	return br;
}

void GraphView::redraw()
{
	redraw(viewport()->size() - QSizeF(MARGIN, MARGIN));
}

void GraphView::redraw(const QSizeF &size)
{
	QRectF r;
	QSizeF mx, my;
	RangeF rx, ry;
	qreal sx, sy;


	if (_visible.isEmpty() || _bounds.isNull()) {
		removeItem(_xAxis);
		removeItem(_yAxis);
		removeItem(_slider);
		removeItem(_info);
		removeItem(_grid);
		_scene->setSceneRect(QRectF());
		return;
	}

	addItem(_xAxis);
	addItem(_yAxis);
	addItem(_slider);
	addItem(_info);
	addItem(_grid);

	rx = RangeF(bounds().left() * _xScale, bounds().right() * _xScale);
	ry = RangeF(bounds().top() * _yScale + _yOffset, bounds().bottom() * _yScale
	  + _yOffset);
	if (ry.size() < _minYRange)
		ry.resize(_minYRange);

	_xAxis->setRange(rx);
	_yAxis->setRange(ry);
	mx = _xAxis->margin();
	my = _yAxis->margin();

	r = _bounds;
	if (r.height() < _minYRange)
		r.adjust(0, -(_minYRange/2 - r.height()/2), 0,
		  _minYRange/2 - r.height()/2);

	sx = (size.width() - (my.width() + mx.width())) / r.width();
	sy = (size.height() - (mx.height() + my.height())
	  - _info->boundingRect().height()) / r.height();

	for (int i = 0; i < _visible.size(); i++)
		_visible.at(i)->setScale(sx, sy);

	QPointF p(r.left() * sx, r.top() * sy);
	QSizeF s(r.width() * sx, r.height() * sy);
	r = QRectF(p, s);
	if (r.height() < _minYRange * sy)
		r.adjust(0, -(_minYRange/2 * sy - r.height()/2), 0,
		  (_minYRange/2) * sy - r.height()/2);
	r = r.toRect();

	_xAxis->setSize(r.width());
	_yAxis->setSize(r.height());
	_xAxis->setPos(r.bottomLeft());
	_yAxis->setPos(r.bottomLeft());

	_grid->setSize(r.size());
	_grid->setTicks(_xAxis->ticks(), _yAxis->ticks());
	_grid->setPos(r.bottomLeft());

	_slider->setArea(r);
	updateSliderPosition();

	r |= _xAxis->sceneBoundingRect();
	r |= _yAxis->sceneBoundingRect();
	_info->setPos(r.topLeft() + QPointF(r.width()/2
	  - _info->boundingRect().width()/2, -_info->boundingRect().height()));

	_scene->setSceneRect(_scene->itemsBoundingRect());
}

void GraphView::resizeEvent(QResizeEvent *)
{
	redraw();
}

void GraphView::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
		newSliderPosition(mapToScene(e->pos()));

	QGraphicsView::mousePressEvent(e);
}

void GraphView::plot(QPainter *painter, const QRectF &target, qreal scale)
{
	QSizeF canvas = QSizeF(target.width() / scale, target.height() / scale);

	setUpdatesEnabled(false);
	redraw(canvas);
	if (_slider->pos().x() == _slider->area().left())
		_slider->hide();
	_scene->render(painter, target);
	_slider->show();
	redraw();
	setUpdatesEnabled(true);
}

void GraphView::clear()
{
	_slider->clear();
	_info->clear();

	for (int i = 0; i < _graphs.count(); i++)
		delete _graphs[i];

	_graphs.clear();
	_visible.clear();
	_palette.reset();

	_bounds = QRectF();
	_sliderPos = 0;

	_scene->setSceneRect(0, 0, 0, 0);
}

void GraphView::updateSliderPosition()
{
	if (bounds().width() <= 0)
		return;

	if (_sliderPos <= bounds().right() && _sliderPos >= bounds().left()) {
		_slider->setPos((_sliderPos / bounds().width())
		  * _slider->area().width(), _slider->area().bottom());
		_slider->setVisible(!_visible.isEmpty());
	} else {
		_slider->setPos(_slider->area().left(), _slider->area().bottom());
		_slider->setVisible(false);
	}

	if (_slider->isVisible())
		updateSliderInfo();
}

void GraphView::updateSliderInfo()
{
	qreal r, y;

	if (_visible.count() > 1) {
		r = 0;
		y = 0;
	} else {
		QRectF br(_visible.first()->bounds());
		if (br.height() < _minYRange)
			br.adjust(0, -(_minYRange/2 - br.height()/2), 0,
			  _minYRange/2 - br.height()/2);

		y = _visible.first()->yAtX(_sliderPos);
		r = (y - br.bottom()) / br.height();
	}

	qreal pos = (_sliderPos / bounds().width()) * _slider->area().width();
	SliderInfoItem::Side s = (pos + _sliderInfo->boundingRect().width()
	  > _slider->area().right()) ? SliderInfoItem::Left : SliderInfoItem::Right;

	_sliderInfo->setSide(s);
	_sliderInfo->setPos(QPointF(0, _slider->boundingRect().height() * r));
	_sliderInfo->setText(_graphType == Time ? Format::timeSpan(_sliderPos,
	  bounds().width() > 3600) : QString::number(_sliderPos * _xScale, 'f', 1)
	  + UNIT_SPACE + _xUnits, (_visible.count() > 1) ? QString()
	  : QString::number(-y * _yScale + _yOffset, 'f', _precision) + UNIT_SPACE
	  + _yUnits);
}

void GraphView::emitSliderPositionChanged(const QPointF &pos)
{
	if (_slider->area().width() <= 0)
		return;

	_sliderPos = (pos.x() / _slider->area().width()) * bounds().width();
	_sliderPos = qMax(_sliderPos, bounds().left());
	_sliderPos = qMin(_sliderPos, bounds().right());
	updateSliderPosition();

	emit sliderPositionChanged(_sliderPos);
}

void GraphView::setSliderPosition(qreal pos)
{
	if (_visible.isEmpty())
		return;

	_sliderPos = pos;
	updateSliderPosition();
}

void GraphView::newSliderPosition(const QPointF &pos)
{
	if (_slider->area().contains(pos))
		_slider->setPos(pos);
}

void GraphView::addInfo(const QString &key, const QString &value)
{
	_info->insert(key, value);
}

void GraphView::clearInfo()
{
	_info->clear();
}

void GraphView::setPalette(const Palette &palette)
{
	_palette = palette;
	_palette.reset();

	for (int i = 0; i < _graphs.count(); i++) {
		QColor color(_palette.nextColor());
		color.setAlpha(255);
		_graphs.at(i)->setColor(color);
	}
}

void GraphView::setGraphWidth(int width)
{
	_width = width;

	for (int i = 0; i < _graphs.count(); i++)
		_graphs.at(i)->setWidth(width);

	redraw();
}

void GraphView::useOpenGL(bool use)
{
	if (use)
		setViewport(new OPENGL_WIDGET);
	else
		setViewport(new QWidget);
}

void GraphView::useAntiAliasing(bool use)
{
	setRenderHint(QPainter::Antialiasing, use);
}

void GraphView::setSliderColor(const QColor &color)
{
	_slider->setColor(color);
	_sliderInfo->setColor(color);
}
