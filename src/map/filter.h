#ifndef FILTER_H
#define FILTER_H

#include "matrix.h"

namespace Filter
{
	MatrixD blur(const MatrixD &m, int radius);
}

#endif // FILTER_H
