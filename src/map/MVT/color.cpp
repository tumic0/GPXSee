#include <QDebug>
#include "color.h"

using namespace MVT;

static qreal pval(const QString &str)
{
	QString ts(str.trimmed());
	ts.chop(1);
	return ts.toFloat() / 100.0;
}

QColor Color::fromJsonString(const QString &str)
{
	QColor ret;

	if (str.startsWith("rgb(")) {
		QStringList comp(str.mid(4, str.size() - 5).split(','));
		if (comp.size() != 3)
			return QColor();
		ret = QColor(comp.at(0).toInt(), comp.at(1).toInt(),
		  comp.at(2).toInt());
	} else if (str.startsWith("rgba(")) {
		QStringList comp(str.mid(5, str.size() - 6).split(','));
		if (comp.size() != 4)
			return QColor();
		ret = QColor(comp.at(0).toInt(), comp.at(1).toInt(),
		  comp.at(2).toInt(), (int)(comp.at(3).toFloat() * 255));
	} else if (str.startsWith("hsl(")) {
		QStringList comp(str.mid(4, str.size() - 5).split(','));
		if (comp.size() != 3)
			return QColor();
		ret = QColor::fromHslF(comp.at(0).toFloat() / 360.0, pval(comp.at(1)),
		  pval(comp.at(2)));
	} else if (str.startsWith("hsla(")) {
		QStringList comp(str.mid(5, str.size() - 6).split(','));
		if (comp.size() != 4)
			return QColor();
		ret = QColor::fromHslF(comp.at(0).toFloat() / 360.0, pval(comp.at(1)),
		  pval(comp.at(2)), comp.at(3).toFloat());
	} else
		ret = QColor(str);

	if (!ret.isValid())
		qWarning() << str << ": invalid color";

	return ret;
}
