#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QPrinter>

class QComboBox;
class QRadioButton;
class FileSelectWidget;
class QDoubleSpinBox;

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

	QPrinter::Unit _units;

	FileSelectWidget *_fileSelect;
	QComboBox *_paperSize;
	QRadioButton *_portrait;
	QRadioButton *_landscape;
	QDoubleSpinBox *_topMargin;
	QDoubleSpinBox *_bottomMargin;
	QDoubleSpinBox *_leftMargin;
	QDoubleSpinBox *_rightMargin;
};

#endif // EXPORTDIALOG_H
