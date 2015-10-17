#include <float.h>
#include <QGraphicsView>
#include <QGraphicsSceneMouseEvent>
#include <QEvent>
#include <QGraphicsSimpleTextItem>
#include "slideritem.h"
#include "sliderinfoitem.h"
#include "infoitem.h"
#include "config.h"
#include "graph.h"

#include <QDebug>


#define MARGIN 10.0


void Scene::mousePressEvent(QGraphicsSceneMouseEvent *e)
{
	if (e->button() == Qt::LeftButton)
		emit mouseClicked(e->scenePos());

	QGraphicsScene::mousePressEvent(e);
}


Graph::Graph(QWidget *parent)
	: QGraphicsView(parent)
{
	_scene = new Scene(this);
	setScene(_scene);

	_xAxis = new AxisItem(AxisItem::X);
	_yAxis = new AxisItem(AxisItem::Y);

	_slider = new SliderItem();
	_slider->setZValue(2.0);

	connect(_slider, SIGNAL(positionChanged(const QPointF&)), this,
	  SLOT(emitSliderPositionChanged(const QPointF&)));
	connect(_scene, SIGNAL(mouseClicked(const QPointF&)), this,
	  SLOT(newSliderPosition(const QPointF&)));

	_info = new InfoItem();

	_sliderInfo = new SliderInfoItem(_slider);
	_sliderInfo->setZValue(2.0);

	_xMax = -FLT_MAX;
	_xMin = FLT_MAX;
	_yMax = -FLT_MAX;
	_yMin = FLT_MAX;

	_xScale = 1;
	_yScale = 1;

	_precision = 0;
}

Graph::~Graph()
{
	if (_xAxis->scene() != _scene)
		delete _xAxis;
	if (_yAxis->scene() != _scene)
		delete _yAxis;

	if (_slider->scene() != _scene)
		delete _slider;

	if (_info->scene() != _scene)
		delete _info;

	delete _scene;
}

void Graph::updateBounds(const QPointF &point)
{
	if (point.x() < _xMin)
		_xMin = point.x();
	if (point.x() > _xMax)
		_xMax = point.x();
	if (point.y() < _yMin)
		_yMin = point.y();
	if (point.y() > _yMax)
		_yMax = point.y();
}

void Graph::createXLabel()
{
	_xAxis->setLabel(QString("%1 [%2]").arg(_xLabel).arg(_xUnits));
}

void Graph::createYLabel()
{
	_yAxis->setLabel(QString("%1 [%2]").arg(_yLabel).arg(_yUnits));
}

void Graph::setXLabel(const QString &label)
{
	_xLabel = label;
	createXLabel();
}

void Graph::setYLabel(const QString &label)
{
	_yLabel = label;
	createYLabel();
}

void Graph::setXUnits(const QString &units)
{
	_xUnits = units;
	createXLabel();
}

void Graph::setYUnits(const QString &units)
{
	_yUnits = units;
	createYLabel();
}

void Graph::loadData(const QVector<QPointF> &data)
{
	QPainterPath path;
	QGraphicsPathItem *pi;
	QColor color = _colorShop.color();


	if (data.size() < 2)
		return;

	updateBounds(data.at(0));
	path.moveTo(data.at(0).x(), -data.at(0).y());
	for (int i = 1; i < data.size(); i++) {
		path.lineTo(data.at(i).x(), -data.at(i).y());
		updateBounds(data.at(i));
	}

	QBrush brush(color, Qt::SolidPattern);
	QPen pen(brush, 0);

	pi = new QGraphicsPathItem(path);
	pi->setPen(pen);
	_scene->addItem(pi);
	_graphs.append(pi);

	if (_graphs.size() > 1)
		_sliderInfo->hide();

	resize(viewport()->size() - QSizeF(MARGIN, MARGIN));
}

void Graph::resize(const QSizeF &size)
{
	QRectF r;
	QSizeF mx, my;
	QTransform transform;
	qreal xs, ys;


	if (_xAxis->scene() == _scene)
		_scene->removeItem(_xAxis);
	if (_yAxis->scene() == _scene)
		_scene->removeItem(_yAxis);
	if (_slider->scene() == _scene)
		_scene->removeItem(_slider);
	if (_info->scene() == _scene)
		_scene->removeItem(_info);

	for (int i = 0; i < _graphs.size(); i++)
		_graphs.at(i)->resetTransform();

	_xAxis->setRange(QPointF(_xMin * _xScale, _xMax * _xScale));
	_yAxis->setRange(QPointF(_yMin * _yScale, _yMax * _yScale));
	mx = _xAxis->margin();
	my = _yAxis->margin();
	r = _scene->itemsBoundingRect();
	xs = (size.width() - (my.width() + mx.width())) / r.width();
	ys = (size.height() - (mx.height() + my.height())
	  - _info->boundingRect().height()) / r.height();
	transform.scale(xs, ys);

	for (int i = 0; i < _graphs.size(); i++)
		_graphs.at(i)->setTransform(transform);

	r = _scene->itemsBoundingRect();
	_xAxis->setSize(r.width());
	_yAxis->setSize(r.height());
	_xAxis->setPos(r.bottomLeft());
	_yAxis->setPos(r.bottomLeft());
	_scene->addItem(_xAxis);
	_scene->addItem(_yAxis);

	_slider->setArea(r);
	_slider->setPos(r.bottomLeft());
	_scene->addItem(_slider);

	r = _scene->itemsBoundingRect();
	_info->setPos(r.topLeft() + QPointF(r.width()/2
	  - _info->boundingRect().width()/2, -_info->boundingRect().height()));
	_scene->addItem(_info);

	_scene->setSceneRect(_scene->itemsBoundingRect());
}

void Graph::resizeEvent(QResizeEvent *)
{
	if (!_graphs.empty())
		resize(viewport()->size() - QSizeF(MARGIN, MARGIN));
}

void Graph::plot(QPainter *painter, const QRectF &target)
{
	qreal ratio = target.width() / target.height();
	QSizeF orig = _scene->sceneRect().size();
	QSizeF canvas = QSizeF(orig.height() * ratio, orig.height());

	resize(canvas);
	_slider->setVisible(false);
	_scene->render(painter, target, QRectF(), Qt::KeepAspectRatioByExpanding);
	_slider->setVisible(true);
	resize(orig);
}

void Graph::clear()
{
	if (_xAxis->scene() == _scene)
		_scene->removeItem(_xAxis);
	if (_yAxis->scene() == _scene)
		_scene->removeItem(_yAxis);

	if (_slider->scene() == _scene)
		_scene->removeItem(_slider);

	if (_info->scene() == _scene)
		_scene->removeItem(_info);
	_sliderInfo->show();

	_info->clear();
	_scene->clear();
	_graphs.clear();
	_colorShop.reset();

	_xMax = -FLT_MAX;
	_xMin = FLT_MAX;
	_yMax = -FLT_MAX;
	_yMin = FLT_MAX;

	_scene->setSceneRect(0, 0, 0, 0);
}

void Graph::emitSliderPositionChanged(const QPointF &pos)
{
	if (_graphs.isEmpty())
		return;

	qreal val = pos.x() / _slider->area().width();
	emit sliderPositionChanged(val);

	const QPainterPath &path = _graphs.at(0)->path();
	QPointF p = path.pointAtPercent(val);
	qreal r = (p.y() - path.boundingRect().bottom())
	  / path.boundingRect().height();
	_sliderInfo->setPos(QPointF(0, _slider->boundingRect().height() * r));
	_sliderInfo->setText(QString::number(-p.y() * _yScale, 'f', _precision));
}

qreal Graph::sliderPosition() const
{
	return _slider->pos().x() / _slider->area().width();
}

void Graph::setSliderPosition(qreal pos)
{
	_slider->setPos(pos * _slider->area().width(), 0);
}

void Graph::newSliderPosition(const QPointF &pos)
{
	if (_slider->area().contains(pos)) {
		_slider->setPos(pos);
		emitSliderPositionChanged(pos);
	}
}

void Graph::addInfo(const QString &key, const QString &value)
{
	_info->insert(key, value);
}
