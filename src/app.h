#ifndef APP_H
#define APP_H

#include <QApplication>

class GUI;
class QTranslator;

class App : QApplication
{
	Q_OBJECT

public:
	App(int &argc, char **argv);
	~App();
	void run();

protected:
	bool event(QEvent *event);

private:
	int &_argc;
	char **_argv;
	GUI *_gui;
	QTranslator *_translator;
};

#endif // APP_H
