#ifndef APP_H
#define APP_H

#include <QApplication>

class GUI;

class App : QApplication
{
	Q_OBJECT

public:
	App(int &argc, char **argv);
	~App();
	int run();

protected:
	bool event(QEvent *event);

private:
	void loadDatums();
	void loadPCSs();

	GUI *_gui;
};

#endif // APP_H
