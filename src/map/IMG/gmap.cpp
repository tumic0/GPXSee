#include <QXmlStreamReader>
#include <QDir>
#include "vectortile.h"
#include "gmap.h"


static SubFile::Type tileType(const QString &suffix)
{
	if (!suffix.compare("TRE"))
		return SubFile::TRE;
	else if (!suffix.compare("RGN"))
		return SubFile::RGN;
	else if (!suffix.compare("LBL"))
		return SubFile::LBL;
	else if (!suffix.compare("TYP"))
		return SubFile::TYP;
	else if (!suffix.compare("GMP"))
		return SubFile::GMP;
	else if (!suffix.compare("NET"))
		return SubFile::NET;
	else
		return SubFile::Unknown;
}

void GMAP::subProduct(QXmlStreamReader &reader, QString &dataDir,
  QString &baseMap)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "Directory")
			dataDir = reader.readElementText();
		else if (reader.name() == "BaseMap")
			baseMap = reader.readElementText();
		else
			reader.skipCurrentElement();
	}
}

void GMAP::mapProduct(QXmlStreamReader &reader, QString &dataDir,
  QString &typFile, QString &baseMap)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == "Name")
			_name = reader.readElementText();
		else if (reader.name() == "TYP")
			typFile = reader.readElementText();
		else if (reader.name() == "SubProduct")
			subProduct(reader, dataDir, baseMap);
		else
			reader.skipCurrentElement();
	}
}

bool GMAP::readXML(const QString &path, QString &dataDir, QString &typFile,
  QString &baseMap)
{
	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text))
		return false;

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == "MapProduct")
			mapProduct(reader, dataDir, typFile, baseMap);
		else
			reader.raiseError("Not a GMAP XML file");
	}
	if (reader.error()) {
		_errorString = QString("%1: %2").arg(reader.lineNumber())
		  .arg(reader.errorString());
		return false;
	}

	return true;
}

bool GMAP::loadTile(const QDir &dir, quint16 &id)
{
	VectorTile *tile = new VectorTile();

	QFileInfoList ml = dir.entryInfoList(QDir::Files);
	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		SubFile::Type type = tileType(fi.suffix());
		if (VectorTile::isTileFile(type)) {
			SubFile *file = tile->addFile(fi.absoluteFilePath(), type);
			file->setId(id++);
		}
	}

	if (!tile->init()) {
		qWarning("%s: Invalid map tile", qPrintable(dir.path()));
		delete tile;
		return false;
	}

	double min[2], max[2];
	min[0] = tile->bounds().left();
	min[1] = tile->bounds().bottom();
	max[0] = tile->bounds().right();
	max[1] = tile->bounds().top();
	_tileTree.Insert(min, max, tile);

	_bounds |= tile->bounds();

	return true;
}

GMAP::GMAP(const QString &fileName) : _fileName(fileName)
{
	QString dataDirPath, typFile, baseMap;
	if (!readXML(fileName, dataDirPath, typFile, baseMap))
		return;

	QDir baseDir(QFileInfo(fileName).absoluteDir());
	if (!baseDir.exists(dataDirPath)) {
		_errorString = "Missing/invalid map data directory";
		return;
	}
	QDir dataDir(baseDir.filePath(dataDirPath));
	QFileInfoList ml = dataDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

	quint16 id = 0;
	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		if (fi.isDir() && fi.baseName() != baseMap)
			loadTile(QDir(fi.absoluteFilePath()), id);
	}

	if (baseDir.exists(typFile))
		_typ = new SubFile(baseDir.filePath(typFile));

	if (!_tileTree.Count())
		_errorString = "No usable map tile found";
	else
		_valid = true;
}

bool GMAP::isGMAP(const QString &path)
{
	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text))
		return false;

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement() && reader.name() == "MapProduct")
		return true;

	return false;
}
