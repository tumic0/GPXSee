#ifndef DATALISTVIEW_H
#define DATALISTVIEW_H

#include <QTabWidget>
#include <QTableView>
#include "track.h"

class Data;

class DataListView : public QTabWidget
{
	Q_OBJECT
public:
	explicit DataListView(Data& data, QWidget *parent = nullptr);
signals:

public slots:

private:
	QTableView *_tracksTableView;
	QTableView *_routesTableView;
	QTableView *_waypointTableView;
};

#endif // DATALISTVIEW_H
