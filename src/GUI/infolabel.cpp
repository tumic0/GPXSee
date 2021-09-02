#include <QtGlobal>
#include "infolabel.h"

InfoLabel::InfoLabel(const QString &text, QWidget *parent)
  : QLabel(text, parent)
{
	QFont f(font());
#ifdef Q_OS_MAC
	f.setPointSize(qMax(10, f.pointSize() - 2));
#else // Q_OS_MAC
	f.setPointSize(f.pointSize() - 1);
#endif // Q_OS_MAC
	setWordWrap(true);
	setFont(f);
}
