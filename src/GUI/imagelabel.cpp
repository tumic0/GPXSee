#include <QDesktopServices>
#include <QFileInfo>
#include <QImageReader>
#include <QLabel>
#include <QMouseEvent>
#include "imagelabel.h"

ImageLabel::ImageLabel(QString path, QSize size, QWidget* parent, Qt::WindowFlags f)
    : QLabel(parent, f), _path(path) {
	QImageReader reader(path);
	reader.setAutoTransform(true);
	reader.setScaledSize(size);
	QImage image = reader.read();
	QPixmap pixmap = QPixmap::fromImage(image);
	setPixmap(pixmap);
}

ImageLabel::~ImageLabel() {}

void ImageLabel::mousePressEvent(QMouseEvent* event) {
	QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo(_path).absoluteFilePath()));
}
