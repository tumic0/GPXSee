#ifndef SEARCHPOINTER_H
#define SEARCHPOINTER_H

#include "common/hash.h"

template <class T>
class SearchPointer
{
public:
	SearchPointer(const T *ptr) : _ptr(ptr) {}

	const T *data() const {return _ptr;}
	bool operator==(const SearchPointer<T> &other) const
	  {return *data() == *(other.data());}

private:
	const T *_ptr;
};

template <class T>
inline HASH_T qHash(const SearchPointer<T> &t)
{
	return ::qHash(*(t.data()));
}

#endif // SEARCHPOINTER_H
