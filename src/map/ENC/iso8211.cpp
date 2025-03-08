#include <QFile>
#include <QRegularExpression>
#include "common/util.h"
#include "iso8211.h"

using namespace ENC;

#define UINT16(x) \
  (((quint16)*(const uchar*)(x)) \
  | ((quint16)(*((const uchar*)(x) + 1)) << 8))

#define INT32(x) ((qint32)UINT32(x))
#define INT16(x) ((qint16)UINT16(x))

struct DR {
	char RecordLength[5];
	char InterchangeLevel;
	char LeaderID;
	char ExtensionFlag;
	char Version;
	char ApplicationIndicator;
	char FieldControlLength[2];
	char BaseAddress[5];
	char ExtCharSetIndicator[3];
	char FieldLengthSize;
	char FieldPosSize;
	char Reserved;
	char FieldTagSize;
};


const QVariant *ISO8211::Field::data(const QByteArray &name, int idx) const
{
	const QVector<QVariant> &v = _data.at(idx);

	for (int i = 0; i < _subFields.size(); i++)
		if (_subFields.at(i) == name)
			return &v.at(i);

	return 0;
}

bool ISO8211::Field::subfield(const char *name, int *val, int idx) const
{
	bool ok;

	const QVariant *v = data(name, idx);
	if (!v)
		return false;
	*val = v->toInt(&ok);

	return ok;
}

bool ISO8211::Field::subfield(const char *name, uint *val, int idx) const
{
	bool ok;

	const QVariant *v = data(name, idx);
	if (!v)
		return false;
	*val = v->toUInt(&ok);

	return ok;
}

bool ISO8211::Field::subfield(const char *name, QByteArray *val, int idx) const
{
	const QVariant *v = data(name, idx);
	if (!v)
		return false;
	*val = v->toByteArray();

	return true;
}

ISO8211::SubFieldDefinition ISO8211::fieldType(const QString &str, int cnt)
{
	if (str == "A" || str == "I" || str == "R")
		return SubFieldDefinition(String, cnt);
	else if (str == "B")
		return SubFieldDefinition(Array, cnt / 8);
	else if (str == "b11")
		return SubFieldDefinition(U8, 1);
	else if (str == "b12")
		return SubFieldDefinition(U16, 2);
	else if (str == "b14")
		return SubFieldDefinition(U32, 4);
	else if (str == "b21")
		return SubFieldDefinition(S8, 1);
	else if (str == "b22")
		return SubFieldDefinition(S16, 2);
	else if (str == "b24")
		return SubFieldDefinition(S32, 4);
	else
		return SubFieldDefinition();
}

int ISO8211::readDR(QVector<FieldDefinition> &fields)
{
	DR ddr;
	QByteArray fieldLen, fieldPos;
	int len, lenSize, posSize, tagSize, offset;

	static_assert(sizeof(ddr) == 24, "Invalid DR alignment");
	if (_file.read((char*)&ddr, sizeof(ddr)) != sizeof(ddr))
		return -1;

	len = Util::str2int(ddr.RecordLength, sizeof(ddr.RecordLength));
	offset = Util::str2int(ddr.BaseAddress, sizeof(ddr.BaseAddress));
	lenSize = Util::str2int(&ddr.FieldLengthSize, sizeof(ddr.FieldLengthSize));
	posSize = Util::str2int(&ddr.FieldPosSize, sizeof(ddr.FieldPosSize));
	tagSize = Util::str2int(&ddr.FieldTagSize, sizeof(ddr.FieldTagSize));

	if (len < 0 || offset < 0 || lenSize < 0 || posSize < 0 || tagSize < 0)
		return -1;

	fields.resize((offset - 1 - sizeof(DR)) / (lenSize + posSize + tagSize));
	fieldLen.resize(lenSize);
	fieldPos.resize(posSize);

	for (int i = 0; i < fields.size(); i++) {
		FieldDefinition &r = fields[i];

		r.tag.resize(tagSize);

		if (_file.read(r.tag.data(), tagSize) != tagSize
		  || _file.read(fieldLen.data(), lenSize) != lenSize
		  || _file.read(fieldPos.data(), posSize) != posSize)
			return -1;

		r.pos = offset + Util::str2int(fieldPos.constData(), posSize);
		r.size = Util::str2int(fieldLen.constData(), lenSize);

		if (r.pos < 0 || r.size < 0)
			return -1;
	}

	return len;
}

bool ISO8211::readDDA(const FieldDefinition &def, SubFields &fields)
{
	static const QRegularExpression re("(\\d*)(\\w+)\\(*(\\d*)\\)*");
	QByteArray ba(def.size, Qt::Initialization::Uninitialized);
	bool repeat = false;
	QVector<SubFieldDefinition> defs;
	QVector<QByteArray> defTags;

	if (!(_file.seek(def.pos) && _file.read(ba.data(), ba.size()) == ba.size()))
		return false;

	QList<QByteArray> list(ba.split('\x1f'));
	if (!list.at(1).isEmpty() && list.at(1).front() == '*') {
		repeat = true;
		list[1].remove(0, 1);
	}
	QList<QByteArray> tags(list.at(1).split('!'));

	if (list.size() > 2) {
		QRegularExpressionMatchIterator it = re.globalMatch(list.at(2));
		int tag = 0;

		defs.resize(tags.size());
		defTags.resize(tags.size());

		while (it.hasNext()) {
			QRegularExpressionMatch match = it.next();
			QString cntStr(match.captured(1));
			QString typeStr(match.captured(2));
			QString sizeStr(match.captured(3));
			uint cnt = 1, size = 0;
			bool ok;

			if (!cntStr.isEmpty()) {
				cnt = cntStr.toUInt(&ok);
				if (!ok)
					return false;
			}
			if (!sizeStr.isEmpty()) {
				size = sizeStr.toUInt(&ok);
				if (!ok)
					return false;
			}

			for (uint i = 0; i < cnt; i++) {
				SubFieldDefinition sfd(fieldType(typeStr, size));
				if (sfd.type() == Unknown)
					return false;
				defs[tag] = sfd;
				defTags[tag] = tags.at(tag);
				tag++;
			}
		}
	}

	fields = SubFields(defTags, defs, repeat);

	return true;
}

bool ISO8211::readDDR()
{
	QVector<FieldDefinition> fields;

	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return false;
	}

	int len = readDR(fields);
	if (len < 0) {
		_errorString = "Not a ISO8211 file";
		return false;
	}

	for (int i = 0; i < fields.size(); i++) {
		SubFields def;
		if (!readDDA(fields.at(i), def)) {
			_errorString = QString("Error reading %1 DDA field")
			  .arg(QString(fields.at(i).tag));
			return false;
		}
		_map.insert(fields.at(i).tag, def);
	}

	if (_file.pos() != len || fields.size() < 2) {
		_errorString = "DDR format error";
		return false;
	}

	return true;
}

bool ISO8211::readUDA(quint64 pos, const FieldDefinition &def,
  const QVector<SubFieldDefinition> &fields, bool repeat, Data &data)
{
	QByteArray ba(def.size, Qt::Initialization::Uninitialized);

	if (!(_file.seek(pos + def.pos)
	  && _file.read(ba.data(), ba.size()) == ba.size()))
		return false;

	const char *sp;
	const char *dp = ba.constData();
	const char *ep = ba.constData() + ba.size() - 1;

	do {
		QVector<QVariant> row(fields.size());

		for (int i = 0; i < fields.size(); i++) {
			const SubFieldDefinition &f = fields.at(i);

			switch (f.type()) {
				case String:
				case Array:
					if (f.size()) {
						row[i] = QVariant(QByteArray(dp, f.size()));
						dp += f.size();
					} else {
						sp = dp;
						while (dp < ep && *dp != '\x1f')
							dp++;
						row[i] = QVariant(QByteArray(sp, dp - sp));
						dp++;
					}
					break;
				case S8:
					row[i] = QVariant(*((qint8*)dp));
					dp++;
					break;
				case S16:
					row[i] = QVariant(INT16(dp));
					dp += 2;
					break;
				case S32:
					row[i] = QVariant(INT32(dp));
					dp += 4;
					break;
				case U8:
					row[i] = QVariant(*((quint8*)dp));
					dp++;
					break;
				case U16:
					row[i] = QVariant(UINT16(dp));
					dp += 2;
					break;
				case U32:
					row[i] = QVariant(UINT32(dp));
					dp += 4;
					break;
				default:
					return false;
			}
		}

		data.append(row);
	} while (repeat && dp < ep);

	return true;
}

bool ISO8211::readRecord(Record &record)
{
	if (_file.atEnd())
		return false;

	QVector<FieldDefinition> fields;
	qint64 pos = _file.pos();


	if (readDR(fields) < 0) {
		_errorString = "Error reading DR";
		return false;
	}

	record.resize(fields.size());

	for (int i = 0; i < fields.size(); i++) {
		const FieldDefinition &def = fields.at(i);
		Data data;

		FieldsMap::const_iterator it(_map.find(def.tag));
		if (it == _map.constEnd()) {
			_errorString = QString("%1: unknown record").arg(QString(def.tag));
			return false;
		}

		if (!readUDA(pos, def, it->defs(), it->repeat(), data)) {
			_errorString = QString("Error reading %1 record")
			  .arg(QString(def.tag));
			return false;
		}

		record[i] = Field(def.tag, it->tags(), data);
	}

	return true;
}


const ISO8211::Field *ISO8211::field(const Record &record, const QByteArray &name)
{
	for (int i = 0; i < record.size(); i++)
		if (record.at(i).tag() == name)
			return &record.at(i);

	return 0;
}
