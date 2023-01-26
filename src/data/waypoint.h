#ifndef WAYPOINT_H
#define WAYPOINT_H

#include <QString>
#include <QDateTime>
#include <QHash>
#include <QVector>
#include <QPixmap>
#include <QDebug>
#include "common/hash.h"
#include "common/coordinates.h"
#include "link.h"
#include "style.h"

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
	const QString &address() const {return _address;}
	const QString &phone() const {return _phone;}
	const QString &symbol() const {return _symbol;}
	const QVector<QString> &images() const {return _images;}
	const QVector<Link> &links() const {return _links;}
	const QDateTime &timestamp() const {return _timestamp;}
	qreal elevation() const {return _elevation;}
	const PointStyle &style() const {return _style;}

	QPair<qreal, qreal> elevations() const;

	void setCoordinates(const Coordinates &coordinates)
	  {_coordinates = coordinates;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &description)
	  {_description = description;}
	void setComment(const QString &comment) {_comment = comment;}
	void setAddress(const QString &address) {_address = address;}
	void setPhone(const QString &phone) {_phone = phone;}
	void setSymbol(const QString &symbol) {_symbol = symbol;}
	void setTimestamp(const QDateTime &timestamp) {_timestamp = timestamp;}
	void setElevation(qreal elevation) {_elevation = elevation;}
	void addImage(const QString &path) {_images.append(path);}
	void addLink(const Link &link) {_links.append(link);}
	void setStyle(const PointStyle &style) {_style = style;}

	bool hasElevation() const {return !std::isnan(_elevation);}

	bool operator==(const Waypoint &other) const
	  {return this->_name == other._name
	  && this->_coordinates == other._coordinates;}

	static void useDEM(bool use) {_useDEM = use;}
	static void showSecondaryElevation(bool show)
	  {_show2ndElevation = show;}

	static const QPixmap *symbolIcon(const QString &symbol);
	static void loadSymbolIcons(const QString &dir);

private:
	Coordinates _coordinates;
	QString _name;
	QString _description;
	QString _comment;
	QString _address;
	QString _phone;
	QString _symbol;
	QVector<QString> _images;
	QVector<Link> _links;
	QDateTime _timestamp;
	qreal _elevation;
	PointStyle _style;

	static bool _useDEM;
	static bool _show2ndElevation;
	static QHash<QString, QPixmap> _symbolIcons;
};

inline HASH_T qHash(const Waypoint &key)
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
