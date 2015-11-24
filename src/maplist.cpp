#include <QFile>
#include "maplist.h"

#include <QDebug>

QList<Map*> MapList::load(const QString &fileName)
{
	QFile file(fileName);
	QList<Map*> mapList;
	int ln = 1;


	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		fprintf(stderr, "Error opening map list file: %s: %s\n",
		  qPrintable(fileName), qPrintable(file.errorString()));
		return mapList;
	}

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split('\t');
		if (list.size() != 2) {
			fprintf(stderr, "Invalid map list entry on line %d\n", ln);
			continue;
		}

		QByteArray ba1 = list[0].trimmed();
		QByteArray ba2 = list[1].trimmed();

		mapList.append(new Map(QString::fromUtf8(ba1.data(), ba1.size()),
		  QString::fromLatin1(ba2.data(), ba2.size())));

		ln++;
	}

	return mapList;
}
