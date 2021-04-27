#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QComboBox>
#include <QRadioButton>
#include <QMessageBox>
#include <QTabWidget>
#include "marginswidget.h"
#include "fileselectwidget.h"
#include "units.h"
#include "pdfexportdialog.h"


PDFExportDialog::PDFExportDialog(PDFExport &exp, Units units, QWidget *parent)
  : QDialog(parent), _export(exp), _units(units)
{
	int index;

	_fileSelect = new FileSelectWidget();
	_fileSelect->setFilter(tr("PDF files") + " (*.pdf);;" + tr("All files")
	  + " (*)");
	_fileSelect->setFile(_export.fileName);

	_paperSize = new QComboBox();
	_paperSize->addItem("A2", QPageSize::PageSizeId::A2);
	_paperSize->addItem("A3", QPageSize::PageSizeId::A3);
	_paperSize->addItem("A4", QPageSize::PageSizeId::A4);
	_paperSize->addItem("A5", QPageSize::PageSizeId::A5);
	_paperSize->addItem("A6", QPageSize::PageSizeId::A6);
	_paperSize->addItem("B3", QPageSize::PageSizeId::B3);
	_paperSize->addItem("B4", QPageSize::PageSizeId::B4);
	_paperSize->addItem("B5", QPageSize::PageSizeId::B5);
	_paperSize->addItem("B6", QPageSize::PageSizeId::B6);
	_paperSize->addItem("Tabloid", QPageSize::PageSizeId::Tabloid);
	_paperSize->addItem("Legal", QPageSize::PageSizeId::Legal);
	_paperSize->addItem("Letter", QPageSize::PageSizeId::Letter);
	if ((index = _paperSize->findData(_export.paperSize)) >= 0)
		_paperSize->setCurrentIndex(index);

	_resolution = new QComboBox();
	_resolution->addItem("150 DPI", 150);
	_resolution->addItem("300 DPI", 300);
	_resolution->addItem("600 DPI", 600);
	if ((index = _resolution->findData(_export.resolution)) >= 0)
		_resolution->setCurrentIndex(index);

	_portrait = new QRadioButton(tr("Portrait"));
	_landscape = new QRadioButton(tr("Landscape"));
	QHBoxLayout *orientationLayout = new QHBoxLayout();
	orientationLayout->addWidget(_portrait);
	orientationLayout->addWidget(_landscape);
	if (_export.orientation == QPageLayout::Orientation::Portrait)
		_portrait->setChecked(true);
	else
		_landscape->setChecked(true);

	_margins = new MarginsFWidget();
	_margins->setUnits((units == Metric) ? tr("cm") : tr("in"));
	_margins->setSingleStep(0.1);
	_margins->setValue((units == Metric)
	  ? _export.margins * MM2CM : _export.margins * MM2IN);

#ifndef Q_OS_MAC
	QGroupBox *pageSetupBox = new QGroupBox(tr("Page Setup"));
#endif // Q_OS_MAC
	QFormLayout *pageSetupLayout = new QFormLayout;
	pageSetupLayout->addRow(tr("Page size:"), _paperSize);
	pageSetupLayout->addRow(tr("Resolution:"), _resolution);
	pageSetupLayout->addRow(tr("Orientation:"), orientationLayout);
	pageSetupLayout->addRow(tr("Margins:"), _margins);
#ifdef Q_OS_MAC
	QFrame *line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	pageSetupLayout->addRow(line);
	pageSetupLayout->addRow(tr("File:"), _fileSelect);
	pageSetupLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
#else // Q_OS_MAC
	pageSetupBox->setLayout(pageSetupLayout);
#endif // Q_OS_MAC

#ifndef Q_OS_MAC
	QGroupBox *outputFileBox = new QGroupBox(tr("Output file"));
	QVBoxLayout *outputFileLayout = new QVBoxLayout();
	outputFileLayout->addWidget(_fileSelect);
	outputFileBox->setLayout(outputFileLayout);
#endif // Q_OS_MAC

	QDialogButtonBox *buttonBox = new QDialogButtonBox();
	buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this,
	  &PDFExportDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this,
	  &PDFExportDialog::reject);

	QVBoxLayout *layout = new QVBoxLayout;
#ifdef Q_OS_MAC
	layout->addLayout(pageSetupLayout);
#else // Q_OS_MAC
	layout->addWidget(pageSetupBox);
	layout->addWidget(outputFileBox);
#endif // Q_OS_MAC
	layout->addWidget(buttonBox);
	setLayout(layout);

	setWindowTitle(tr("Export to PDF"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void PDFExportDialog::accept()
{
	QString error;
	if (!_fileSelect->checkFile(error)) {
		QMessageBox::warning(this, tr("Error"), error);
		return;
	}

	QPageLayout::Orientation orientation = _portrait->isChecked()
	  ? QPageLayout::Orientation::Portrait : QPageLayout::Orientation::Landscape;
	QPageSize::PageSizeId paperSize = static_cast<QPageSize::PageSizeId>
	  (_paperSize->itemData(_paperSize->currentIndex()).toInt());
	int resolution = _resolution->itemData(_resolution->currentIndex()).toInt();

	_export.fileName = _fileSelect->file();
	_export.paperSize = paperSize;
	_export.resolution = resolution;
	_export.orientation = orientation;
	_export.margins = (_units == Imperial)
	  ? _margins->value() / MM2IN : _margins->value() / MM2CM;

	QDialog::accept();
}
