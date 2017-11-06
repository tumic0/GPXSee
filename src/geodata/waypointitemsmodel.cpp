#include <QBrush>
#include "waypointitemsmodel.h"

void WaypointItemsModel::append(const Waypoint &w)
{
	QAbstractTableModel::beginInsertRows(QModelIndex(), rowCount(), rowCount());
	QList<Waypoint>::append(w);
	QAbstractTableModel::endInsertRows();
}

void WaypointItemsModel::append(const QList<Waypoint> &w)
{
	QAbstractTableModel::beginInsertRows(QModelIndex(), rowCount(), rowCount() + w.count() - 1);
	QList<Waypoint>::append(w);
	QAbstractTableModel::endInsertRows();
}

void WaypointItemsModel::clear()
{
	QAbstractTableModel::beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	QList::clear();
	QAbstractTableModel::endRemoveRows();
}

int WaypointItemsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return QList::count();
}

int WaypointItemsModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return N_COLUMNS;
}

QVariant WaypointItemsModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		Waypoint w = at(index.row());
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
