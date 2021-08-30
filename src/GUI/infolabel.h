#ifndef INFOLABEL_H
#define INFOLABEL_H

#include <QLabel>

class InfoLabel : public QLabel
{
public:
	InfoLabel(const QString &text, QWidget *parent = 0);
};

#endif // INFOLABEL_H
