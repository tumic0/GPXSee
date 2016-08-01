#ifndef TOOLTIP_H
#define TOOLTIP_H

#include <QString>
#include <QList>

class ToolTip
{
public:
	ToolTip();

	void insert(const QString &key, const QString &value);
	QString toString();

private:
	class KV {
	public:
		QString key;
		QString value;

		KV(const QString &k, const QString &v)
		  {key = k; value = v;}
		bool operator==(const KV &other) const
		  {return this->key == other.key;}
	};

	QList<KV> _list;
};

#endif // TOOLTIP_H
