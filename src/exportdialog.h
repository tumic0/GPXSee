#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>

class QPrinter;
class QComboBox;
class QRadioButton;
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
	QRadioButton *_portrait;
	QRadioButton *_landscape;
};

#endif // EXPORTDIALOG_H
