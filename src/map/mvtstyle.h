#ifndef MVTSTYLE_H
#define MVTSTYLE_H

#include <QStringList>

class MVTStyle
{
public:
	MVTStyle(const QString &name, const QStringList &layers)
	  : _name(name), _layers(layers) {}

	const QString &name() const {return _name;}
	bool matches(const QStringList &layers) const;

	static QList<MVTStyle> fromJSON(const QByteArray &json);

private:
	QString _name;
	QStringList _layers;
};

#endif // MVTSTYLE_H
