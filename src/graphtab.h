#ifndef GRAPHTAB_H
#define GRAPHTAB_H

#include <QList>
#include "graphview.h"
#include "units.h"

class Data;
class PathItem;

class GraphTab : public GraphView
{
	Q_OBJECT

public:
	GraphTab(QWidget *parent = 0) : GraphView(parent)
	  {setFrameShape(QFrame::NoFrame);}

	virtual QString label() const = 0;
	virtual void loadData(const Data &data, const QList<PathItem *> &paths) = 0;
	virtual void clear() = 0;
	virtual void setUnits(enum Units units) = 0;
	virtual void showTracks(bool show) = 0;
	virtual void showRoutes(bool show) = 0;
};

#endif // GRAPHTAB_H
