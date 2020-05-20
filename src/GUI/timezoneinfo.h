#ifndef TIMEZONEINFO_H
#define TIMEZONEINFO_H

#include <QTimeZone>
#include <QDataStream>

class TimeZoneInfo
{
public:
	enum Type {
		UTC,
		System,
		Custom
	};

	TimeZoneInfo() : _type(UTC), _customZone(QTimeZone::systemTimeZone()) {}

	Type type()  const {return _type;}
	const QTimeZone &customZone() const {return _customZone;}
	QTimeZone zone() const
	{
		if (_type == UTC)
			return QTimeZone::utc();
		else if (_type == System)
			return QTimeZone::systemTimeZone();
		else
			return _customZone;
	}

	void setType(Type type) {_type = type;}
	void setCustomZone(const QTimeZone &zone) {_customZone = zone;}

	bool operator==(const TimeZoneInfo &other) const
	{
		if (_type == UTC || _type == System)
			return _type == other._type;
		else
			return (other._type == Custom && _customZone == other._customZone);
	}
	bool operator!=(const TimeZoneInfo &other) {return !(*this == other);}

private:
	friend QDataStream& operator<<(QDataStream &out, const TimeZoneInfo &info);
	friend QDataStream& operator>>(QDataStream &in, TimeZoneInfo &info);

	Type _type;
	QTimeZone _customZone;
};

Q_DECLARE_METATYPE(TimeZoneInfo)

inline QDataStream &operator<<(QDataStream &out, const TimeZoneInfo &info)
{
	out << static_cast<int>(info._type) << info._customZone;
	return out;
}

inline QDataStream &operator>>(QDataStream &in, TimeZoneInfo &info)
{
	int t;

	in >> t;
	info._type = static_cast<TimeZoneInfo::Type>(t);
	in >> info._customZone;

	return in;
}

#endif // TIMEZONEINFO_H
