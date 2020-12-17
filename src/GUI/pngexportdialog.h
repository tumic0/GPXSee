#ifndef PNGEXPORTDIALOG_H
#define PNGEXPORTDIALOG_H

#include <QDialog>
#include <QMargins>

class FileSelectWidget;
class MarginsWidget;
class QSpinBox;
class QCheckBox;

struct PNGExport
{
	QString fileName;
	QSize size;
	QMargins margins;
	bool antialiasing;
};

class PNGExportDialog : public QDialog
{
	Q_OBJECT

public:
	PNGExportDialog(PNGExport &exp, QWidget *parent = 0);

public slots:
	void accept();

private:
	PNGExport &_export;

	FileSelectWidget *_fileSelect;
	QSpinBox *_width;
	QSpinBox *_height;
	MarginsWidget *_margins;
	QCheckBox *_antialiasing;
};

#endif // PNGEXPORTDIALOG_H
