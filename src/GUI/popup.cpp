#include <QToolTip>
#include <QStyle>
#include <QStylePainter>
#include <QStyleOptionFrame>
#include <QLabel>
#include <QMouseEvent>
#include <QBasicTimer>
#include <QScreen>
#include <QApplication>
#include <QVBoxLayout>
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
#include <QDesktopWidget>
#endif // QT 5.15
#include "data/imageinfo.h"
#include "imagelabel.h"
#include "popup.h"
#include <QDebug>

static QSize thumbnailSize(const ImageInfo &img, int limit)
{
	int width, height;
	if (img.size().width() > img.size().height()) {
		width = qMin(img.size().width(), limit);
		qreal ratio = img.size().width() / (qreal)img.size().height();
		height = (int)(width / ratio);
	} else {
		height = qMin(img.size().height(), limit);
		qreal ratio = img.size().height() / (qreal)img.size().width();
		width = (int)(height / ratio);
	}

	return QSize(width, height);
}

class PopupWidget : public QFrame
{
public:
	PopupWidget(const QVector<ImageInfo> &images, const QString &text, QWidget *parent = 0);
	~PopupWidget();

	bool eventFilter(QObject *o, QEvent *ev);
	void place(const QPoint &pos, QWidget *w);
	void deleteAfterTimer();
	void stopTimer() {_timer.stop();}
	void setImages(const QVector<ImageInfo> &images);

	static PopupWidget *_instance;
	QLabel *label;

protected:
	void paintEvent(QPaintEvent *event);
	void timerEvent(QTimerEvent *event);
	void contextMenuEvent(QContextMenuEvent *) {}

private:
	QBasicTimer _timer;
	QHBoxLayout *imagesLayout;
};

PopupWidget *PopupWidget::_instance = 0;

PopupWidget::PopupWidget(const QVector<ImageInfo> &images, const QString &text, QWidget *parent)
  : QFrame(parent, Qt::ToolTip | Qt::BypassGraphicsProxyWidget
	| Qt::WindowDoesNotAcceptFocus | Qt::FramelessWindowHint)
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

	setMouseTracking(true);

	QVBoxLayout *layout = new QVBoxLayout();
	int margin = 1 + style()->pixelMetric(QStyle::PM_ToolTipLabelFrameWidth, 0, this);
	layout->setContentsMargins(margin, margin, margin, margin);

	imagesLayout = new QHBoxLayout();
	imagesLayout->setContentsMargins(0, 0, 0, 0);
	imagesLayout->setSpacing(10);

	setImages(images);

	layout->addLayout(imagesLayout);

	label = new QLabel(text, this);
	label->setMargin(0);
	label->setAlignment(Qt::AlignLeft);
	label->setIndent(1);
	label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	label->setOpenExternalLinks(true);
	label->setWordWrap(true);
	label->setFrameStyle(QFrame::NoFrame);
	layout->addWidget(label);

	layout->addStretch();
	setLayout(layout);

	qApp->installEventFilter(this);
}

PopupWidget::~PopupWidget()
{
	_instance = 0;
}

void PopupWidget::setImages(const QVector<ImageInfo> &images)
{
	for (QLayoutItem *item = imagesLayout->takeAt(0); item != NULL; item = imagesLayout->takeAt(0)) {
		item->widget()->deleteLater();
	}

	for (int i = 0; i < images.size(); i++) {
		const ImageInfo &img = images.at(i);
		QSize size(thumbnailSize(img, qMin(960/images.size(), 240)));

		QWidget *imageLabel = new ImageLabel(img.path(), size, this);
		imageLabel->show();
		imagesLayout->addWidget(imageLabel);
	}
}

void PopupWidget::paintEvent(QPaintEvent *event)
{
	QStylePainter p(this);
	QStyleOptionFrame opt;
	opt.initFrom(this);
	p.drawPrimitive(QStyle::PE_PanelTipLabel, opt);
	p.end();
	QFrame::paintEvent(event);
}

void PopupWidget::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == _timer.timerId()) {
		_timer.stop();
		deleteLater();
	}
}

bool PopupWidget::eventFilter(QObject *o, QEvent *ev)
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

void PopupWidget::place(const QPoint &pos, QWidget *w)
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

void PopupWidget::deleteAfterTimer()
{
	if (!_timer.isActive())
		_timer.start(300, this);
}

void Popup::show(const QPoint &pos, const QVector<ImageInfo> &images, const QString &text, QWidget *w)
{
	if (text.isEmpty())
		return;

	if (PopupWidget::_instance) {
		PopupWidget::_instance->stopTimer();
		PopupWidget::_instance->label->setText(text);
		PopupWidget::_instance->setImages(images);
	} else
		PopupWidget::_instance = new PopupWidget(images, text);

	PopupWidget::_instance->resize(PopupWidget::_instance->sizeHint());
	PopupWidget::_instance->place(pos, w);
	PopupWidget::_instance->showNormal();
}

void Popup::clear()
{
	if (PopupWidget::_instance)
		delete PopupWidget::_instance;
}
