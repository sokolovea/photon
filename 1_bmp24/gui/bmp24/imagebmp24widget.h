#ifndef IMAGEBMP24WIDGET_H
#define IMAGEBMP24WIDGET_H

#include <QWidget>
#include <QImage>
#include <QScrollArea>
#include <qlabel.h>

class ImageBmp24Widget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageBmp24Widget(const QImage& qImage, QWidget *parent = nullptr);
    void updateImage(const QImage& qImage);
    const QImage& getQImage();

signals:
private:
    QLabel *label;
    QImage internalQImage;
    QScrollArea *scrollArea;
};

#endif // IMAGEBMP24WIDGET_H
