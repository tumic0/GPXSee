#include "routeitemsmodel.h"
#include "format.h"

void RouteItemsModel::append(Route * const &r)
{
	QAbstractTableModel::beginInsertRows(QModelIndex(), rowCount(), rowCount());
	QList<Route*>::append(r);
	QAbstractTableModel::endInsertRows();
}

void RouteItemsModel::clear()
{
	QAbstractTableModel::beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	QList::clear();
	QAbstractTableModel::endRemoveRows();
}

int RouteItemsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return QList::count();
}

int RouteItemsModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return N_COLUMNS;
}

QVariant RouteItemsModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		Route* r = at(index.row());
		switch(index.column()) {
		case ROUTE_NAME:
			return r->name();
		case ROUTE_DISTANCE:
			// TODO: Use global preferences for units
			return Format::distance(r->distance(), Units::Metric);
		}
	}
	return QVariant();
}
