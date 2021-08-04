#include <QToolTip>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QBasicTimer>
#include <QScreen>
#include <QVBoxLayout>
#include <QApplication>
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include <QDesktopWidget>
#endif // QT 5.15
#include "tooltip.h"
#include "thumbnail.h"
#include "flowlayout.h"
#include "popup.h"


class PopupFrame : public QFrame
{
public:
	PopupFrame(const ToolTip &toolTip, QWidget *parent = 0);
	~PopupFrame();

	const ToolTip &toolTip() const {return _toolTip;}

	bool eventFilter(QObject *o, QEvent *ev);
	void place(const QPoint &pos, QWidget *w);
	void deleteAfterTimer();
	void stopTimer() {_timer.stop();}

	static PopupFrame *_instance;

protected:
	void paintEvent(QPaintEvent *event);
	void timerEvent(QTimerEvent *event);
	void contextMenuEvent(QContextMenuEvent *) {}

private:
	void createLayout(const ToolTip &content);

	QBasicTimer _timer;
	ToolTip _toolTip;
};

PopupFrame *PopupFrame::_instance = 0;

PopupFrame::PopupFrame(const ToolTip &toolTip, QWidget *parent)
  : QFrame(parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget
    | Qt::WindowDoesNotAcceptFocus), _toolTip(toolTip)
{
	delete _instance;
	_instance = this;

	setForegroundRole(QPalette::ToolTipText);
	setBackgroundRole(QPalette::ToolTipBase);
	setPalette(QToolTip::palette());
	ensurePolished();

	setFrameStyle(QFrame::NoFrame);
	setWindowOpacity(style()->styleHint(QStyle::SH_ToolTipLabel_Opacity, 0,
	  this) / 255.0);

	createLayout(toolTip);

	setMouseTracking(true);

	qApp->installEventFilter(this);
}

PopupFrame::~PopupFrame()
{
	_instance = 0;
}

void PopupFrame::createLayout(const ToolTip &content)
{
	QVBoxLayout *layout = new QVBoxLayout();
	int margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0,
	  this);
	layout->setContentsMargins(margin, margin, margin, margin);
	layout->setSpacing(0);

	if (!content.images().isEmpty()) {
		FlowLayout *imagesLayout = new FlowLayout(0, 2, 2);
		int size = qMin(960/content.images().size(), 240);

		for (int i = 0; i < content.images().size(); i++)
			imagesLayout->addWidget(new Thumbnail(content.images().at(i), size));

		layout->addLayout(imagesLayout);
	}

	if (!content.list().isEmpty()) {
		QString html = "<table>";
		for (int i = 0; i < content.list().count(); i++)
			html += "<tr><td align=\"right\"><b>" + content.list().at(i).key()
			  + ":&nbsp;</b></td><td>" + content.list().at(i).value()
			  + "</td></tr>";
		html += "</table>";

		QLabel *label = new QLabel(html);
		label->setAlignment(Qt::AlignLeft);
		label->setIndent(1);
		label->setTextInteractionFlags(Qt::TextBrowserInteraction);
		label->setOpenExternalLinks(true);
		label->setWordWrap(true);

		layout->addWidget(label);
	}

	setLayout(layout);
}

void PopupFrame::paintEvent(QPaintEvent *event)
{
	QStylePainter p(this);
	QStyleOptionFrame opt;
	opt.initFrom(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
	p.end();
	QFrame::paintEvent(event);
}

void PopupFrame::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timer.timerId()) {
		_timer.stop();
		deleteLater();
	}
}

bool PopupFrame::eventFilter(QObject *o, QEvent *ev)
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

void PopupFrame::place(const QPoint &pos, QWidget *w)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
	QRect screen = QApplication::desktop()->screenGeometry(w);
#else // QT 5.15
	QRect screen = w->screen()->geometry();
#endif // QT 5.15
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

void PopupFrame::deleteAfterTimer()
{
	if (!_timer.isActive())
		_timer.start(300, this);
}

void Popup::show(const QPoint &pos, const ToolTip &toolTip, QWidget *w)
{
	if (toolTip.isEmpty())
		return;

	if (PopupFrame::_instance) {
		if (toolTip == PopupFrame::_instance->toolTip())
			PopupFrame::_instance->stopTimer();
		else {
			delete PopupFrame::_instance;
			PopupFrame::_instance = new PopupFrame(toolTip);
		}
	} else
		PopupFrame::_instance = new PopupFrame(toolTip);

	PopupFrame::_instance->resize(PopupFrame::_instance->sizeHint());
	PopupFrame::_instance->place(pos, w);
	PopupFrame::_instance->showNormal();
}

void Popup::clear()
{
	if (PopupFrame::_instance)
		delete PopupFrame::_instance;
}
