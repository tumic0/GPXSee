#ifndef IMG_SECTION_H
#define IMG_SECTION_H

#include <QtGlobal>

namespace IMG {

struct Section {
	quint32 offset;
	quint32 size;

	Section() : offset(0), size(0) {}
};

}

#endif // IMG_SECTION_H
