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
	const QString &comment() const {return _comment;}
	const QVector<Link> &links() const {return _links;}

	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &desc) {_desc = desc;}
	void setComment(const QString &comment) {_comment = comment;}
	void addLink(const Link &link) {_links.append(link);}

private:
	QString _name;
	QString _desc;
	QString _comment;
	QVector<Link> _links;
};

#endif // ROUTEDATA_H
