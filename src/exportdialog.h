#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class QPrinter;
class QComboBox;
class FileSelectWidget;

class ExportDialog : public QDialog
{
	Q_OBJECT

public:
	ExportDialog(QPrinter *printer, QWidget *parent = 0);

public slots:
	void accept();

private:
	bool checkFile();

	QPrinter *_printer;

	FileSelectWidget *_fileSelect;
	QComboBox *_paperSize;
	QComboBox *_orientation;
};

#endif // EXPORTDIALOG_H
