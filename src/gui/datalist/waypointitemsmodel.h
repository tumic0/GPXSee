#ifndef WAYPOINTITEMSMODEL_H
#define WAYPOINTITEMSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "geoitems/geoitems.h"
#include "geoitems/waypointitem.h"

class WaypointItemsModel : public QAbstractTableModel{
	Q_OBJECT

	enum Columns {
		WAYPOINT_NAME,
		N_COLUMNS
	};
public:
	WaypointItemsModel(GeoItems &geoItems, QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private slots:
	void addWaypointItem(const Waypoint &w, WaypointItem *waypointItem);
	void clear();

private:
	QList<WaypointItem*> _waypointItems;
};


#endif // WAYPOINTITEMSMODEL_H
