#ifndef PDFEXPORTDIALOG_H
#define PDFEXPORTDIALOG_H

#include <QDialog>
#include <QPrinter>
#include "margins.h"
#include "units.h"

class QComboBox;
class QRadioButton;
class FileSelectWidget;
class MarginsFWidget;

struct PDFExport
{
	QString fileName;
	QPrinter::PaperSize paperSize;
	QPrinter::Orientation orientation;
	MarginsF margins;
	int resolution;
};

class PDFExportDialog : public QDialog
{
	Q_OBJECT

public:
	PDFExportDialog(PDFExport &exp, Units units, QWidget *parent = 0);

public slots:
	void accept();

private:
	PDFExport &_export;

	Units _units;
	FileSelectWidget *_fileSelect;
	QComboBox *_paperSize;
	QComboBox *_resolution;
	QRadioButton *_portrait;
	QRadioButton *_landscape;
	MarginsFWidget *_margins;
};

#endif // PDFEXPORTDIALOG_H
