#include <QToolTip>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QBasicTimer>
#include <QScreen>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QApplication>
#include "tooltip.h"
#include "thumbnail.h"
#include "flowlayout.h"
#include "popup.h"

static inline QPointF mousePos(QEvent *ev)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return static_cast<QMouseEvent*>(ev)->globalPos();
#else // QT 6
	return static_cast<QMouseEvent*>(ev)->globalPosition();
#endif // QT 6
}

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
		QFormLayout *textLayout = new QFormLayout();
		textLayout->setLabelAlignment(Qt::AlignRight);
		textLayout->setHorizontalSpacing(5);
		textLayout->setVerticalSpacing(2);

		for (int i = 0; i < content.list().count(); i++) {
			QLabel *key = new QLabel(content.list().at(i).key() + ":");
			key->setTextFormat(Qt::PlainText);
			key->setAlignment(Qt::AlignTop);
			key->setStyleSheet("font-weight: bold");
			QLabel *value = new QLabel(content.list().at(i).value());
			value->setSizePolicy(QSizePolicy::MinimumExpanding,
			  QSizePolicy::Preferred);
			value->setTextFormat(Qt::RichText);
			value->setAlignment(Qt::AlignTop);
			value->setTextInteractionFlags(Qt::TextBrowserInteraction);
			value->setOpenExternalLinks(true);
			value->setWordWrap(true);

			textLayout->addRow(key, value);
		}

		layout->addLayout(textLayout);
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
			if (!r.contains(mousePos(ev)))
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
	QRect screen = w->screen()->geometry();
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
