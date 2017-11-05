#ifndef TRACKITEMSMODEL_H
#define TRACKITEMSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "track.h"

class TrackItemsModel : public QAbstractTableModel, public QList<Track*> {
	Q_OBJECT

	enum Columns {
		TRACK_NAME,
		TRACK_DISTANCE,
		N_COLUMNS
	};
public:
	virtual void append(Track * const &t);

	virtual void clear();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // TRACKITEMSMODEL_H
