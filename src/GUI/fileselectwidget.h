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

#ifdef Q_OS_ANDROID
	QString file() const {return _fileName;}
#else // Q_OS_ANDROID
	QString file() const {return _edit->text();}
	void setFile(const QString &file) {_edit->setText(file);}
	void setFilter(const QString &filter) {_filter = filter;}
#endif // Q_OS_ANDROID
	bool checkFile(QString &error) const;

private slots:
	void browse();

private:
	QLineEdit *_edit;
#ifdef Q_OS_WIN32
	QPushButton *_button;
#else // Q_OS_WIN32
	QToolButton *_button;
#endif // Q_OS_WIN32
#ifdef Q_OS_ANDROID
	QString _fileName;
#else // Q_OS_ANDROID
	QString _filter;
#endif // Q_OS_ANDROID
};

#endif // FILESELECTWIDGET_H
