#ifndef MISC_H
#define MISC_H

#include <QRectF>

double niceNum(double x, int round);
int str2int(const char *str, int len);
QRectF scaled(const QRectF &rect, qreal factor);

#endif // MISC_H
