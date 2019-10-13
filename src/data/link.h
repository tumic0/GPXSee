#ifndef LINK_H
#define LINK_H

#include <QString>

class Link {
public:
	Link() {}
	Link(const QString &URL, const QString &text = QString())
		: _URL(URL), _text(text) {}

	const QString &URL() const {return _URL;}
	const QString &text() const {return _text;}

private:
	QString _URL;
	QString _text;
};

#endif // LINK_H
