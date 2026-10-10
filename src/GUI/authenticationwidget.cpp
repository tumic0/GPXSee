#include <QFormLayout>
#include "macos.h"
#include "authenticationwidget.h"

AuthenticationWidget::AuthenticationWidget(QWidget *parent) : QWidget(parent)
{
	bool macos = MacOS::match(style());

	_username = new QLineEdit();
	_password = new PasswordEdit();

	if (macos) {
		// A hack to fix the issue with different field sizes
		_username->setMinimumWidth(150);
		_password->setMinimumWidth(150);
	}

	QFormLayout *layout = new QFormLayout();
	layout->addRow(tr("Username:"), _username);
	layout->addRow(tr("Password:"), _password);
	// Workaround for cut off border Qt mac style bug
	if (macos)
		layout->setContentsMargins(0, 0, 5, 0);
	else
		layout->setContentsMargins(QMargins());

	setLayout(layout);
}
