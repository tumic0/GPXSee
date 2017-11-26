#ifndef FILESELECTWIDGET_H
#define FILESELECTWIDGET_H

#include <QWidget>
#include <QLineEdit>

class QPushButton;
class QToolButton;

class FileSelectWidget : public QWidget
{
	Q_OBJECT

public:
	FileSelectWidget(QWidget *parent = 0);

	QString file() {return _edit->text();}
	void setFile(const QString &file) {_edit->setText(file);}
	void setFilter(const QString &filter) {_filter = filter;}

private slots:
	void browse();

private:
	QLineEdit *_edit;
#ifdef Q_OS_WIN32
	QPushButton *_button;
#else // Q_OS_WIN32
	QToolButton *_button;
#endif // Q_OS_WIN32

	QString _filter;
};

#endif // FILESELECTWIDGET_H
