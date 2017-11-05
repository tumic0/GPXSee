#ifndef WAYPOINTITEMSMODEL_H
#define WAYPOINTITEMSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "waypoint.h"

class WaypointItemsModel : public QAbstractTableModel, public QList<Waypoint> {
	Q_OBJECT

	enum Columns {
		WAYPOINT_NAME,
		N_COLUMNS
	};
public:
	virtual void append(Waypoint const &w);
	virtual void append(const QList<Waypoint> &w);

	virtual void clear();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
};


#endif // WAYPOINTITEMSMODEL_H
