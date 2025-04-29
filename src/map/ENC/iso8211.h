#ifndef ENC_ISO8211_H
#define ENC_ISO8211_H

#include <QFile>
#include <QByteArray>
#include <QVariant>

namespace ENC {

class ISO8211
{
public:
	typedef QVector<QVector<QVariant> > Data;

	class Field
	{
	public:
		Field() : _tag(0) {}
		Field(quint32 tag, const Data &data) : _tag(tag), _data(data) {}

		quint32 tag() const {return _tag;}
		const Data &data() const {return _data;}

	private:
		quint32 _tag;
		Data _data;
	};

	class Record : public QVector<Field>
	{
	public:
		const Field *field(quint32 name) const;
	};

	ISO8211(const QString &path) : _file(path) {}
	bool readDDR();
	bool readRecord(Record &record);
	const QString &errorString() const {return _errorString;}

	static constexpr quint32 NAME(const char str[4])
	{
		return static_cast<quint32>(str[0])
		  + (static_cast<quint32>(str[1]) << 8)
		  + (static_cast<quint32>(str[2]) << 16)
		  + (static_cast<quint32>(str[3]) << 24);
	}

private:
	enum FieldType {Unknown, String, Array, S8, S16, S32, U8, U16, U32};

	struct FieldDefinition
	{
		quint32 tag;
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
		SubFields(const QVector<quint32> &tags,
		  const QVector<SubFieldDefinition> &defs, bool repeat)
		  : _tags(tags), _defs(defs), _repeat(repeat) {}

		const QVector<quint32> &tags() const {return _tags;}
		const QVector<SubFieldDefinition> &defs() const {return _defs;}

		bool repeat() const {return _repeat;}

	private:
		QVector<quint32> _tags;
		QVector<SubFieldDefinition> _defs;
		bool _repeat;
	};

	typedef QMap<quint32, SubFields> FieldsMap;

	static SubFieldDefinition fieldType(const QString &str, int cnt);

	int readDR(QVector<FieldDefinition> &fields);
	bool readDDA(const FieldDefinition &def, SubFields &fields);
	bool readUDA(quint64 pos, const FieldDefinition &def,
	  const QVector<SubFieldDefinition> &fields, bool repeat, Data &data);

	QFile _file;
	FieldsMap _map;
	QString _errorString;
};

}

#endif // ENC_ISO8211_H
