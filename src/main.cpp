//#include "GUI/timezoneinfo.h"
#include "GUI/app.h"

int main(int argc, char *argv[])
{
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
//	qRegisterMetaTypeStreamOperators<TimeZoneInfo>("TimeZoneInfo");

	App app(argc, argv);
	return app.run();
}
