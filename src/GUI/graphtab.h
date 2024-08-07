#ifndef GRAPHTAB_H
#define GRAPHTAB_H

#include <QtGlobal>
#include <QList>
#include "graphview.h"
#include "units.h"
#include "timetype.h"

class Map;
class Data;
class GraphItem;

class GraphTab : public GraphView
{
	Q_OBJECT

public:
	GraphTab(QWidget *parent = 0) : GraphView(parent)
	{
#if defined(Q_OS_WIN32) || defined(Q_OS_MAC)
		setFrameShape(QFrame::NoFrame);
#endif // Q_OS_WIN32 || Q_OS_MAC
	}
	virtual ~GraphTab() {}

	virtual QString label() const = 0;
	virtual QList<GraphItem*> loadData(const Data &data, Map *map) = 0;
	virtual void clear() {GraphView::clear();}
	virtual void setUnits(enum Units units) {GraphView::setUnits(units);}
	virtual void setGraphType(GraphType type) {GraphView::setGraphType(type);}
	virtual void setTimeType(enum TimeType type) {Q_UNUSED(type)}
	virtual void showTracks(bool show) {Q_UNUSED(show)}
	virtual void showRoutes(bool show) {Q_UNUSED(show)}
};

#endif // GRAPHTAB_H
