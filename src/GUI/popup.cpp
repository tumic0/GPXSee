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


class PopupLabel : public QLabel
{
public:
	PopupLabel(const QString &text, QWidget *parent = 0);
	~PopupLabel();

	bool eventFilter(QObject *o, QEvent *ev);
	void place(const QPoint &pos, QWidget *w);
	void deleteAfterTimer();
	void stopTimer() {_timer.stop();}

	static PopupLabel *_instance;

protected:
	void paintEvent(QPaintEvent *event);
	void timerEvent(QTimerEvent *event);
	void contextMenuEvent(QContextMenuEvent *) {}

private:
	QBasicTimer _timer;
};

PopupLabel *PopupLabel::_instance = 0;

PopupLabel::PopupLabel(const QString &text, QWidget *parent)
  : QLabel(text, parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
	| Qt::WindowDoesNotAcceptFocus
#endif // QT5
)
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
	setWordWrap(true);

	setMouseTracking(true);

	qApp->installEventFilter(this);
}

PopupLabel::~PopupLabel()
{
	_instance = 0;
}

void PopupLabel::paintEvent(QPaintEvent *event)
{
	QStylePainter p(this);
	QStyleOptionFrame opt;
	opt.init(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
	p.end();
	QLabel::paintEvent(event);
}

void PopupLabel::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timer.timerId()) {
		_timer.stop();
		deleteLater();
	}
}

bool PopupLabel::eventFilter(QObject *o, QEvent *ev)
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
			QPointF p(static_cast<QMouseEvent*>(ev)->globalPos());
			if (!r.contains(p))
				deleteAfterTimer();
			break;
		}
		default:
			break;
	}

	return false;
}

void PopupLabel::place(const QPoint &pos, QWidget *w)
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

void PopupLabel::deleteAfterTimer()
{
	if (!_timer.isActive())
		_timer.start(300, this);
}


void Popup::show(const QPoint &pos, const QString &text, QWidget *w)
{
	if (PopupLabel::_instance) {
		PopupLabel::_instance->stopTimer();
		PopupLabel::_instance->setText(text);
	} else
		PopupLabel::_instance = new PopupLabel(text);

	PopupLabel::_instance->resize(PopupLabel::_instance->sizeHint());
	PopupLabel::_instance->place(pos, w);
	PopupLabel::_instance->showNormal();
}

void Popup::clear()
{
	if (PopupLabel::_instance)
		delete PopupLabel::_instance;
}
