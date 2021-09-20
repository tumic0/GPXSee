#include <QAction>
#include "icons.h"
#include "passwordedit.h"

PasswordEdit::PasswordEdit(QWidget *parent) : QLineEdit(parent)
{
	_show = false;
	_action = addAction(QIcon(SHOW_PWD_ICON), QLineEdit::TrailingPosition);
	connect(_action, &QAction::triggered, this, &PasswordEdit::showPassword);
	setEchoMode(QLineEdit::Password);
}

void PasswordEdit::showPassword()
{
	if (_show) {
		_action->setIcon(QIcon(SHOW_PWD_ICON));
		setEchoMode(QLineEdit::Password);
		_show = false;
	} else {
		_action->setIcon(QIcon(HIDE_PWD_ICON));
		setEchoMode(QLineEdit::Normal);
		_show = true;
	}
}
