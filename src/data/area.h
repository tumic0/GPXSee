#ifndef AREA_H
#define AREA_H

#include <QString>
#include <QList>
#include "polygon.h"

class Area : public QList<Polygon>
{
public:
	const QString& name() const {return _name;}
	const QString& description() const {return _desc;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &desc) {_desc = desc;}

	bool isValid() const
	{
		if (isEmpty())
			return false;
		for (int i = 0; i < size(); i++)
			if (!at(i).isValid())
				return false;
		return true;
	}

	RectC boundingRect() const
	{
		RectC ret;
		for (int i = 0; i < size(); i++)
			ret |= at(i).boundingRect();
		return ret;
	}

private:
	QString _name;
	QString _desc;
};

#endif // AREA_H
