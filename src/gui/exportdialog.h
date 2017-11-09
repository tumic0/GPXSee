#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

#include <QDialog>
#include <QPrinter>
#include "margins.h"
#include "units.h"

class QComboBox;
class QRadioButton;
class FileSelectWidget;
class QDoubleSpinBox;

struct Export {
	QString fileName;
	QPrinter::PaperSize paperSize;
	QPrinter::Orientation orientation;
	MarginsF margins;
	int resolution;

	Units units;
};

class ExportDialog : public QDialog
{
	Q_OBJECT

public:
	ExportDialog(Export *exp, QWidget *parent = 0);

public slots:
	void accept();

private:
	bool checkFile();

	Export *_export;

	FileSelectWidget *_fileSelect;
	QComboBox *_paperSize;
	QComboBox *_resolution;
	QRadioButton *_portrait;
	QRadioButton *_landscape;
	QDoubleSpinBox *_topMargin;
	QDoubleSpinBox *_bottomMargin;
	QDoubleSpinBox *_leftMargin;
	QDoubleSpinBox *_rightMargin;
};

#endif // EXPORTDIALOG_H
