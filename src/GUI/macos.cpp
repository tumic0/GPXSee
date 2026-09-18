#include <QFrame>
#include <QLabel>
#include <QStyle>
#include "macos.h"

QFrame *MacOS::line()
{
	QFrame *l = new QFrame();
	l->setFrameShape(QFrame::HLine);
	l->setFrameShadow(QFrame::Sunken);

	return l;
}

QLabel *MacOS::heading(const QString &text)
{
	QLabel *l = new QLabel(text);
	QFont font = l->font();
	font.setWeight(QFont::Medium);
	l->setFont(font);
	l->setAlignment(Qt::AlignHCenter);

	return l;
}

bool MacOS::match(const QStyle *style)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
	return (style->name().toLower() == "macos");
#else // QT6
	Q_UNUSED(style);
#ifdef Q_OS_MACOS
	return true;
#else // Q_OS_MACOS
	return false;
#endif // Q_OS_MACOS
#endif // QT6
}
