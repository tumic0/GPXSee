#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QString>
#include <QList>
#include "common/kv.h"
#include "data/imageinfo.h"

class ToolTip
{
public:
	void insert(const QString &key, const QString &value);
	void setImage(const ImageInfo &image) {_img = image;}
	QString toString() const;

private:
	QList<KV<QString, QString> > _list;
	ImageInfo _img;
};

#endif // TOOLTIP_H
