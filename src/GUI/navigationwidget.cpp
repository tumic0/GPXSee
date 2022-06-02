#include <QEvent>
#include <QResizeEvent>
#include <QPainter>
#include "mapview.h"
#include "navigationwidget.h"

#define MARGIN 5
#define SIZE   40

#ifdef Q_OS_ANDROID

NavigationWidget::NavigationWidget(MapView *view)
  : QWidget(view), _showPrev(false), _showNext(false)
{
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents);

	newParent();

	connect(view, &MapView::clicked, this, &NavigationWidget::viewClicked);
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
	c.setAlpha(64);
	p.setBrush(c);
	p.setPen(Qt::NoPen);

	if (_showPrev) {
		QPainterPath path;
		path.addEllipse(QRect(MARGIN, rect().center().y() - SIZE/2, SIZE, SIZE));
		path.moveTo(QPointF(MARGIN + 0.66*SIZE, rect().center().y() - SIZE/4));
		path.lineTo(QPointF(MARGIN + SIZE/4, rect().center().y()));
		path.lineTo(QPointF(MARGIN + 0.66*SIZE, rect().center().y() + SIZE/4));
		path.closeSubpath();
		p.drawPath(path);
	}
	if (_showNext) {
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

void NavigationWidget::viewClicked(const QPoint &pos)
{
	QRect prevRect(MARGIN, rect().center().y() - SIZE/2, SIZE, SIZE);
	QRect nextRect(rect().right() - (MARGIN + SIZE), rect().center().y()
	  - SIZE/2, SIZE, SIZE);

	if (prevRect.contains(pos))
		emit prev();
	else if (nextRect.contains(pos))
		emit next();
}

#endif // Q_OS_ANDROID
