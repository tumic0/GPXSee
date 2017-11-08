#ifndef POWERGRAPH_H
#define POWERGRAPH_H

#include "graphtab.h"

class PowerGraph : public GraphTab
{
	Q_OBJECT

public:
	PowerGraph(GeoItems &geoItems, QWidget *parent = 0);

	QString label() const {return tr("Power");}
	void clear();
	void showTracks(bool show);

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

#endif // POWERGRAPH_H
