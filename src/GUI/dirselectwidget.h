#ifndef DIRSELECTWIDGET_H
#define DIRSELECTWIDGET_H

#include <QWidget>
#include <QLineEdit>

class DirSelectWidget : public QWidget
{
	Q_OBJECT

public:
	DirSelectWidget(QWidget *parent = 0);

	QString dir() const {return _edit->text();}
	void setDir(const QString &path) {_edit->setText(path);}

private slots:
	void browse();

private:
	QLineEdit *_edit;
};

#endif // DIRSELECTWIDGET_H
