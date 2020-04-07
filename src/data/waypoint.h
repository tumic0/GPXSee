#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QString>
#include <QDateTime>
#include <QHash>
#include <QVector>
#include <QDebug>
#include "common/coordinates.h"
#include "imageinfo.h"
#include "link.h"
#include "address.h"

class Waypoint
{
public:
	Waypoint() : _elevation(NAN) {}
	Waypoint(const Coordinates &coordinates)
	  : _coordinates(coordinates), _elevation(NAN) {}

	const Coordinates &coordinates() const {return _coordinates;}
	Coordinates &rcoordinates() {return _coordinates;}
	const QString &name() const {return _name;}
	const QString &description() const {return _description;}
	const QString &comment() const {return _comment;}
	const Address &address() const {return _address;}
	const QVector<ImageInfo> &images() const {return _images;}
	const QVector<Link> &links() const {return _links;}
	const QDateTime &timestamp() const {return _timestamp;}
	qreal elevation() const {return _elevation;}

	QPair<qreal, qreal> elevations() const;

	void setCoordinates(const Coordinates &coordinates)
	  {_coordinates = coordinates;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &description)
	  {_description = description;}
	void setComment(const QString &comment) {_comment = comment;}
	void setAddress(const Address &address) {_address = address;}
	void setTimestamp(const QDateTime &timestamp) {_timestamp = timestamp;}
	void setElevation(qreal elevation) {_elevation = elevation;}
	void addImage(const ImageInfo &image) {_images.append(image);}
	void addLink(const Link &link) {_links.append(link);}

	bool hasElevation() const {return !std::isnan(_elevation);}

	bool operator==(const Waypoint &other) const
	  {return this->_name == other._name
	  && this->_coordinates == other._coordinates;}

	static void useDEM(bool use) {_useDEM = use;}
	static void showSecondaryElevation(bool show)
	  {_show2ndElevation = show;}

private:
	Coordinates _coordinates;
	QString _name;
	QString _description;
	QString _comment;
	Address _address;
	QVector<ImageInfo> _images;
	QVector<Link> _links;
	QDateTime _timestamp;
	qreal _elevation;

	static bool _useDEM;
	static bool _show2ndElevation;
};

inline uint qHash(const Waypoint &key)
{
	return ::qHash(key.name());
}

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const Waypoint &waypoint)
{
	dbg.nospace() << "Waypoint(" << waypoint.coordinates() << ", "
	  << waypoint.name() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

#endif // WAYPOINT_H
