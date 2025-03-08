#include <QByteArrayList>
#include "csv.h"

/*
RFC 4180 parser with the following enchancements:
 - allows an arbitrary delimiter
 - allows LF line ends in addition to CRLF line ends
*/

bool CSV::readEntry(QByteArrayList &list, int limit)
{
	int state = 0, len = 0;
	char c;
	QByteArray field;

	list.clear();

	while (_device->getChar(&c)) {
		if (limit && ++len > limit)
			return false;

		switch (state) {
			case 0:
				if (c == '\r')
					state = 3;
				else if (c == '\n') {
					list.append(field);
					_line++;
					return true;
				} else if (c == _delimiter) {
					list.append(field);
					field.clear();
				} else if (c == '"') {
					if (!field.isEmpty())
						return false;
					state = 1;
				} else
					field.append(c);
				break;
			case 1:
				if (c == '"')
					state = 2;
				else {
					field.append(c);
					if (c == '\n')
						_line++;
				}
				break;
			case 2:
				if (c == '"') {
					field.append('"');
					state = 1;
				} else if (c == _delimiter || c == '\r' || c == '\n') {
					_device->ungetChar(c);
					state = 0;
				} else
					return false;
				break;
			case 3:
				if (c == '\n') {
					_device->ungetChar(c);
					state = 0;
				} else
					return false;
				break;
		}
	}

	list.append(field);

	return (_device->atEnd() && (state == 0 || state == 2));
}
