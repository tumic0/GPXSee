#ifndef METATYPE_H
#define METATYPE_H

#include <QSqlField>

static inline QMetaType::Type METATYPE(const QSqlField &f)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	return static_cast<QMetaType::Type>(f.type());
#else // QT 6
	return static_cast<QMetaType::Type>(f.metaType().id());
#endif // QT 6
}

#endif // METATYPE_H
