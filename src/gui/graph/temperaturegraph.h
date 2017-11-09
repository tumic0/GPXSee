#ifndef TEMPERATUREGRAPH_H
#define TEMPERATUREGRAPH_H

#include "graphtab.h"

class TemperatureGraph : public GraphTab
{
	Q_OBJECT

public:
	TemperatureGraph(GeoItems &geoItems, QWidget *parent = 0);

	QString label() const {return tr("Temperature");}
	void clear();
	void showTracks(bool show);

public slots:
	virtual void addTrack(const Track &track, TrackItem *item);
	virtual void addRoute(const Route &track, RouteItem *item) {
		Q_UNUSED(track)
		Q_UNUSED(item)
	}

private slots:
	virtual void setUnits(enum Units units);

private:
	qreal avg() const;
	qreal min() const {return bounds().top();}
	qreal max() const {return bounds().bottom();}
	void setYUnits(Units units);
	void setInfo();

	QList<QPointF> _avg;

	bool _showTracks;
};

#endif // TEMPERATUREGRAPH_H
