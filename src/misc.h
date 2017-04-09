#ifndef MISC_H
#define MISC_H

#include <QRectF>

#define ARRAY_SIZE(array) \
  (sizeof(array) / sizeof((array)[0]))

double niceNum(double x, int round);
int str2int(const char *str, int len);
QRectF scaled(const QRectF &rect, qreal factor);

#endif // MISC_H
