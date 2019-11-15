#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QString>
#include <QList>
#include <QVector>
#include "common/kv.h"
#include "data/imageinfo.h"

class ToolTip
{
public:
	void insert(const QString &key, const QString &value);
	void setImages(const QVector<ImageInfo> &images) {_images = images;}
	QString toString() const;

private:
	QList<KV<QString, QString> > _list;
	QVector<ImageInfo> _images;
};

#endif // TOOLTIP_H
