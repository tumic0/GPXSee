#include <QJsonDocument>
#include <QJsonArray>
#include "mvtstyle.h"

static QStringList stringlist(const QJsonArray &array)
{
	QStringList list;
	list.reserve(array.size());

	for (int i = 0; i < array.size(); i++)
		list.append(array.at(i).toString());

	return list;
}

QList<MVTStyle> MVTStyle::fromJSON(const QByteArray &json)
{
	QList<MVTStyle> list;
	QJsonDocument doc(QJsonDocument::fromJson(json));

	if (doc.isNull())
		return QList<MVTStyle>();
	else {
		QJsonArray styles(doc.array());
		for (int i = 0; i < styles.size(); i++) {
			list.append(MVTStyle(styles.at(i)["name"].toString(),
			  stringlist(styles.at(i)["layers"].toArray())));
		}
	}

	return list;
}
