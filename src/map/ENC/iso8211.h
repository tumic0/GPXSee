#ifndef ENC_ISO8211_H
#define ENC_ISO8211_H

#include <QByteArray>
#include <QVariant>
#include <QDebug>

class QFile;

#define UINT32(x) \
  (((quint32)*(const uchar*)(x)) \
  | ((quint32)(*((const uchar*)(x) + 1)) << 8) \
  | ((quint32)(*((const uchar*)(x) + 2)) << 16) \
  | ((quint32)(*((const uchar*)(x) + 3)) << 24))

#define UINT16(x) \
  (((quint16)*(const uchar*)(x)) \
  | ((quint16)(*((const uchar*)(x) + 1)) << 8))

#define INT32(x) ((qint32)UINT32(x))
#define INT16(x) ((qint16)UINT16(x))

namespace ENC {

class ISO8211
{
public:
	enum FieldType {String, Array, S8, S16, S32, U8, U16, U32};

	struct FieldDefinition {
		QByteArray tag;
		int pos;
		int size;
	};

	struct SubFieldDefinition {
		QByteArray tag;
		FieldType type;
		int size;
	};

	class SubFields : public QVector<SubFieldDefinition>
	{
	public:
		SubFields() : QVector<SubFieldDefinition>(), _repeat(false) {}

		bool repeat() const {return _repeat;}
		void setRepeat(bool repeat) {_repeat = repeat;}

	private:
		bool _repeat;
	};

	class Data : public QVector<QVector<QVariant> >
	{
	public:
		Data() : QVector<QVector<QVariant> >(), _fields(0) {}

		void setFields(const SubFields *fields) {_fields = fields;}

		const QVariant *field(const QByteArray &name, int idx = 0) const
		{
			const QVector<QVariant> &v = at(idx);

			for (int i = 0; i < _fields->size(); i++)
				if (_fields->at(i).tag == name)
					return &v.at(i);

			return 0;
		}

	private:
		const SubFields *_fields;
	};

	class Field
	{
	public:
		const QByteArray &tag() const {return _tag;}
		void setTag(const QByteArray &tag) {_tag = tag;}
		Data &rdata() {return _data;}
		const Data &data() const {return _data;}

		bool subfield(const char *name, int *val, int idx = 0) const;
		bool subfield(const char *name, uint *val, int idx = 0) const;
		bool subfield(const char *name, QByteArray *val, int idx = 0) const;

	private:
		QByteArray _tag;
		Data _data;
	};

	class Record : public QVector<Field>
	{
	public:
		const Field *field(const QByteArray &name) const
		{
			for (int i = 0; i < size(); i++)
				if (at(i).tag() == name)
					return &at(i);
			return 0;
		}
	};

	bool readDDR(QFile &file);
	bool readRecord(QFile &file, Record &record);

	const QString &errorString() const {return _errorString;}

private:
	typedef QMap<QByteArray, SubFields> FieldsMap;

	static bool fieldType(const QString &str, int cnt, FieldType &type,
	  int &size);

	int readDR(QFile &file, QVector<FieldDefinition> &fields) const;
	bool readDDA(QFile &file, const FieldDefinition &def, SubFields &fields);
	bool readUDA(QFile &file, quint64 pos, const FieldDefinition &def,
	  const SubFields &fields, Data &data) const;

	FieldsMap _map;
	QString _errorString;
};

#ifndef QT_NO_DEBUG
inline QDebug operator<<(QDebug dbg, const ISO8211::FieldDefinition &def)
{
	dbg.nospace() << "Field(" << def.tag << ", " << def.size << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const ISO8211::SubFieldDefinition &def)
{
	dbg.nospace() << "SubField(" << def.tag << ", " << def.type << ", "
	  << def.size << ")";
	return dbg.space();
}

inline QDebug operator<<(QDebug dbg, const ISO8211::Field &field)
{
	dbg.nospace() << "Field(" << field.tag() /*<< ", " << field.data()*/ << ")";
	return dbg.space();
}
#endif // QT_NO_DEBUG

}

#endif // ENC_ISO8211_H
