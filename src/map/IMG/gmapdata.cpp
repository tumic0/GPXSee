#include <QXmlStreamReader>
#include <QDir>
#include "vectortile.h"
#include "gmapdata.h"

using namespace IMG;

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

void GMAPData::subProduct(QXmlStreamReader &reader, QString &dataDir,
  QString &baseMap)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Directory"))
			dataDir = reader.readElementText();
		else if (reader.name() == QLatin1String("BaseMap"))
			baseMap = reader.readElementText();
		else
			reader.skipCurrentElement();
	}
}

void GMAPData::mapProduct(QXmlStreamReader &reader, QString &dataDir,
  QString &typFile, QString &baseMap)
{
	while (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("Name"))
			_name = reader.readElementText();
		else if (reader.name() == QLatin1String("TYP"))
			typFile = reader.readElementText();
		else if (reader.name() == QLatin1String("SubProduct"))
			subProduct(reader, dataDir, baseMap);
		else
			reader.skipCurrentElement();
	}
}

bool GMAPData::readXML(const QString &path, QString &dataDir, QString &typFile,
  QString &baseMap)
{
	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text))
		return false;

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement()) {
		if (reader.name() == QLatin1String("MapProduct"))
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

bool GMAPData::loadTile(const QDir &dir, bool baseMap)
{
	VectorTile *tile = new VectorTile();

	QFileInfoList ml = dir.entryInfoList(QDir::Files);
	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		SubFile::Type tt = tileType(fi.suffix());
		if (VectorTile::isTileFile(tt)) {
			_files.append(new QString(fi.absoluteFilePath()));
			tile->addFile(_files.last(), tt);
		}
	}

	if (!tile->init()) {
		qWarning("%s: Invalid map tile", qPrintable(dir.path()));
		delete tile;
		return false;
	}
	if (baseMap)
		tile->markAsBasemap();

	double min[2], max[2];
	min[0] = tile->bounds().left();
	min[1] = tile->bounds().bottom();
	max[0] = tile->bounds().right();
	max[1] = tile->bounds().top();
	_tileTree.Insert(min, max, tile);

	_bounds |= tile->bounds();
	if (tile->zooms().min() < _zooms.min())
		_zooms.setMin(tile->zooms().min());

	return true;
}

GMAPData::GMAPData(const QString &fileName) : _fileName(fileName)
{
	QString dataDirPath, typFilePath, baseMapPath;
	if (!readXML(fileName, dataDirPath, typFilePath, baseMapPath))
		return;

	QDir baseDir(QFileInfo(fileName).absoluteDir());
	if (!baseDir.exists(dataDirPath)) {
		_errorString = "Missing/invalid map data directory";
		return;
	}
	QDir dataDir(baseDir.filePath(dataDirPath));
	QFileInfoList ml = dataDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);


	QFileInfo baseMap(dataDir.filePath(baseMapPath));
	_baseMap = !baseMapPath.isEmpty() && baseMap.exists();

	for (int i = 0; i < ml.size(); i++) {
		const QFileInfo &fi = ml.at(i);
		if (fi.isDir())
			loadTile(QDir(fi.absoluteFilePath()),
			  fi.absoluteFilePath() == baseMap.absoluteFilePath());
	}

	if (baseDir.exists(typFilePath)) {
		_files.append(new QString(baseDir.filePath(typFilePath)));
		_typ = new SubFile(_files.last());
	}

	if (!_tileTree.Count())
		_errorString = "No usable map tile found";
	else
		_valid = true;
}

GMAPData::~GMAPData()
{
	qDeleteAll(_files);
}

bool GMAPData::isGMAP(const QString &path)
{
	QFile file(path);

	if (!file.open(QFile::ReadOnly | QFile::Text))
		return false;

	QXmlStreamReader reader(&file);
	if (reader.readNextStartElement()
	  && reader.name() == QLatin1String("MapProduct"))
		return true;

	return false;
}
