#ifndef STYLECOMBOBOX_H
#define STYLECOMBOBOX_H

#include <QComboBox>

class StyleComboBox : public QComboBox
{
	Q_OBJECT

public:
	StyleComboBox(QWidget *parent = 0);

	void setValue(Qt::PenStyle value);
};

#endif // STYLECOMBOBOX_H
