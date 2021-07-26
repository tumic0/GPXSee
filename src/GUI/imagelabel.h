#ifndef IMAGELABEL_H
#define IMAGELABEL_H

class ImageLabel : public QLabel {
public:
    ImageLabel(QString path, QSize size, QWidget* parent = 0, Qt::WindowFlags f = Qt::WindowFlags());
    ~ImageLabel();

protected:
    void mousePressEvent(QMouseEvent* event);

	QString path;

};


#endif // IMAGELABEL_H
