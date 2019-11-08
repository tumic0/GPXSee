#ifndef ADDRESS_H
#define ADDRESS_H

#include <QString>

class Address
{
public:
	Address() {}
	Address(const QString &street, const QString &city)
	  : _street(street), _city(city) {}

	const QString &street() const {return _street;}
	const QString &city() const {return _city;}
	const QString &state() const {return _state;}
	const QString &country() const {return _country;}
	const QString &postalCode() const {return _postalCode;}

	void setStreet(const QString &street) {_street = street;}
	void setCity(const QString &city) {_city = city;}
	void setState(const QString &state) {_state = state;}
	void setCountry(const QString &country) {_country = country;}
	void setPostalCode(const QString &postalCode) {_postalCode = postalCode;}

	bool isValid() const {return !(_street.isEmpty() || _city.isEmpty());}

private:
	QString _street;
	QString _city;
	QString _state;
	QString _country;
	QString _postalCode;
};

#endif // ADDRESS_H
