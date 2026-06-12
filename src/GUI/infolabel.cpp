#include "macos.h"
#include "infolabel.h"

InfoLabel::InfoLabel(const QString &text, QWidget *parent)
  : QLabel(text, parent)
{
	QFont f(font());
	if (MacOS::match(style()))
		f.setPointSize(qMax(10, f.pointSize() - 2));
	else
		f.setPointSize(f.pointSize() - 1);

	setWordWrap(true);
	setFont(f);
}
