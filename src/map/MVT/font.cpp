#include <QString>
#include <QFontDatabase>
#include <QJsonArray>
#include "font.h"

using namespace MVT;

static QList<QPair<QString, QFont::Weight> > weights()
{
	QList<QPair<QString, QFont::Weight> > list;

	list.append(QPair<QString, QFont::Weight>("Thin", QFont::Thin));
	list.append(QPair<QString, QFont::Weight>("Extra Light", QFont::ExtraLight));
	list.append(QPair<QString, QFont::Weight>("Light", QFont::Light));
	list.append(QPair<QString, QFont::Weight>("Regular", QFont::Normal));
	list.append(QPair<QString, QFont::Weight>("Medium", QFont::Medium));
	list.append(QPair<QString, QFont::Weight>("Demi Bold", QFont::DemiBold));
	list.append(QPair<QString, QFont::Weight>("Extra Bold", QFont::ExtraBold));
	list.append(QPair<QString, QFont::Weight>("Bold", QFont::Bold));
	list.append(QPair<QString, QFont::Weight>("Black", QFont::Black));

	return list;
}

static QList<QPair<QString, QFont::Stretch> > stretches()
{
	QList<QPair<QString, QFont::Stretch> > list;

	list.append(QPair<QString, QFont::Stretch>("Ultra Condensed",
	  QFont::UltraCondensed));
	list.append(QPair<QString, QFont::Stretch>("Extra Condensed",
	  QFont::ExtraCondensed));
	list.append(QPair<QString, QFont::Stretch>("Semi Expanded",
	  QFont::SemiExpanded));
	list.append(QPair<QString, QFont::Stretch>("Extra Expanded",
	  QFont::ExtraExpanded));
	list.append(QPair<QString, QFont::Stretch>("Ultra Expanded",
	  QFont::UltraExpanded));
	list.append(QPair<QString, QFont::Stretch>("Condensed",
	  QFont::Condensed));
	list.append(QPair<QString, QFont::Stretch>("Expanded",
	  QFont::Expanded));

	return list;
}

static QList<QPair<QString, QFont::Style> > styles()
{
	QList<QPair<QString, QFont::Style> > list;

	list.append(QPair<QString, QFont::Style>("Italic", QFont::StyleItalic));
	list.append(QPair<QString, QFont::Style>("Oblique", QFont::StyleOblique));

	return list;
}

static QList<QPair<QString, QFont::Weight> > weightList = weights();
static QList<QPair<QString, QFont::Stretch> > stretchList = stretches();
static QList<QPair<QString, QFont::Style> > styleList = styles();

static const QStringList &fonts()
{
	static QStringList l(QFontDatabase().families());
	return l;
}

static void parse(const QString &str, QString &family, QFont::Weight &weight,
  QFont::Stretch &stretch, QFont::Style &style)
{
	int weightIndex, stretchIndex, styleIndex, index;
	weightIndex = stretchIndex = styleIndex = str.length();

	for (int i = 0; i < weightList.size(); i++) {
		if ((index = str.indexOf(weightList.at(i).first)) >= 0) {
			weightIndex = index - 1;
			weight = weightList.at(i).second;
			break;
		}
	}

	for (int i = 0; i < stretchList.size(); i++) {
		if ((index = str.indexOf(stretchList.at(i).first)) >= 0) {
			stretchIndex = index - 1;
			stretch = stretchList.at(i).second;
			break;
		}
	}

	for (int i = 0; i < styleList.size(); i++) {
		if ((index = str.indexOf(styleList.at(i).first)) >= 0) {
			styleIndex = index - 1;
			style = styleList.at(i).second;
			break;
		}
	}

	family = str.mid(0, qMin(qMin(weightIndex, stretchIndex), styleIndex));
}

static bool matchFamily(const QString &family)
{
	for (int j = 0; j < fonts().size(); j++)
		if (fonts().at(j).startsWith(family))
			return true;

	return false;
}

QFont Font::fromJsonArray(const QJsonArray &json)
{
	if (json.isEmpty())
		return QFont("Open Sans");

	// Try exact match in order of the font list
	QString family;
	QFont::Weight weight = QFont::Normal;
	QFont::Stretch stretch = QFont::Unstretched;
	QFont::Style style = QFont::StyleNormal;

	for (int i = 0; i < json.size(); i++) {
		if (!json.at(i).isString())
			continue;
		parse(json.at(i).toString(), family, weight, stretch, style);
		if (matchFamily(family)) {
			QFont font(family);
			font.setWeight(weight);
			font.setStretch(stretch);
			font.setStyle(style);
			return font;
		}
	}

	// Use Qt's font matching logic on the first font in the list
	parse(json.first().toString(), family, weight, stretch, style);

	QFont font(family);
	font.setWeight(weight);
	font.setStretch(stretch);
	font.setStyle(style);

	return font;
}
