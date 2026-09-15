#ifndef MACOS_H
#define MACOS_H

class QString;
class QFrame;
class QLabel;
class QStyle;

namespace MacOS
{
	bool match(const QStyle *style);

	QFrame *line();
	QLabel *heading(const QString &text);
}

#endif // MACOS_H
