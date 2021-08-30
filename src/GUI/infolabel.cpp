#include "infolabel.h"

InfoLabel::InfoLabel(const QString &text, QWidget *parent)
  : QLabel(text, parent)
{
	QFont f(font());
	f.setPointSize(f.pointSize() - 1);
	setWordWrap(true);
	setFont(f);
}
