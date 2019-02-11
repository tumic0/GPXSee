#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <QList>
#include <QVector>
#include <QString>
#include "trackpoint.h"

typedef QVector<Trackpoint> SegmentData;

class TrackData : public QList<SegmentData>
{
public:
	const QString& name() const {return _name;}
	const QString& description() const {return _desc;}
	void setName(const QString &name) {_name = name;}
	void setDescription(const QString &desc) {_desc = desc;}

private:
	QString _name;
	QString _desc;
};

#endif // TRACKDATA_H
