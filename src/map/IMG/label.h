#ifndef IMG_LABEL_H
#define IMG_LABEL_H

#include <QString>
#include <QDebug>
#include "shield.h"

namespace IMG {

class Label {
public:
	Label() {}
	Label(const QString &text, const Shield &shield = Shield())
	  : _text(text), _shield(shield) {}

	const Shield &shield() const {return _shield;}
	const QString &text() const {return _text;}
	bool isValid() const {return _shield.isValid() || !_text.isEmpty();}

	void setText(const QString &text) {_text = text;}

private:
	QString _text;
	Shield _shield;
};

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::Label &label)
{
	dbg.nospace() << "Label(";
	if (label.shield().isValid())
		dbg << label.shield() << ", ";
	dbg << label.text() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_LABEL_H
