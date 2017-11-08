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
	RouteItemsModel(GeoItems &geoItems, QObject *parent = Q_NULLPTR);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private slots:
	void addRouteItem(const Route &r, RouteItem *routeItem);
	void setUnits(Units units);
	void clear();

private:
	QList<RouteItem*> _routeItems;
	Units _units;
};

#endif // ROUTEITEMSMODEL_H
