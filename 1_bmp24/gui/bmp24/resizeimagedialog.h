#ifndef RESIZEIMAGEDIALOG_H
#define RESIZEIMAGEDIALOG_H
#include <QDialog>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "bitmap_util.h"

class ResizeImageDialog : public QDialog
{
    Q_OBJECT
public:
    ResizeImageDialog(WorkMode workMode, QWidget* parent = nullptr);
    int32_t getScaleValue() {
        return this->spinBox->value();
    }
private:
    QLabel *label;
    QSpinBox *spinBox;
    QPushButton *applyButton;
    QPushButton *cancelButton;
};

#endif // RESIZEIMAGEDIALOG_H
