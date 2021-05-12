#include "address.h"

QString Address::address() const
{
	QString addr(_street);

	if (addr.isEmpty())
		addr = _city;
	else
		addr += "\n" + _city;
	if (!_postalCode.isEmpty())
		addr += "\n" + _postalCode;
	if (!_state.isEmpty())
		addr += "\n" + _state;
	if (!_country.isEmpty())
		addr += "\n" + _country;

	return addr;
}
