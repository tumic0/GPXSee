#ifndef IMG_SHIELD_H
#define IMG_SHIELD_H

#include <QString>
#include "common/config.h"

#define FIRST_SHIELD Shield::USInterstate
#define LAST_SHIELD  Shield::Oval

namespace IMG {

class Shield
{
public:
	enum Type {
		None,
		USInterstate,
		USShield,
		USRound,
		Hbox,
		Box,
		Oval
	};

	Shield() : _type(None) {}
	Shield(Type type, const QString &name) : _type(type), _text(name) {}

	Type type() const {return _type;}
	const QString &text() const {return _text;}
	bool isValid() const {return _type > None && !_text.isEmpty();}

	bool operator==(const Shield &other) const
	  {return _type == other._type && _text == other._text;}

private:
	Type _type;
	QString _text;
};

inline HASH_T qHash(const IMG::Shield &shield)
{
	return ::qHash(shield.text()) ^ ::qHash(shield.type());
}

}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const IMG::Shield &shield)
{
	dbg.nospace() << "Shield(" << shield.type() << ", " << shield.text() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // IMG_SHIELD_H
