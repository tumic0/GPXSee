#include "datalistview.h"
#include "data.h"
#include <QHeaderView>

DataListView::DataListView(Data& data, QWidget *parent)
    : QTabWidget(parent)
    , _tracksTableView(new QTableView(0))
    , _routesTableView(new QTableView(0))
    , _waypointTableView(new QTableView(0))
{
	_tracksTableView->setModel(data.trackItemsModel());
	_routesTableView->setModel(data.routeItemsModel());
	_waypointTableView->setModel(data.waypointItemsModel());

	_tracksTableView->horizontalHeader()->setStretchLastSection(true);
	_routesTableView->horizontalHeader()->setStretchLastSection(true);
	_waypointTableView->horizontalHeader()->setStretchLastSection(true);

	addTab(_tracksTableView, tr("Tracks"));
	addTab(_routesTableView, tr("Routes"));
	addTab(_waypointTableView, tr("Waypoints"));
}

