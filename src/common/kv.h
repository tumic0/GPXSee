#ifndef KV_H
#define KV_H

#include <QString>

class KV {
public:
	KV(const QString &key, const QString &value) : _key(key), _value(value) {}

	const QString &key() const {return _key;}
	const QString &value() const {return _value;}

	bool operator==(const KV &other) const
	  {return this->key() == other.key();}

private:
	QString _key;
	QString _value;
};

#endif // KV_H
