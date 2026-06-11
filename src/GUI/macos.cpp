#include <QFrame>
#include <QStyle>
#include "macos.h"

QFrame *MacOS::line()
{
	QFrame *l = new QFrame();
	l->setFrameShape(QFrame::HLine);
	l->setFrameShadow(QFrame::Sunken);

	return l;
}

bool MacOS::match(const QStyle *style)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	return (style->name().toLower() == "macos");
#else // QT6
#ifdef Q_OS_MAC
	return true;
#else // Q_OS_MAC
	return false;
#endif // Q_OS_MAC
#endif // QT6
}
