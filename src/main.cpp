#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include "gui.h"
#include "icons.h"


int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	QString locale = QLocale::system().name();
	QTranslator translator;
	translator.load(QString(":/lang/gpxsee_") + locale);
	app.installTranslator(&translator);
#ifdef Q_OS_MAC
	app.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif // Q_OS_MAC

	GUI gui;
	gui.setWindowIcon(QIcon(QPixmap(APP_ICON)));
	gui.show();

	for (int i = 1; i < argc; i++)
		gui.openFile(QString::fromLocal8Bit(argv[i]));

	return app.exec();
}
