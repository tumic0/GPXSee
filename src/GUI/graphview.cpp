#include <QSet>
#include <QGraphicsScene>
#include <QEvent>
#include <QMouseEvent>
#include <QGestureEvent>
#include <QScrollBar>
#include <QGraphicsSimpleTextItem>
#include <QPalette>
#include <QLocale>
#include <QOpenGLWidget>
#include "data/graph.h"
#include "axisitem.h"
#include "axislabelitem.h"
#include "slideritem.h"
#include "sliderinfoitem.h"
#include "infoitem.h"
#include "griditem.h"
#include "graphitem.h"
#include "format.h"
#include "graphicsscene.h"
#include "graphview.h"

#define MARGIN 10.0

#define IW(item) ((item)->boundingRect().width())
#define IH(item) ((item)->boundingRect().height())

static inline QPoint POS(QMouseEvent *e)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return e->pos();
#else // QT 6
	return e->position().toPoint();
#endif // QT 6
}

GraphView::GraphView(QWidget *parent)
	: QGraphicsView(parent)
{
	const QPalette &p = palette();

	_scene = new GraphicsScene(this);
	setScene(_scene);

	setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	setRenderHint(QPainter::Antialiasing, true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setBackgroundBrush(QBrush(p.brush(QPalette::Base)));
	viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
	grabGesture(Qt::PinchGesture);

	_xAxis = new AxisItem(AxisItem::X);
	_xAxis->setZValue(1.0);
	_yAxis = new AxisItem(AxisItem::Y);
	_yAxis->setZValue(1.0);
	_xAxisLabel = new AxisLabelItem(AxisLabelItem::X);
	_xAxisLabel->setZValue(1.0);
	_yAxisLabel = new AxisLabelItem(AxisLabelItem::Y);
	_yAxisLabel->setZValue(1.0);
	_slider = new SliderItem();
	_slider->setZValue(4.0);
	_sliderInfo = new SliderInfoItem(_slider);
	_sliderInfo->setZValue(4.0);
	_info = new InfoItem();
	_grid = new GridItem();
	_message = new QGraphicsSimpleTextItem(tr("Data not available"));
	_message->setBrush(p.brush(QPalette::Disabled, QPalette::WindowText));

	connect(_slider, &SliderItem::positionChanged, this,
	  &GraphView::emitSliderPositionChanged);

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

	_zoom = 1.0;

	_angleDelta = 0;
	_dragStart = 0;
}

GraphView::~GraphView()
{
	delete _xAxis;
	delete _yAxis;
	delete _xAxisLabel;
	delete _yAxisLabel;
	delete _slider;
	delete _info;
	delete _grid;
	delete _message;
}

void GraphView::setYLabel(const QString &label)
{
	_yLabel = label;
	_yAxisLabel->setLabel(_yLabel, _yUnits);
}

void GraphView::setYUnits(const QString &units)
{
	_yUnits = units;
	_yAxisLabel->setLabel(_yLabel, _yUnits);
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

	_xAxisLabel->setLabel(_xLabel, _xUnits);
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
	_zoom = 1.0;

	for (int i = 0; i < _graphs.count(); i++) {
		GraphItem *gi = _graphs.at(i);
		gi->setGraphType(type);
		if (gi->bounds().isNull())
			removeItem(gi);
		else
			addItem(gi);
		_bounds |= gi->bounds();
	}

	if (type == Distance)
		_xLabel = tr("Distance");
	else
		_xLabel = tr("Time");
	setXUnits();

	if (singleGraph())
		_sliderPos = (type == Distance)
		  ? _graphs.first()->distanceAtTime(_sliderPos)
		  : _graphs.first()->timeAtDistance(_sliderPos);
	else
		_sliderPos = 0;

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

void GraphView::addGraph(GraphItem *graph)
{
	_graphs.append(graph);
	if (!graph->bounds().isNull())
		_scene->addItem(graph);
	_bounds |= graph->bounds();

	setXUnits();
}

void GraphView::removeGraph(GraphItem *graph)
{
	_graphs.removeOne(graph);
	_scene->removeItem(graph);

	_bounds = QRectF();
	for (int i = 0; i < _graphs.count(); i++)
		_bounds |= _graphs.at(i)->bounds();

	setXUnits();
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

	if (_bounds.isNull()) {
		removeItem(_xAxis);
		removeItem(_yAxis);
		removeItem(_xAxisLabel);
		removeItem(_yAxisLabel);
		removeItem(_slider);
		removeItem(_info);
		removeItem(_grid);
		if (_graphs.isEmpty())
			removeItem(_message);
		else
			addItem(_message);
		_scene->setSceneRect(_scene->itemsBoundingRect());
		return;
	}

	removeItem(_message);
	addItem(_xAxis);
	addItem(_yAxis);
	addItem(_xAxisLabel);
	addItem(_yAxisLabel);
	addItem(_slider);
	addItem(_info);
	addItem(_grid);

	rx = RangeF(bounds().left() * _xScale, bounds().right() * _xScale);
	ry = RangeF(bounds().top() * _yScale + _yOffset, bounds().bottom() * _yScale
	  + _yOffset);
	if (ry.size() < _minYRange * _yScale)
		ry.resize(_minYRange * _yScale);

	_xAxis->setZoom(_zoom);
	_xAxis->setRange(rx);
	_xAxis->setZoom(_zoom);
	_yAxis->setRange(ry);
	mx = _xAxis->margin();
	my = _yAxis->margin();

	r = _bounds;
	if (r.height() < _minYRange)
		r.adjust(0, -(_minYRange/2 - r.height()/2), 0,
		  _minYRange/2 - r.height()/2);

	sx = (size.width() - (my.width() + mx.width()) - IW(_yAxisLabel))
	  / r.width();
	sy = (size.height() - (mx.height() + my.height())
	  - IH(_info) - IH(_xAxisLabel)) / r.height();
	sx *= _zoom;

	for (int i = 0; i < _graphs.size(); i++)
		_graphs.at(i)->setScale(sx, sy);

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

	_info->setPos(QPointF(r.width()/2 - IW(_info)/2 - (IW(_yAxisLabel)
	  + IW(_yAxis))/2 + r.left(), r.top() - IH(_info) - my.height()));
	_xAxisLabel->setPos(QPointF(r.width()/2 - IW(_xAxisLabel)/2 + r.left(),
	  r.bottom() + mx.height()));
	_yAxisLabel->setPos(QPointF(r.left() - my.width() - IW(_yAxisLabel),
	  r.bottom() - (r.height()/2 + IH(_yAxisLabel)/2)));

	_scene->setSceneRect(_scene->itemsBoundingRect());
}

void GraphView::resizeEvent(QResizeEvent *e)
{
	redraw(e->size() - QSizeF(MARGIN, MARGIN));

	QGraphicsView::resizeEvent(e);
}

void GraphView::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
		newSliderPosition(mapToScene(POS(e)));
	else if (e->button() == Qt::RightButton)
		_dragStart = POS(e).x();

	QGraphicsView::mousePressEvent(e);
}

void GraphView::mouseMoveEvent(QMouseEvent *e)
{
	if (e->buttons() & Qt::RightButton) {
		QScrollBar *sb = horizontalScrollBar();
		int x = POS(e).x();
		sb->setSliderPosition(sb->sliderPosition() - (x - _dragStart));
		_dragStart = x;
	}

	QGraphicsView::mouseMoveEvent(e);
}

void GraphView::wheelEvent(QWheelEvent *e)
{
	_angleDelta += e->angleDelta().y();
	if (qAbs(_angleDelta) < (15 * 8))
		return;
	_angleDelta = _angleDelta % (15 * 8);

	QPointF pos = mapToScene(e->position().toPoint());
	QRectF gr(_grid->boundingRect());
	QPointF r(pos.x() / gr.width(), pos.y() / gr.height());

	_zoom = (e->angleDelta().y() > 0) ? _zoom * 1.25 : qMax(_zoom / 1.25, 1.0);
	redraw();

	QRectF ngr(_grid->boundingRect());
	QPointF npos(mapFromScene(QPointF(r.x() * ngr.width(),
	  r.y() * ngr.height())));
	QScrollBar *sb = horizontalScrollBar();
	sb->setSliderPosition(sb->sliderPosition() + npos.x()
	  - e->position().toPoint().x());

	QGraphicsView::wheelEvent(e);
}

void GraphView::pinchGesture(QPinchGesture *gesture)
{
	QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();

	if (changeFlags & QPinchGesture::ScaleFactorChanged) {
		QPointF pos = mapToScene(gesture->centerPoint().toPoint());
		QRectF gr(_grid->boundingRect());
		QPointF r(pos.x() / gr.width(), pos.y() / gr.height());

		_zoom = qMax(_zoom * gesture->scaleFactor(), 1.0);
		redraw();

		QRectF ngr(_grid->boundingRect());
		QPointF npos(mapFromScene(QPointF(r.x() * ngr.width(),
		  r.y() * ngr.height())));
		QScrollBar *sb = horizontalScrollBar();
		sb->setSliderPosition(sb->sliderPosition() + npos.x()
		  - gesture->centerPoint().x());
	}
}

void GraphView::paintEvent(QPaintEvent *e)
{
	QRectF viewRect(mapToScene(rect()).boundingRect());
	_info->setPos(QPointF(viewRect.left() + (viewRect.width() - IW(_info))/2.0,
	  _info->pos().y()));
	_xAxisLabel->setPos(QPointF(viewRect.left() + (viewRect.width()
	  - IW(_xAxisLabel))/2.0, _xAxisLabel->pos().y()));

	QGraphicsView::paintEvent(e);
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
	_graphs.clear();

	_slider->clear();
	_info->clear();

	_palette.reset();

	_bounds = QRectF();
	_sliderPos = 0;
	_zoom = 1.0;

	removeItem(_xAxis);
	removeItem(_yAxis);
	removeItem(_xAxisLabel);
	removeItem(_yAxisLabel);
	removeItem(_slider);
	removeItem(_info);
	removeItem(_grid);
	removeItem(_message);

	_scene->setSceneRect(0, 0, 0, 0);
}

void GraphView::updateSliderPosition()
{
	if (_sliderPos <= bounds().right() && _sliderPos >= bounds().left()) {
		_slider->setPos((_sliderPos / bounds().width())
		  * _slider->area().width(), _slider->area().bottom());
		_slider->setVisible(true);
		updateSliderInfo();
	} else {
		_slider->setPos(_slider->area().left(), _slider->area().bottom());
		_slider->setVisible(false);
	}
}

bool GraphView::singleGraph() const
{
	return (_graphs.count() == 1
	  || (_graphs.count() == 2 && _graphs.first()->secondaryGraph()));
}

void GraphView::updateSliderInfo()
{
	QLocale l(QLocale::system());
	qreal r = 0, y = 0;
	GraphItem *cardinal = singleGraph() ? _graphs.first() : 0;

	if (cardinal) {
		QRectF br(_bounds);
		if (br.height() < _minYRange)
			br.adjust(0, -(_minYRange/2 - br.height()/2), 0,
			  _minYRange/2 - br.height()/2);

		y = -cardinal->yAtX(_sliderPos);
		r = (y - br.bottom()) / br.height();
	}

	qreal pos = (_sliderPos / bounds().width()) * _slider->area().width();
	SliderInfoItem::Side s = (pos + _sliderInfo->boundingRect().width()
	  > _slider->area().right()) ? SliderInfoItem::Left : SliderInfoItem::Right;

	_sliderInfo->setSide(s);
	_sliderInfo->setPos(QPointF(0, _slider->boundingRect().height() * r));
	QString xText(_graphType == Time ? Format::timeSpan(_sliderPos,
	  bounds().width() > 3600) : l.toString(_sliderPos * _xScale, 'f', 1)
	  + UNIT_SPACE + _xUnits);
	QString yText((!cardinal) ? QString() : l.toString(-y * _yScale + _yOffset,
	  'f', _precision) + UNIT_SPACE + _yUnits);
	if (cardinal && cardinal->secondaryGraph()) {
		qreal delta = y + cardinal->secondaryGraph()->yAtX(_sliderPos);
		yText += QString(" ") + QChar(0x0394) + l.toString(-delta * _yScale
		  + _yOffset, 'f', _precision) + UNIT_SPACE + _yUnits;
	}
	_sliderInfo->setText(xText, yText);
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
	if (_graphs.isEmpty())
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

	QSet<GraphItem*> secondary;
	for (int i = 0; i < _graphs.count(); i++) {
		GraphItem *g = _graphs[i];
		if (g->secondaryGraph())
			secondary.insert(g->secondaryGraph());
	}

	for (int i = 0; i < _graphs.count(); i++) {
		GraphItem *g = _graphs[i];
		if (secondary.contains(g))
			continue;

		QColor color(_palette.nextColor());
		g->setColor(color);
		if (g->secondaryGraph())
			g->secondaryGraph()->setColor(color);
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
		setViewport(new QOpenGLWidget);
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

void GraphView::changeEvent(QEvent *e)
{
	if (e->type() == QEvent::PaletteChange) {
		const QPalette &p = palette();
		_message->setBrush(p.brush(QPalette::Disabled, QPalette::WindowText));
		setBackgroundBrush(QBrush(p.brush(QPalette::Base)));
	}

	QGraphicsView::changeEvent(e);
}

bool GraphView::event(QEvent *event)
{
	if (event->type() == QEvent::Gesture)
		return gestureEvent(static_cast<QGestureEvent*>(event));

	return QGraphicsView::event(event);
}

bool GraphView::gestureEvent(QGestureEvent *event)
{
	if (QGesture *pinch = event->gesture(Qt::PinchGesture))
		pinchGesture(static_cast<QPinchGesture *>(pinch));

	return true;
}
