#ifndef TRACKITEMSMODEL_H
#define TRACKITEMSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "geoitems/geoitems.h"
#include "geoitems/trackitem.h"
#include "units.h"

class TrackItemsModel : public QAbstractTableModel {
	Q_OBJECT

	enum Columns {
		TRACK_NAME,
		TRACK_DISTANCE,
		N_COLUMNS
	};
public:
	TrackItemsModel(GeoItems &geoItems, QObject *parent = 0);

	int rowCount(const QModelIndex &parent = QModelIndex()) const;

	int columnCount(const QModelIndex &parent = QModelIndex()) const;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private slots:
	void addTrackItem(const Track &t, TrackItem *trackItem);
	void setUnits(Units units);
	void clear();

private:
	QList<TrackItem*> _trackItems;
	Units _units;

};

#endif // TRACKITEMSMODEL_H
