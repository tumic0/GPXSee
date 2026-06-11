#include <QVBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QTabWidget>
#include <QCheckBox>
#include "units.h"
#include "fileselectwidget.h"
#include "marginswidget.h"
#include "macos.h"
#include "pngexportdialog.h"


PNGExportDialog::PNGExportDialog(PNGExport &exp, QWidget *parent)
  : QDialog(parent), _export(exp)
{
#ifdef Q_OS_ANDROID
	setWindowFlags(Qt::Window);
	setWindowState(Qt::WindowFullScreen);
#endif /* Q_OS_ANDROID */

	bool macos = MacOS::match(style());

	_fileSelect = new FileSelectWidget();
#ifndef Q_OS_ANDROID
	_fileSelect->setFilter(tr("PNG files") + " (*.png);;" + tr("All files")
	  + " (*)");
	_fileSelect->setFile(_export.fileName);
#endif // Q_OS_ANDROID

	_width = new QSpinBox();
	_width->setMinimum(256);
	_width->setMaximum(4096);
	_width->setValue(_export.size.width());
	_width->setSuffix(UNIT_SPACE + tr("px"));
	_height = new QSpinBox();
	_height->setMinimum(256);
	_height->setMaximum(4096);
	_height->setValue(_export.size.height());
	_height->setSuffix(UNIT_SPACE + tr("px"));

	_margins = new MarginsWidget();
	_margins->setValue(_export.margins);
	_margins->setUnits(tr("px"));

	_antialiasing = new QCheckBox(tr("Use anti-aliasing"));
	_antialiasing->setChecked(_export.antialiasing);

	QFormLayout *pageSetupLayout = new QFormLayout;
	pageSetupLayout->addRow(tr("Image width:"), _width);
	pageSetupLayout->addRow(tr("Image height:"), _height);
	pageSetupLayout->addRow(tr("Margins:"), _margins);
	pageSetupLayout->addWidget(_antialiasing);
	if (macos) {
		pageSetupLayout->addRow(MacOS::line());
		pageSetupLayout->addRow(tr("File:"), _fileSelect);
		pageSetupLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
	}

	QDialogButtonBox *buttonBox = new QDialogButtonBox();
	buttonBox->addButton(tr("Export"), QDialogButtonBox::AcceptRole);
	buttonBox->addButton(QDialogButtonBox::Cancel);
	connect(buttonBox, &QDialogButtonBox::accepted, this,
	  &PNGExportDialog::accept);
	connect(buttonBox, &QDialogButtonBox::rejected, this,
	  &PNGExportDialog::reject);

	QVBoxLayout *layout = new QVBoxLayout;
	if (macos)
		layout->addLayout(pageSetupLayout);
	else {
		QGroupBox *pageSetupBox = new QGroupBox(tr("Image Setup"));
		pageSetupBox->setLayout(pageSetupLayout);
		QGroupBox *outputFileBox = new QGroupBox(tr("Output file"));
		QVBoxLayout *outputFileLayout = new QVBoxLayout();
		outputFileLayout->addWidget(_fileSelect);
		outputFileBox->setLayout(outputFileLayout);
		layout->addWidget(pageSetupBox);
		layout->addWidget(outputFileBox);
	}
#ifdef Q_OS_ANDROID
	layout->addStretch();
#endif // Q_OS_ANDROID
	layout->addWidget(buttonBox);
	setLayout(layout);

	setWindowTitle(tr("Export to PNG"));
	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

void PNGExportDialog::accept()
{
	QString error;
	if (!_fileSelect->checkFile(error)) {
		QMessageBox::warning(this, tr("Error"), error);
		return;
	}

	_export.fileName = _fileSelect->file();
	_export.size = QSize(_width->value(), _height->value());
	_export.margins = _margins->value();
	_export.antialiasing = _antialiasing->isChecked();

	QDialog::accept();
}
