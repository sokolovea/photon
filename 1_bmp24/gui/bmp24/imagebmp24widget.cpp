#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include "imagebmp24widget.h"

ImageBmp24Widget::ImageBmp24Widget(const QImage& qImage, QWidget *parent)
    : QWidget(parent), label(new QLabel(this)), scrollArea(new QScrollArea(this))
{
        updateImage(qImage);
        label->setScaledContents(false);
        this->label->setAlignment(Qt::AlignCenter);
        this->label->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);

        scrollArea->setWidget(label);
        scrollArea->setWidgetResizable(true);
        scrollArea->setAlignment(Qt::AlignCenter);
        scrollArea->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

        QVBoxLayout *layout = new QVBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(scrollArea);
        layout->setAlignment(Qt::AlignCenter);
        this->setLayout(layout);
        this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ImageBmp24Widget::updateImage(const QImage& qImage)
{
    if (qImage.format() == QImage::Format_BGR888) {
        this->label->setPixmap(QPixmap::fromImage(qImage.mirrored(false, true)));
        this->internalQImage = qImage;
    } else {
        auto convertedImage = qImage.convertToFormat(QImage::Format_BGR888);
        this->label->setPixmap(QPixmap::fromImage(convertedImage.mirrored(false, true)));
        this->internalQImage = convertedImage;
    }
    this->label->setMinimumSize(qImage.size());
    this->label->setMaximumSize(qImage.size());
    this->label->adjustSize();
}

const QImage& ImageBmp24Widget::getQImage()
{
    return this->internalQImage;
}


