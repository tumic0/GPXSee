#ifndef DIRSELECTWIDGET_H
#define DIRSELECTWIDGET_H

#include <QWidget>
#include <QLineEdit>

class QPushButton;
class QToolButton;

class DirSelectWidget : public QWidget
{
	Q_OBJECT

public:
	DirSelectWidget(QWidget *parent = 0);

	QString dir() const {return _edit->text();}
	void setDir(const QString &path) {_edit->setText(path);}
	bool checkDir(QString &error) const;

private slots:
	void browse();

private:
	QLineEdit *_edit;
#ifdef Q_OS_WIN32
	QPushButton *_button;
#else // Q_OS_WIN32
	QToolButton *_button;
#endif // Q_OS_WIN32
};

#endif // DIRSELECTWIDGET_H
