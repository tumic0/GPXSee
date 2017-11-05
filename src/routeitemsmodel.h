#ifndef ROUTEITEMSMODEL_H
#define ROUTEITEMSMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "route.h"

class RouteItemsModel : public QAbstractTableModel, public QList<Route*> {
	Q_OBJECT

	enum Columns {
		ROUTE_NAME,
		ROUTE_DISTANCE,
		N_COLUMNS
	};
public:
	virtual void append(Route * const &r);

	virtual void clear();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;

	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
};

#endif // ROUTEITEMSMODEL_H
