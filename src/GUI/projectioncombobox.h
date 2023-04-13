#ifndef PROJECTIONCOMBOBOX_H
#define PROJECTIONCOMBOBOX_H

#include <QComboBox>
#include <QList>
#include "common/kv.h"

class ProjectionComboBox : public QComboBox
{
public:
	ProjectionComboBox(const QList<KV<int, QString> > &list, QWidget *parent = 0);
};

#endif // PROJECTIONCOMBOBOX_H
