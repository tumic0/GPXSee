#ifndef PCS_H
#define PCS_H

#include <QDebug>
#include <QList>
#include "datum.h"
#include "projection.h"

class PCS
{
public:
	PCS() {}
	PCS(const Datum &datum, const Projection::Method &method,
	  const Projection::Setup &setup)
		: _datum(datum), _method(method), _setup(setup) {}
	PCS(int id);
	PCS(int gcs, int proj);

	const Datum &datum() const {return _datum;}
	const Projection::Method &method() const {return _method;}
	const Projection::Setup &setup() const {return _setup;}

	bool isNull() const
	  {return (_datum.isNull() && _method.isNull());}
	bool isValid() const
	  {return !(_datum.isNull() || _method.isNull());}

	static bool loadList(const QString &path);
	static const QString &errorString() {return _errorString;}
	static int errorLine() {return _errorLine;}

private:
	class Entry;

	static void error(const QString &str);

	Datum _datum;
	Projection::Method _method;
	Projection::Setup _setup;

	static QList<Entry> _pcss;
	static QString _errorString;
	static int _errorLine;
};

QDebug operator<<(QDebug dbg, const PCS &pcs);

#endif // PCS_H
