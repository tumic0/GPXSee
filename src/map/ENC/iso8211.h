#ifndef ENC_ISO8211_H
#define ENC_ISO8211_H

#include <QFile>
#include <QByteArray>
#include <QVariant>
#include <QDebug>

#define UINT32(x) \
  (((quint32)*(const uchar*)(x)) \
  | ((quint32)(*((const uchar*)(x) + 1)) << 8) \
  | ((quint32)(*((const uchar*)(x) + 2)) << 16) \
  | ((quint32)(*((const uchar*)(x) + 3)) << 24))

namespace ENC {

class ISO8211
{
public:
	typedef QVector<QVector<QVariant> > Data;

	class Field
	{
	public:
		Field() {}
		Field(const QByteArray &tag, const QVector<QByteArray> &subFields,
		  const Data &data) : _tag(tag), _subFields(subFields), _data(data) {}

		const QByteArray &tag() const {return _tag;}
		const QVector<QByteArray> &subFields() const {return _subFields;}
		const Data &data() const {return _data;}

		bool subfield(const char *name, int *val, int idx = 0) const;
		bool subfield(const char *name, uint *val, int idx = 0) const;
		bool subfield(const char *name, QByteArray *val, int idx = 0) const;

	private:
		const QVariant *data(const QByteArray &name, int idx = 0) const;

		QByteArray _tag;
		QVector<QByteArray> _subFields;
		Data _data;
	};

	typedef QVector<Field> Record;

	ISO8211(const QString &path) : _file(path) {}
	bool readDDR();
	bool readRecord(Record &record);

	const QString &errorString() const {return _errorString;}

	static const Field *field(const Record &record, const QByteArray &name);

private:
	enum FieldType {Unknown, String, Array, S8, S16, S32, U8, U16, U32};

	struct FieldDefinition
	{
		QByteArray tag;
		int pos;
		int size;
	};

	class SubFieldDefinition
	{
	public:
		SubFieldDefinition() : _type(Unknown), _size(0) {}
		SubFieldDefinition(FieldType type, int size)
		  : _type(type), _size(size) {}

		FieldType type() const {return _type;}
		int size() const {return _size;}

	private:
		FieldType _type;
		int _size;
	};

	class SubFields
	{
	public:
		SubFields() : _repeat(false) {}
		SubFields(const QVector<QByteArray> &tags,
		  const QVector<SubFieldDefinition> &defs, bool repeat)
		  : _tags(tags), _defs(defs), _repeat(repeat) {}

		const QVector<QByteArray> &tags() const {return _tags;}
		const QVector<SubFieldDefinition> &defs() const {return _defs;}

		bool repeat() const {return _repeat;}

	private:
		QVector<QByteArray> _tags;
		QVector<SubFieldDefinition> _defs;
		bool _repeat;
	};

	typedef QMap<QByteArray, SubFields> FieldsMap;

	static SubFieldDefinition fieldType(const QString &str, int cnt);

	int readDR(QVector<FieldDefinition> &fields);
	bool readDDA(const FieldDefinition &def, SubFields &fields);
	bool readUDA(quint64 pos, const FieldDefinition &def,
	  const QVector<SubFieldDefinition> &fields, bool repeat, Data &data);

	QFile _file;
	FieldsMap _map;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const ISO8211::Field &field)
{
	dbg.nospace() << "Field(" << field.tag() << ", " << field.subFields() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

}

#endif // ENC_ISO8211_H
