#include <QHeaderView>

#include "datalistview.h"
#include "trackitemsmodel.h"
#include "routeitemsmodel.h"
#include "waypointitemsmodel.h"

DataListView::DataListView(GeoItems& geoItems, QWidget *parent)
    : QTabWidget(parent)
	, _tracksTableView(new QTableView(this))
	, _routesTableView(new QTableView(this))
	, _waypointTableView(new QTableView(this))
{
	_tracksTableView->setModel(new TrackItemsModel(geoItems, _tracksTableView));
	_routesTableView->setModel(new RouteItemsModel(geoItems, _routesTableView));
	_waypointTableView->setModel(new WaypointItemsModel(geoItems, _waypointTableView));

	_tracksTableView->horizontalHeader()->setStretchLastSection(true);
	_routesTableView->horizontalHeader()->setStretchLastSection(true);
	_waypointTableView->horizontalHeader()->setStretchLastSection(true);

	addTab(_tracksTableView, tr("Tracks"));
	addTab(_routesTableView, tr("Routes"));
	addTab(_waypointTableView, tr("Waypoints"));
}

