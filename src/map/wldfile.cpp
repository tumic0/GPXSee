#include <QFile>
#include "wldfile.h"

WLDFile::WLDFile(const QString &fileName)
{
	bool ok;
	double val[6];
	QFile file(fileName);

	if (!file.open(QIODevice::ReadOnly)) {
		_errorString = file.errorString();
		return;
	}

	for (int i = 0; i < 6; i++) {
		QByteArray line(file.readLine(128).trimmed());
		val[i] = line.toDouble(&ok);
		if (!ok) {
			_errorString = QString("Parse error on line %1").arg(i+1);
			return;
		}
	}
	double matrix[16] = {val[0], val[1], 0, val[4], val[2], val[3], 0, val[5]};

	_transform = Transform(matrix);

	if (!_transform.isValid())
		_errorString = _transform.errorString();
}
