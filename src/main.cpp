#include "GUI/app.h"
#include "config.h"

int main(int argc, char *argv[])
{
#ifdef ENABLE_HIDPI
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif // ENABLE_HIDPI

	App app(argc, argv);
	app.run();

	return 0;
}
