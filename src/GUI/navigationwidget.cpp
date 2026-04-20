#include <QEvent>
#include <QResizeEvent>
#include <QPainter>
#include "mapview.h"
#include "navigationwidget.h"

#define MARGIN 10
#define SIZE   40

#ifdef Q_OS_ANDROID

NavigationWidget::NavigationWidget(QWidget *parent)
  : QWidget(parent), _menuHover(false), _prevHover(false), _nextHover(false),
  _showPrev(false), _showNext(false)
{
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);

	newParent();
}

bool NavigationWidget::eventFilter(QObject *obj, QEvent *ev)
{
	if (obj == parent()) {
		if (ev->type() == QEvent::Resize)
			resize(static_cast<QResizeEvent*>(ev)->size());
		else if (ev->type() == QEvent::ChildAdded)
			raise();
	}

	return QWidget::eventFilter(obj, ev);
}

bool NavigationWidget::event(QEvent* ev)
{
	if (ev->type() == QEvent::ParentAboutToChange) {
		if (parent())
			parent()->removeEventFilter(this);
	} else if (ev->type() == QEvent::ParentChange)
		newParent();

	return QWidget::event(ev);
}

void NavigationWidget::paintEvent(QPaintEvent *ev)
{
	Q_UNUSED(ev);
	QPainter p(this);

	QColor c(Qt::black);
	c.setAlpha(_menuHover ? 128 : 64);
	p.setBrush(Qt::NoBrush);
	p.setPen(QPen(QBrush(c), 2, Qt::SolidLine, Qt::RoundCap));

	p.drawLine(
	  QPoint(rect().right() - (MARGIN + SIZE*0.875), rect().top() + MARGIN
		+ SIZE/4),
	  QPoint(rect().right() - (MARGIN + SIZE*0.125), rect().top() + MARGIN
		+ SIZE/4));
	p.drawLine(
	  QPoint(rect().right() - (MARGIN + SIZE*0.875), rect().top() + MARGIN
		+ SIZE/2),
	  QPoint(rect().right() - (MARGIN + SIZE*0.125), rect().top() + MARGIN
		+ SIZE/2));
	p.drawLine(
	  QPoint(rect().right() - (MARGIN + SIZE*0.875), rect().top() + MARGIN
		+ (SIZE/4)*3),
	  QPoint(rect().right() - (MARGIN + SIZE*0.125), rect().top() + MARGIN
		+ (SIZE/4)*3));

	p.setPen(Qt::NoPen);

	if (_showPrev) {
		c.setAlpha(_prevHover ? 128 : 64);
		p.setBrush(c);

		QPainterPath path;
		path.addEllipse(QRect(MARGIN, rect().center().y() - SIZE/2, SIZE, SIZE));
		path.moveTo(QPointF(MARGIN + 0.66*SIZE, rect().center().y() - SIZE/4));
		path.lineTo(QPointF(MARGIN + SIZE/4, rect().center().y()));
		path.lineTo(QPointF(MARGIN + 0.66*SIZE, rect().center().y() + SIZE/4));
		path.closeSubpath();
		p.drawPath(path);
	}
	if (_showNext) {
		c.setAlpha(_nextHover ? 128 : 64);
		p.setBrush(c);

		QPainterPath path;
		path.addEllipse(QRect(rect().right() - (MARGIN + SIZE),
		  rect().center().y() - SIZE/2, SIZE, SIZE));
		path.moveTo(QPointF(rect().right() - (MARGIN + 0.66*SIZE),
		  rect().center().y() - SIZE/4));
		path.lineTo(QPointF(rect().right() - (MARGIN + SIZE/4),
		  rect().center().y()));
		path.lineTo(QPointF(rect().right() - (MARGIN + 0.66*SIZE),
		  rect().center().y() + SIZE/4));
		path.closeSubpath();
		p.drawPath(path);
	}
}

void NavigationWidget::newParent()
{
	if (!parent())
		return;

	parent()->installEventFilter(this);
	raise();
}

bool NavigationWidget::pressed(const QPoint &pos)
{
	QRect menuRect(rect().right() - (MARGIN + SIZE), rect().top() + MARGIN,
	  SIZE, SIZE);
	QRect prevRect(MARGIN, rect().center().y() - SIZE/2, SIZE, SIZE);
	QRect nextRect(rect().right() - (MARGIN + SIZE), rect().center().y()
	  - SIZE/2, SIZE, SIZE);

	if (prevRect.contains(pos)) {
		_prevHover = true;
		update();
		return true;
	} else if (nextRect.contains(pos)) {
		_nextHover = true;
		update();
		return true;
	} else if (menuRect.contains(pos)) {
		_menuHover = true;
		update();
		return true;
	}

	return false;
}

bool NavigationWidget::released(const QPoint &pos)
{
	QRect menuRect(rect().right() - (MARGIN + SIZE), rect().top() + MARGIN,
	  SIZE, SIZE);
	QRect prevRect(MARGIN, rect().center().y() - SIZE/2, SIZE, SIZE);
	QRect nextRect(rect().right() - (MARGIN + SIZE), rect().center().y()
	  - SIZE/2, SIZE, SIZE);

	if (_menuHover || _prevHover || _nextHover) {
		_menuHover = false;
		_prevHover = false;
		_nextHover = false;

		update();
	}

	if (prevRect.contains(pos)) {
		emit prev();
		return true;
	} else if (nextRect.contains(pos)) {
		emit next();
		return true;
	} else if (menuRect.contains(pos)) {
		emit menu(pos);
		return true;
	}

	return false;
}

#endif // Q_OS_ANDROID
