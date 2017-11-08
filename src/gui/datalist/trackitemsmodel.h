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
	TrackItemsModel(GeoItems &geoItems, QObject *parent = Q_NULLPTR);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private slots:
	void addTrackItem(const Track &t, TrackItem *trackItem);
	void setUnits(Units units);
	void clear();

private:
	QList<TrackItem*> _trackItems;
	Units _units;

};

#endif // TRACKITEMSMODEL_H
