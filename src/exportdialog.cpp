#include <QPrinter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include "fileselectwidget.h"
#include "exportdialog.h"


ExportDialog::ExportDialog(QPrinter *printer, QWidget *parent)
  : QDialog(parent), _printer(printer)
{
	int index;

	setWindowTitle(tr("Export to PDF"));

	_fileSelect = new FileSelectWidget();
	_fileSelect->setFilter(tr("PDF files (*.pdf);;All files (*)"));
	_fileSelect->setFile(_printer->outputFileName());

	_paperSize = new QComboBox();
	_paperSize->addItem("A3", QPrinter::A3);
	_paperSize->addItem("A4", QPrinter::A4);
	_paperSize->addItem("A5", QPrinter::A5);
	_paperSize->addItem("Tabloid", QPrinter::Tabloid);
	_paperSize->addItem("Legal", QPrinter::Legal);
	_paperSize->addItem("Letter", QPrinter::Letter);
	if ((index = _paperSize->findData(_printer->paperSize())) >= 0)
		_paperSize->setCurrentIndex(index);

	_portrait = new QRadioButton(tr("Portrait"));
	_landscape = new QRadioButton(tr("Landscape"));
	QHBoxLayout *orientationLayout = new QHBoxLayout();
	orientationLayout->addWidget(_portrait);
	orientationLayout->addWidget(_landscape);
	if (_printer->orientation() == QPrinter::Portrait)
		_portrait->setChecked(true);
	else
		_landscape->setChecked(true);

	QGroupBox *contentBox = new QGroupBox(tr("Settings"));
	QFormLayout *contentLayout = new QFormLayout;
	contentLayout->addRow(tr("Page size:"), _paperSize);
	contentLayout->addRow(tr("Orientation:"), orientationLayout);
	contentLayout->addRow(tr("Output file:"), _fileSelect);
	contentBox->setLayout(contentLayout);

	QPushButton *exportButton = new QPushButton(tr("Export"));
	exportButton->setAutoDefault(true);
	exportButton->setDefault(true);
	connect(exportButton, SIGNAL(clicked()), this, SLOT(accept()));
	QPushButton *cancelButton = new QPushButton(tr("Cancel"));
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));

	QHBoxLayout *buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(exportButton);
	buttonsLayout->addWidget(cancelButton);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(contentBox);
	layout->addLayout(buttonsLayout);

	setLayout(layout);
}

bool ExportDialog::checkFile()
{
	if (_fileSelect->file().isEmpty()) {
		QMessageBox::warning(this, tr("Error"), tr("No output file selected."));
		return false;
	}

	QFile file(_fileSelect->file());
	QFileInfo fi(file);
	bool exists = fi.exists();
	bool opened = false;

	if (exists && fi.isDir()) {
		QMessageBox::warning(this, tr("Error"), tr("%1 is a directory.")
		  .arg(file.fileName()));
		return false;
	} else if ((exists && !fi.isWritable())
	  || !(opened = file.open(QFile::Append))) {
		QMessageBox::warning(this, tr("Error"), tr("%1 is not writable.")
		  .arg(file.fileName()));
		return false;
	}
	if (opened) {
		file.close();
		if (!exists)
			file.remove();
	}

	return true;
}

void ExportDialog::accept()
{
	if (!checkFile())
		return;

	QPrinter::Orientation orientation = _portrait->isChecked()
	  ? QPrinter::Portrait : QPrinter::Landscape;
	QPrinter::PaperSize paperSize = static_cast<QPrinter::PaperSize>
	  (_paperSize->itemData(_paperSize->currentIndex()).toInt());

	_printer->setOutputFormat(QPrinter::PdfFormat);
	_printer->setOutputFileName(_fileSelect->file());
	_printer->setPaperSize(paperSize);
	_printer->setOrientation(orientation);

	QDialog::accept();
}
