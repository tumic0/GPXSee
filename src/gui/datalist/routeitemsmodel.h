#ifndef ROUTEITEMSMODEL_H
#define ROUTEITEMSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "geoitems/geoitems.h"
#include "geoitems/routeitem.h"
#include "units.h"

class RouteItemsModel : public QAbstractTableModel {
	Q_OBJECT

	enum Columns {
		ROUTE_NAME,
		ROUTE_DISTANCE,
		N_COLUMNS
	};
public:
	RouteItemsModel(GeoItems &geoItems, QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private slots:
	void addRouteItem(const Route &r, RouteItem *routeItem);
	void setUnits(Units units);
	void clear();

private:
	QList<RouteItem*> _routeItems;
	Units _units;
};

#endif // ROUTEITEMSMODEL_H
