#include <cctype>
#include <cmath>
#include "util.h"


int str2int(const char *str, int len)
{
	int res = 0;

	for (const char *sp = str; sp < str + len; sp++) {
		if (::isdigit(*sp))
			res = res * 10 + *sp - '0';
		else
			return -1;
	}

	return res;
}

double niceNum(double x, bool round)
{
	int expv;
	double f;
	double nf;

	expv = (int)floor(log10(x));
	f = x / pow(10.0, expv);

	if (round) {
		if (f < 1.5)
			nf = 1.0;
		else if (f < 3.0)
			nf = 2.0;
		else if (f < 7.0)
			nf = 5.0;
		else
			nf = 10.0;
	} else {
		if (f <= 1.0)
			nf = 1.0;
		else if (f <= 2.0)
			nf = 2.0;
		else if (f <= 5.0)
			nf = 5.0;
		else
			nf = 10.0;
	}

	return nf * pow(10.0, expv);
}
