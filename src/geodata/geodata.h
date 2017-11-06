#ifndef GEODATA_H
#define GEODATA_H

#include <QVector>
#include <QString>

template<class T>
class GeoData : public QVector<T>
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

#endif // GEODATA_H
