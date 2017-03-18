#include <QFile>
#include <QFileInfo>
#include "onlinemap.h"
#include "maplist.h"


QList<Map*> MapList::load(const QString &fileName, QObject *parent)
{
	QList<Map*> maps;
	QFileInfo fi(fileName);

	if (!fi.exists())
		return maps;

	QFile file(fileName);

	if (!file.open(QFile::ReadOnly | QFile::Text)) {
		qWarning("Error opening map list file: %s: %s\n",
		  qPrintable(fileName), qPrintable(file.errorString()));
		return maps;
	}

	int ln = 0;
	while (!file.atEnd()) {
		ln++;
		QByteArray line = file.readLine();
		QList<QByteArray> list = line.split('\t');
		if (list.size() != 2) {
			qWarning("Invalid map list entry on line %d\n", ln);
			continue;
		}

		QByteArray ba1 = list[0].trimmed();
		QByteArray ba2 = list[1].trimmed();

		maps.append(new OnlineMap(QString::fromUtf8(ba1.data(), ba1.size()),
		  QString::fromLatin1(ba2.data(), ba2.size()), parent));
	}

	return maps;
}
