#ifndef MVT_FONT_H
#define MVT_FONT_H

#include <QFont>

class QJsonArray;

namespace MVT {

namespace Font
{
	QFont fromJsonArray(const QJsonArray &json);
}

}

#endif // MVT_FONT_H
