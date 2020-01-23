#ifndef LINK_H
#define LINK_H

#include <QString>

class Link {
public:
	Link() {}
	Link(const QString &URL, const QString &text = QString())
		: _url(URL), _text(text) {}

	void setURL(const QString &URL) {_url = URL;}
	void setText(const QString &text) {_text = text;}
	const QString &URL() const {return _url;}
	const QString &text() const {return _text;}

private:
	QString _url;
	QString _text;
};

#endif // LINK_H
