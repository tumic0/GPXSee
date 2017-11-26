#ifndef ROUTEDATA_H
#define ROUTEDATA_H

#include <QVector>
#include <QString>
#include "waypoint.h"

class RouteData : public QVector<Waypoint>
{
public:
	const QString& name() const {return _name;}
	const QString& description() const {return _desc;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &desc) {_desc = desc;}

private:
	QString _name;
	QString _desc;
};

#endif // ROUTEDATA_H
