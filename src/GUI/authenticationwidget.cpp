#include <QFormLayout>
#include "authenticationwidget.h"

AuthenticationWidget::AuthenticationWidget(QWidget *parent) : QWidget(parent)
{
	_username = new QLineEdit();
	_password = new PasswordEdit();
#ifdef Q_OS_MAC
	/* A hack to fix the issue with different field sizes on Mac */
	_username->setMinimumWidth(150);
	_password->setMinimumWidth(150);
#endif // Q_OS_MAC

	QFormLayout *layout = new QFormLayout();
	layout->addRow(tr("Username:"), _username);
	layout->addRow(tr("Password:"), _password);
	layout->setContentsMargins(0, 0, 0, 0);

	setLayout(layout);
}
