#ifndef ROUTEDATA_H
#define ROUTEDATA_H

#include <QVector>
#include <QString>
#include "waypoint.h"
#include "link.h"

class RouteData : public QVector<Waypoint>
{
public:
	const QString &name() const {return _name;}
	const QString &description() const {return _desc;}
	const QVector<Link> &links() const {return _links;}

	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &desc) {_desc = desc;}
	void addLink(const Link &link) {_links.append(link);}

private:
	QString _name;
	QString _desc;
	QVector<Link> _links;
};

#endif // ROUTEDATA_H
