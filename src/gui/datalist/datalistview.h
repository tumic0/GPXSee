#ifndef DATALISTVIEW_H
#define DATALISTVIEW_H

#include <QTabWidget>
#include <QTableView>
#include "geoitems/geoitems.h"

class Data;

class DataListView : public QTabWidget
{
	Q_OBJECT
public:
	explicit DataListView(GeoItems &geoItems, QWidget *parent = 0);
signals:

public slots:

private:
	QTableView *_tracksTableView;
	QTableView *_routesTableView;
	QTableView *_waypointTableView;
};

#endif // DATALISTVIEW_H
