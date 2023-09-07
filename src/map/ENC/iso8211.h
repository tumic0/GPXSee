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
		SubFieldDefinition(const QByteArray &tag, FieldType type, int size)
		  : _tag(tag), _type(type), _size(size) {}

		const QByteArray &tag() const {return _tag;}
		FieldType type() const {return _type;}
		int size() const {return _size;}

	private:
		QByteArray _tag;
		FieldType _type;
		int _size;
	};

	class SubFields
	{
	public:
		SubFields() : _repeat(false) {}
		SubFields(const QVector<SubFieldDefinition> &defs, bool repeat)
		  : _defs(defs), _repeat(repeat) {}

		int size() const {return _defs.size();}
		const SubFieldDefinition &at(int i) const {return _defs.at(i);}

		bool repeat() const {return _repeat;}

	private:
		QVector<SubFieldDefinition> _defs;
		bool _repeat;
	};

	class Data
	{
	public:
		Data() : _fields(0) {}
		Data(const SubFields *fields) : _fields(fields) {}

		int size() const {return _data.size();}
		const QVector<QVariant> &at(int i) const {return _data.at(i);}

		const SubFields *fields() const {return _fields;}
		const QVariant *field(const QByteArray &name, int idx = 0) const
		{
			const QVector<QVariant> &v = _data.at(idx);

			for (int i = 0; i < _fields->size(); i++)
				if (_fields->at(i).tag() == name)
					return &v.at(i);

			return 0;
		}

	private:
		friend class ISO8211;

		void append(QVector<QVariant> &row) {_data.append(row);}

		QVector<QVector<QVariant> > _data;
		const SubFields *_fields;
	};

	class Field
	{
	public:
		Field() {}
		Field(const QByteArray &tag, const Data &data)
		  : _tag(tag), _data(data) {}

		const QByteArray &tag() const {return _tag;}
		const Data &data() const {return _data;}

		bool subfield(const char *name, int *val, int idx = 0) const;
		bool subfield(const char *name, uint *val, int idx = 0) const;
		bool subfield(const char *name, QByteArray *val, int idx = 0) const;

	private:
		QByteArray _tag;
		Data _data;
	};

	typedef QVector<Field> Record;

	ISO8211(const QString &path) : _file(path) {}
	bool readDDR();
	bool readRecord(Record &record);

	const QString &errorString() const {return _errorString;}

	static const Field *field(const Record &record, const QByteArray &name);

private:
	typedef QMap<QByteArray, SubFields> FieldsMap;

	static SubFieldDefinition fieldType(const QString &str, int cnt,
	  const QByteArray &tag);

	int readDR(QVector<FieldDefinition> &fields);
	bool readDDA(const FieldDefinition &def, SubFields &fields);
	bool readUDA(quint64 pos, const FieldDefinition &def, Data &data);

	QFile _file;
	FieldsMap _map;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const ISO8211::FieldDefinition &def)
{
	dbg.nospace() << "FieldDefinition(" << def.tag << ", " << def.size << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const ISO8211::SubFieldDefinition &def)
{
	dbg.nospace() << "SubField(" << def.tag() << ", " << def.type() << ", "
	  << def.size() << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const ISO8211::Field &field)
{
	dbg.nospace() << "Field(" << field.tag() << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

}

#endif // ENC_ISO8211_H
