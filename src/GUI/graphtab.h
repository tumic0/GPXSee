#ifndef GRAPHTAB_H
#define GRAPHTAB_H

#include <QList>
#include "graphview.h"
#include "units.h"
#include "timetype.h"

class Data;
class PathItem;

class GraphTab : public GraphView
{
	Q_OBJECT

public:
	GraphTab(QWidget *parent = 0) : GraphView(parent)
	  {setFrameShape(QFrame::NoFrame);}
	virtual ~GraphTab() {}

	virtual QString label() const = 0;
	virtual void loadData(const Data &data, const QList<PathItem *> &paths) = 0;
	virtual void clear() {GraphView::clear();}
	virtual void setUnits(enum Units units) {GraphView::setUnits(units);}
	virtual void setGraphType(GraphType type) {GraphView::setGraphType(type);}
	virtual void setTimeType(enum TimeType type) {Q_UNUSED(type)}
	virtual void showTracks(bool show) {Q_UNUSED(show)}
	virtual void showRoutes(bool show) {Q_UNUSED(show)}
};

#endif // GRAPHTAB_H
