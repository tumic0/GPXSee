#include <QBrush>
#include "trackitemsmodel.h"
#include "format.h"

TrackItemsModel::TrackItemsModel(GeoItems &geoItems, QObject *parent)
	: QAbstractTableModel(parent)
{
	QObject::connect(&geoItems, SIGNAL(addedTrackItem(Track,TrackItem*)),
					 this, SLOT(addTrackItem(Track,TrackItem*)));
	QObject::connect(&geoItems, SIGNAL(cleared()),
					 this, SLOT(clear()));
	QObject::connect(&geoItems, SIGNAL(unitsChanged(Units)),
					 this, SLOT(setUnits(Units)));
}

int TrackItemsModel::rowCount(const QModelIndex &parent) const  {
	Q_UNUSED(parent);
	return _trackItems.count();
}

int TrackItemsModel::columnCount(const QModelIndex &parent) const {
	Q_UNUSED(parent);
	return N_COLUMNS;
}

QVariant TrackItemsModel::data(const QModelIndex &index, int role) const {
	if (role == Qt::DisplayRole) {
		const Track &t = _trackItems.at(index.row())->track();
		switch(index.column()) {
		case TRACK_NAME:
			return t.name();
		case TRACK_DISTANCE:
			return Format::distance(t.distance(), _units);
		}
	} else if(role == Qt::BackgroundRole) {
		QBrush color = _trackItems.at(index.row())->color();
		return color;
	}

	return QVariant();
}

QVariant TrackItemsModel::headerData(int section, Qt::Orientation orientation, int role) const {
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) {
			switch (section) {
			case TRACK_NAME:
				return tr("Name");
			case TRACK_DISTANCE:
				return tr("Distance");
			}
		}
	}

	return QVariant();
}

void TrackItemsModel::addTrackItem(const Track &t, TrackItem *trackItem)
{
	Q_UNUSED(t)

	QAbstractTableModel::beginInsertRows(QModelIndex(), rowCount(), rowCount());
	_trackItems.append(trackItem);
	QAbstractTableModel::endInsertRows();
}

void TrackItemsModel::setUnits(Units units)
{
	_units = units;

	QModelIndex startColumn = this->index(0, TRACK_DISTANCE);
	QModelIndex endColumn = this->index(rowCount(), TRACK_DISTANCE);
	//QVector<int> roles;
	//roles.append(Qt::DisplayRole);
	//emit QAbstractItemModel::dataChanged(startColumn, endColumn, roles);
	emit QAbstractItemModel::dataChanged(startColumn, endColumn, roles);
}

void TrackItemsModel::clear()
{
	QAbstractTableModel::beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	_trackItems.clear();
	QAbstractTableModel::endRemoveRows();
}
