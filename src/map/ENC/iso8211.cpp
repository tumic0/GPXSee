#include <QtEndian>
#include <QFile>
#include <QRegularExpression>
#include "common/util.h"
#include "iso8211.h"

using namespace ENC;

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

static QString NAME(quint32 tag)
{
	char buffer[sizeof(quint32)];
	qToLittleEndian<quint32>(tag, buffer);
	return QString::fromLatin1(buffer, sizeof(buffer));
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

const ISO8211::Field *ISO8211::Record::field(quint32 name) const
{
	for (int i = 0; i < size(); i++)
		if (at(i).tag() == name)
			return &at(i);

	return 0;
}

int ISO8211::readDR(QVector<FieldDefinition> &fields)
{
	static_assert(sizeof(DR) == 24, "Invalid DR alignment");
	if (_dp + sizeof(DR) > _end)
		return -1;

	const DR *ddr = (const DR*)(_dp);
	int len = Util::str2int(ddr->RecordLength, sizeof(ddr->RecordLength));
	int offset = Util::str2int(ddr->BaseAddress, sizeof(ddr->BaseAddress));
	int lenSize = Util::str2int(&ddr->FieldLengthSize, sizeof(ddr->FieldLengthSize));
	int posSize = Util::str2int(&ddr->FieldPosSize, sizeof(ddr->FieldPosSize));
	int tagSize = Util::str2int(&ddr->FieldTagSize, sizeof(ddr->FieldTagSize));
	_dp += sizeof(DR);

	if (len < 0 || offset < 0 || lenSize < 0 || posSize < 0 || tagSize != 4)
		return -1;

	int count = (offset - 1 - sizeof(DR)) / (lenSize + posSize + tagSize);
	if (_dp + count * (lenSize + posSize + tagSize) > _end)
		return -1;
	fields.resize(count);

	for (int i = 0; i < fields.size(); i++) {
		FieldDefinition &r = fields[i];

		r.tag = *_dp | ((quint32)*(_dp + 1)) << 8 | ((quint32)*(_dp + 2)) << 16
		  | ((quint32)*(_dp + 3)) << 24;
		_dp += 4;
		r.size = Util::str2int((const char*)_dp, lenSize);
		_dp += lenSize;
		r.pos = Util::str2int((const char*)_dp, posSize);
		_dp += posSize;

		if (r.pos < 0 || r.size < 0)
			return -1;

		r.pos += offset;
	}

	return len;
}

bool ISO8211::readDDA(const FieldDefinition &def, SubFields &fields)
{
	static const QRegularExpression re(
	  "([0-9]*)(A|I|R|B|b11|b12|b14|b21|b22|b24)\\(*([0-9]*)\\)*");
	bool repeat = false;
	QVector<SubFieldDefinition> defs;
	QVector<quint32> defTags;

	if (_data + def.pos + def.size > _end)
		return false;
	_dp = _data + def.pos;

	QByteArray ba(QByteArray::fromRawData((const char*)_dp, def.size));
	_dp += def.size;
	QList<QByteArray> list(ba.split('\x1f'));

	if (list.size() > 2) {
		if (!list.at(1).isEmpty() && list.at(1).front() == '*') {
			repeat = true;
			list[1].remove(0, 1);
		}

		QList<QByteArray> tags(list.at(1).split('!'));
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
				if (tag >= tags.size())
					return false;
				defs[tag] = sfd;
				defTags[tag] = (tags.at(tag).length() == 4)
				  ? qFromLittleEndian<quint32>(tags.at(tag).constData()) : 0;
				tag++;
			}
		}

		if (tag != tags.size())
			return false;
	}

	fields = SubFields(defTags, defs, repeat);

	return true;
}

bool ISO8211::readDDR()
{
	QVector<FieldDefinition> defs;

	if (!_file.open(QIODevice::ReadOnly)) {
		_errorString = _file.errorString();
		return false;
	}

	qint64 size = _file.size();
	if (!(_data = _file.map(0, size))) {
		_errorString = _file.errorString();
		return false;
	}
	_end = _data + size;
	_dp = _data;

	int len = readDR(defs);
	if (len < 0) {
		_errorString = "Not a ISO8211 file";
		return false;
	}

	for (int i = 0; i < defs.size(); i++) {
		SubFields fields;
		const FieldDefinition &def = defs.at(i);

		if (!readDDA(def, fields)) {
			_errorString = QString("Error reading %1 DDA field")
			  .arg(NAME(def.tag));
			return false;
		}
		_map.insert(def.tag, fields);
	}

	if (defs.size() < 2 || defs.last().pos + defs.last().size != len) {
		_errorString = "DDR format error";
		return false;
	}

	return true;
}

bool ISO8211::readUDA(int pos, const FieldDefinition &def,
  const QVector<SubFieldDefinition> &fields, bool repeat, Data &data)
{
	if (_data + pos + def.pos + def.size > _end)
		return false;
	_dp = _data + pos + def.pos;

	const char *dp = (const char*)_dp;
	const char *ep = (const char*)_dp + def.size - 1;
	const char *sp;

	_dp += def.size;

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
					row[i] = QVariant(qFromLittleEndian<qint16>(dp));
					dp += 2;
					break;
				case S32:
					row[i] = QVariant(qFromLittleEndian<qint32>(dp));
					dp += 4;
					break;
				case U8:
					row[i] = QVariant(*((quint8*)dp));
					dp++;
					break;
				case U16:
					row[i] = QVariant(qFromLittleEndian<quint16>(dp));
					dp += 2;
					break;
				case U32:
					row[i] = QVariant(qFromLittleEndian<quint32>(dp));
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
	QVector<FieldDefinition> fields;

	Q_ASSERT(_dp != _data);
	int pos = _dp - _data;

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
			_errorString = QString("%1: unknown record").arg(NAME(def.tag));
			return false;
		}

		if (!readUDA(pos, def, it->defs(), it->repeat(), data)) {
			_errorString = QString("Error reading %1 record").arg(NAME(def.tag));
			return false;
		}

		record[i] = Field(def.tag, data);
	}

	return true;
}

ISO8211::ISO8211(const QString &path) : _file(path), _data(0), _end(0), _dp(0)
{
}

ISO8211::~ISO8211()
{
	_file.unmap(_data);
}
