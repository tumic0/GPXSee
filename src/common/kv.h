#ifndef KV_H
#define KV_H

template <class KEY, class VALUE>
class KV {
public:
	KV(const KEY &key, const VALUE &value) : _key(key), _value(value) {}

	const KEY &key() const {return _key;}
	const VALUE &value() const {return _value;}

	bool operator==(const KV &other) const {return _key == other._key;}
	bool operator<(const KV &other) const {return _key < other._key;}

private:
	KEY _key;
	VALUE _value;
};

#endif // KV_H
