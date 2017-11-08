#ifndef CADENCEGRAPH_H
#define CADENCEGRAPH_H

#include "graphtab.h"

class CadenceGraph : public GraphTab
{
	Q_OBJECT

public:
	CadenceGraph(GeoItems &geoItems, QWidget *parent = 0);

	QString label() const {return tr("Cadence");}
	void clear();
	void showTracks(bool show);
	void showRoutes(bool show) {Q_UNUSED(show);}

public slots:
	virtual void addTrack(const Track &track, TrackItem *item);
	virtual void addRoute(const Route &track, RouteItem *item) {
		Q_UNUSED(track)
		Q_UNUSED(item)
	}

private:
	qreal avg() const;
	qreal max() const {return bounds().bottom();}
	void setInfo();

	QList<QPointF> _avg;

	bool _showTracks;
};

#endif // CADENCEGRAPH_H
