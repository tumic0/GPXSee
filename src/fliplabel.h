#ifndef FLIPLABEL_H
#define FLIPLABEL_H

#include <QLabel>
#include <QMap>

class FlipLabel : public QLabel
{
public:
	FlipLabel(QWidget *parent = 0) : QLabel(parent) {}

	void addItem(const QString &key, const QString &value);

protected:
	void mousePressEvent(QMouseEvent *event);

private:
	QMap<QString, QString> _labels;
	QMap<QString, QString>::const_iterator _it;
};

#endif // FLIPLABEL_H
