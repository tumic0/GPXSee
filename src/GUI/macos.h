#ifndef MACOS_H
#define MACOS_H

class QFrame;
class QStyle;

namespace MacOS
{
	bool match(const QStyle *style);
	QFrame *line();
}

#endif // MACOS_H
