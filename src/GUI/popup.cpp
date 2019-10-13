#include <QToolTip>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QBasicTimer>
#include "popup.h"

#include <QDebug>


class Label : public QLabel
{
public:
	Label(const QString &text, QWidget *parent = 0);
	~Label();

	bool eventFilter(QObject *o, QEvent *ev);
	void place(const QPoint &pos, QWidget *w);
	void deleteAfterTimer();
	void stopTimer() {_timer.stop();}

	static Label *_instance;

protected:
	void paintEvent(QPaintEvent *event);
	void timerEvent(QTimerEvent *event);

private:
	QBasicTimer _timer;
};

Label *Label::_instance = 0;

Label::Label(const QString &text, QWidget *parent)
  : QLabel(text, parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget)
{
	delete _instance;
	_instance = this;

	setForegroundRole(QPalette::ToolTipText);
	setBackgroundRole(QPalette::ToolTipBase);
	setPalette(QToolTip::palette());
	ensurePolished();
	setMargin(1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0,
	  this));
	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft);
	setIndent(1);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0,
	  this) / 255.0);

	setTextInteractionFlags(Qt::TextBrowserInteraction);
	setOpenExternalLinks(true);

	setMouseTracking(true);

	qApp->installEventFilter(this);
}

Label::~Label()
{
	_instance = 0;
}

void Label::paintEvent(QPaintEvent *event)
{
	QStylePainter p(this);
	QStyleOptionFrame opt;
	opt.init(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
	p.end();
	QLabel::paintEvent(event);
}

void Label::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timer.timerId()) {
		_timer.stop();
		deleteLater();
	}
}

bool Label::eventFilter(QObject *o, QEvent *ev)
{
	Q_UNUSED(o);

	switch (ev->type()) {
		case QEvent::KeyPress:
		case QEvent::KeyRelease: {
			const int key = static_cast<QKeyEvent *>(ev)->key();
			if (key == Qt::Key_Escape) {
				deleteLater();
				return true;
			}
			break;
		}
		case QEvent::FocusIn:
		case QEvent::FocusOut:
		case QEvent::WindowActivate:
		case QEvent::WindowDeactivate:
		case QEvent::Close:
			deleteLater();
			break;
		case QEvent::MouseMove: {
			QRectF r(geometry().adjusted(-5, -20, 5, 20));
			QPointF p(static_cast<QMouseEvent*>(ev)->screenPos());
			if (!r.contains(p))
				deleteAfterTimer();
			break;
		}
		default:
			break;
	}

	return false;
}

void Label::place(const QPoint &pos, QWidget *w)
{
	QRect screen = QApplication::desktop()->screenGeometry(w);
	QPoint p(pos.x() + 2, pos.y() + 16);

	if (p.x() + width() > screen.x() + screen.width())
		p.rx() -= 4 + width();
	if (p.y() + height() > screen.y() + screen.height())
		p.ry() -= 24 + height();
	if (p.y() < screen.y())
		p.setY(screen.y());
	if (p.x() + width() > screen.x() + screen.width())
		p.setX(screen.x() + screen.width() - width());
	if (p.x() < screen.x())
		p.setX(screen.x());
	if (p.y() + height() > screen.y() + screen.height())
		p.setY(screen.y() + screen.height() - height());

	this->move(p);
}

void Label::deleteAfterTimer()
{
	if (!_timer.isActive())
		_timer.start(300, this);
}


void Popup::show(const QPoint &pos, const QString &text, QWidget *w)
{
	if (Label::_instance) {
		Label::_instance->stopTimer();
		Label::_instance->setText(text);
		Label::_instance->resize(Label::_instance->sizeHint());
	} else {
		Label::_instance = new Label(text);
		Label::_instance->showNormal();
	}

	Label::_instance->place(pos, w);
}
