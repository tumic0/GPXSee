#include <QBrush>
#include "trackitemsmodel.h"
#include "format.h"

void TrackItemsModel::append(Track * const &t) {
	QAbstractTableModel::beginInsertRows(QModelIndex(), rowCount(), rowCount());
	QList<Track*>::append(t);
	QAbstractTableModel::endInsertRows();
}

void TrackItemsModel::clear() {
	QAbstractTableModel::beginRemoveRows(QModelIndex(), 0, rowCount()-1);
	QList::clear();
	QAbstractTableModel::endRemoveRows();
}

int TrackItemsModel::rowCount(const QModelIndex &parent) const  {
	Q_UNUSED(parent);
	return QList::count();
}

int TrackItemsModel::columnCount(const QModelIndex &parent) const {
	Q_UNUSED(parent);
	return N_COLUMNS;
}

QVariant TrackItemsModel::data(const QModelIndex &index, int role) const {
	if (role == Qt::DisplayRole) {
		Track* t = at(index.row());
		switch(index.column()) {
		case TRACK_NAME:
			return t->name();
		case TRACK_DISTANCE:
			// TODO: Use global preferences for units
			return Format::distance(t->distance(), Units::Metric);
		}
	} else if(role == Qt::BackgroundRole) {
		if (index.row()%2 == 0) {
			QBrush greyBrush(Qt::lightGray);
			return greyBrush;
		}
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
