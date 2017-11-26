#ifndef TRACKDATA_H
#define TRACKDATA_H

#include <QVector>
#include <QString>
#include "trackpoint.h"

class TrackData : public QVector<Trackpoint>
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
