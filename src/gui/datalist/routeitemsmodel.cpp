#include <QBrush>
#include "routeitemsmodel.h"
#include "format.h"

RouteItemsModel::RouteItemsModel(GeoItems &geoItems, QObject *parent)
	: QAbstractTableModel(parent)
{
	QObject::connect(&geoItems, SIGNAL(addedRouteItem(Route,RouteItem*)),
					 this, SLOT(addRouteItem(Route,RouteItem*)));
	QObject::connect(&geoItems, SIGNAL(cleared()),
					 this, SLOT(clear()));
	QObject::connect(&geoItems, SIGNAL(unitsChanged(Units)),
					 this, SLOT(setUnits(Units)));
}

int RouteItemsModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return _routeItems.count();
}

int RouteItemsModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return N_COLUMNS;
}

QVariant RouteItemsModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::DisplayRole) {
		const Route &r = _routeItems.at(index.row())->route();
		switch(index.column()) {
		case ROUTE_NAME:
			return r.name();
		case ROUTE_DISTANCE:
			return Format::distance(r.distance(), _units);
		}
	} else if(role == Qt::BackgroundRole) {
		QBrush color = _routeItems.at(index.row())->color();
		return color;
	}

	return QVariant();
}

QVariant RouteItemsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case ROUTE_NAME:
				return tr("Name");
			case ROUTE_DISTANCE:
				return tr("Distance");
			}
		}
	}

	return QVariant();
}

void RouteItemsModel::addRouteItem(const Route &r, RouteItem *routeItem)
{
	Q_UNUSED(r)

	QAbstractTableModel::beginInsertRows(QModelIndex(), rowCount(), rowCount());
	_routeItems.append(routeItem);
	QAbstractTableModel::endInsertRows();
}

void RouteItemsModel::setUnits(Units units)
{
	_units = units;

	QModelIndex startColumn = this->index(0, ROUTE_DISTANCE);
	QModelIndex endColumn = this->index(rowCount(), ROUTE_DISTANCE);
	//QVector<int> roles;
	//roles.append(Qt::DisplayRole);
	//emit QAbstractItemModel::dataChanged(startColumn, endColumn, roles);
	emit QAbstractItemModel::dataChanged(startColumn, endColumn);
}

void RouteItemsModel::clear()
{
	QAbstractTableModel::beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	_routeItems.clear();
	QAbstractTableModel::endRemoveRows();
}
