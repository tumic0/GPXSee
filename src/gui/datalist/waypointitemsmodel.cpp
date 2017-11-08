#include <QBrush>
#include "waypointitemsmodel.h"

WaypointItemsModel::WaypointItemsModel(GeoItems &geoItems, QObject *parent)
	: QAbstractTableModel(parent)
{
	QObject::connect(&geoItems, SIGNAL(addedWaypointItem(Waypoint,WaypointItem*)),
					 this, SLOT(addWaypointItem(Waypoint,WaypointItem*)));
	QObject::connect(&geoItems, SIGNAL(cleared()),
					 this, SLOT(clear()));
}

int WaypointItemsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return _waypointItems.count();
}

int WaypointItemsModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return N_COLUMNS;
}

QVariant WaypointItemsModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		const Waypoint &w = _waypointItems.at(index.row())->waypoint();
		switch(index.column()) {
		case WAYPOINT_NAME:
			return w.name();
		}
	} else if(role == Qt::BackgroundRole) {
		if (index.row()%2 == 0) {
			QBrush greyBrush(Qt::lightGray);
			return greyBrush;
		}
	}
	return QVariant();
}

QVariant WaypointItemsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case WAYPOINT_NAME:
				return tr("Name");
			}
		}
	}

	return QVariant();
}

void WaypointItemsModel::addWaypointItem(const Waypoint &w, WaypointItem *waypointItem)
{
	QAbstractTableModel::beginInsertRows(QModelIndex(), rowCount(), rowCount());
	_waypointItems.append(waypointItem);
	QAbstractTableModel::endInsertRows();
}

void WaypointItemsModel::clear()
{
	QAbstractTableModel::beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	_waypointItems.clear();
	QAbstractTableModel::endRemoveRows();
}
