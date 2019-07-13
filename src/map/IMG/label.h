#ifndef LABEL_H
#define LABEL_H

#include <QString>
#include <QDebug>

#define FIRST_SHIELD Label::Shield::USInterstate
#define LAST_SHIELD  Label::Shield::Oval

class Label {
public:
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

inline uint qHash(const Label::Shield &shield)
{
	return qHash(shield.text()) ^ shield.type();
}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Label::Shield &shield)
{
	dbg.nospace() << "Shield(" << shield.type() << ", " << shield.text() << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const Label &label)
{
	dbg.nospace() << "Label(";
	if (label.shield().isValid())
		dbg << label.shield() << ", ";
	dbg << label.text() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // LABEL_H
