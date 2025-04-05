#include "resizeimagedialog.h"

ResizeImageDialog::ResizeImageDialog(WorkMode workMode, QWidget* parent) : QDialog(parent) {
    QString actionMessage = workMode == WorkMode::INCREASE ? "увеличения масштаба:" : "уменьшения масштаба:";
    label = new QLabel("Задайте коэффициент для " + actionMessage, this);
    label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    spinBox = new QSpinBox(this);
    spinBox->setRange(1, 10);
    spinBox->setValue(3);
    spinBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    applyButton = new QPushButton("Применить", this);
    connect(applyButton, &QPushButton::clicked, this, &ResizeImageDialog::accept);

    cancelButton = new QPushButton("Отмена", this);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);

    QVBoxLayout *windowLayout = new QVBoxLayout(this);
    QVBoxLayout *buttonsLayout = new QVBoxLayout(this);
    QHBoxLayout *spinBoxLayout = new QHBoxLayout(this);
    buttonsLayout->setAlignment(Qt::AlignRight);
    buttonsLayout->addWidget(applyButton);
    buttonsLayout->addWidget(cancelButton);
    spinBoxLayout->addWidget(label);
    spinBoxLayout->addWidget(spinBox);
    windowLayout->addLayout(spinBoxLayout);
    windowLayout->addLayout(buttonsLayout);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setLayout(windowLayout);
    setWindowTitle("Изменение масштаба изображения");
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
}
