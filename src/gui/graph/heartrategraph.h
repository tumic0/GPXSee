#ifndef HEARTRATEGRAPH_H
#define HEARTRATEGRAPH_H

#include "graphtab.h"

class HeartRateGraph : public GraphTab
{
	Q_OBJECT

public:
	HeartRateGraph(GeoItems &geoItems, QWidget *parent = 0);

	QString label() const {return tr("Heart rate");}
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

#endif // HEARTRATEGRAPH_H
