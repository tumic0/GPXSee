#include "common/config.h"
#include "GUI/app.h"

int main(int argc, char *argv[])
{
#ifdef ENABLE_HIDPI
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif // ENABLE_HIDPI

	App app(argc, argv);
	return app.run();
}
