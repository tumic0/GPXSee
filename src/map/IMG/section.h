#ifndef SECTION_H
#define SECTION_H

#include <QtGlobal>

namespace IMG {

struct Section {
	quint32 offset;
	quint32 size;

	Section() : offset(0), size(0) {}
};

}

#endif // SECTION_H
