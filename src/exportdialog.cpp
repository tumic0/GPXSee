#include <QPrinter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QComboBox>
#include <QPushButton>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QLocale>
#include "fileselectwidget.h"
#include "exportdialog.h"


ExportDialog::ExportDialog(QPrinter *printer, QWidget *parent)
  : QDialog(parent), _printer(printer)
{
	int index;

	setWindowTitle(tr("Export to PDF"));

	_fileSelect = new FileSelectWidget();
	_fileSelect->setFilter(tr("PDF files (*.pdf);;All files (*)"));
	_fileSelect->setFile(QString("%1/export.pdf").arg(QDir::currentPath()));

	_paperSize = new QComboBox();
	_paperSize->addItem("A0", QPrinter::A0);
	_paperSize->addItem("A1", QPrinter::A1);
	_paperSize->addItem("A2", QPrinter::A2);
	_paperSize->addItem("A3", QPrinter::A3);
	_paperSize->addItem("A4", QPrinter::A4);
	_paperSize->addItem("A5", QPrinter::A5);
	_paperSize->addItem("A6", QPrinter::A6);
	_paperSize->addItem("Tabloid", QPrinter::Tabloid);
	_paperSize->addItem("Legal", QPrinter::Legal);
	_paperSize->addItem("Letter", QPrinter::Letter);
	index = (QLocale::system().measurementSystem() == QLocale::ImperialSystem)
	  ? 9 /* Letter */ : 4 /* A4 */;
	_paperSize->setCurrentIndex(index);

	_orientation = new QComboBox();
	_orientation->addItem(tr("Portrait"), QPrinter::Portrait);
	_orientation->addItem(tr("Landscape"), QPrinter::Landscape);
	index = _printer->orientation() == QPrinter::Portrait ? 0 : 1;
	_orientation->setCurrentIndex(index);

	QGroupBox *contentBox = new QGroupBox(tr("Export settings"));
	QFormLayout *contentLayout = new QFormLayout;
	contentLayout->addRow(tr("Page size:"), _paperSize);
	contentLayout->addRow(tr("Orientation"), _orientation);
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
	if (_fileSelect->file().isNull()) {
		QMessageBox::warning(this, tr("Error"), tr("No output file selected."));
		return false;
	}

	QFileInfo fi(_fileSelect->file());

	if (fi.isDir()) {
		QMessageBox::warning(this, tr("Error"),
		  tr("The output file is a directory."));
		return false;
	}

	QFileInfo di(fi.path());
	if (!di.isWritable()) {
		QMessageBox::warning(this, tr("Error"),
		  tr("The output file is not writable."));
		return false;
	}

	return true;
}

void ExportDialog::accept()
{
	if (!checkFile())
		return;

	QPrinter::Orientation orientation = static_cast<QPrinter::Orientation>
	  (_orientation->itemData(_orientation->currentIndex()).toInt());
	QPrinter::PaperSize paperSize = static_cast<QPrinter::PaperSize>
	  (_paperSize->itemData(_paperSize->currentIndex()).toInt());

	_printer->setOutputFormat(QPrinter::PdfFormat);
	_printer->setOutputFileName(_fileSelect->file());
	_printer->setPaperSize(paperSize);
	_printer->setOrientation(orientation);

	QDialog::accept();
}
