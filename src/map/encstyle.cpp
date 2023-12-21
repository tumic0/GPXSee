#include "encstyle.h"

const ENC::Style *ENCStyle::style()
{
	static ENC::Style s;
	return &s;
}
