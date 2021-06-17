#ifndef WLDFILE_H
#define WLDFILE_H

#include "transform.h"

class WLDFile
{
public:
	WLDFile(const QString &fileName);

	const Transform &transform() const {return _transform;}
	const QString &errorString() const {return _errorString;}

private:
	Transform _transform;
	QString _errorString;
};

#endif // WLDFILE_H
