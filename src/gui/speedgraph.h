#ifndef SPEEDGRAPH_H
#define SPEEDGRAPH_H

#include <QList>
#include "graphtab.h"

class SpeedGraph : public GraphTab
{
	Q_OBJECT

public:
	SpeedGraph(GeoItems &geoItems, QWidget *parent = 0);

	QString label() const {return tr("Speed");}
	void clear();
	void setUnits(Units units);
	void setTimeType(TimeType type);
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
	void setYUnits(Units units);
	void setInfo();

	QList<QPointF> _avg;
	QList<QPointF> _mavg;

	enum TimeType _timeType;
	bool _showTracks;
};

#endif // SPEEDGRAPH_H
