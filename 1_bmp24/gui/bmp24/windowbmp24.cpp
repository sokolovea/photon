#include <QFileDialog>
#include <QMessageBox>
#include <QCoreApplication>
#include <QWidget>
#include <QApplication>
#include <QIcon>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRandomGenerator>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QImage>
#include <QGroupBox>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QToolBar>
#include <QStatusBar>
#include <QSpinBox>

#include "windowbmp24.h"
#include "resizeimagedialog.h"
#include "bitmap.h"
#include "bitmap_util.h"
#include "imagebmp24widget.h"


WindowBmp24::WindowBmp24(QWidget *parent)
    : QMainWindow{parent}
{
    this->qMdiArea = new QMdiArea(this);
    this->qMdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->qMdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    this->setCentralWidget(qMdiArea);
    toolBar = addToolBar("Инструменты");

    buttonOpenImage = new QPushButton(QIcon(":/resourses/open.png"), "", this);
    buttonOpenImage->setToolTip("Открыть");
    buttonOpenImage->setFixedSize(50, 50);
    buttonOpenImage->setIconSize(QSize(44, 44));
    connect(buttonOpenImage, &QPushButton::clicked, this, &WindowBmp24::onOpenImage);
    toolBar->addWidget(buttonOpenImage);

    buttonSaveImage = new QPushButton(QIcon(":/resourses/save.png"), "", this);
    buttonSaveImage->setToolTip("Сохранить");
    buttonSaveImage->setFixedSize(50, 50);
    buttonSaveImage->setIconSize(QSize(40, 40));
    connect(buttonSaveImage, &QPushButton::clicked, this, &WindowBmp24::onSaveImage);

    toolBar->addWidget(buttonSaveImage);
    toolBar->addSeparator();

    buttonIncreaseScale = new QPushButton(QIcon(":/resourses/zoomx2.png"), "", this);
    buttonIncreaseScale->setToolTip("Увеличить");
    buttonIncreaseScale->setFixedSize(50, 50);
    buttonIncreaseScale->setIconSize(QSize(44, 44));
    connect(buttonIncreaseScale, &QPushButton::clicked, this, &WindowBmp24::onIncreaseScale);
    toolBar->addWidget(buttonIncreaseScale);

    buttonDecreaseScale = new QPushButton(QIcon(":/resourses/zoomx0dot5.png"), "", this);
    buttonDecreaseScale->setToolTip("Уменьшить");
    buttonDecreaseScale->setFixedSize(50, 50);
    buttonDecreaseScale->setIconSize(QSize(44, 44));
    connect(buttonDecreaseScale, &QPushButton::clicked, this, &WindowBmp24::onDecreaseScale);
    toolBar->addWidget(buttonDecreaseScale);

    buttonCustomDecreaseScale = new QPushButton(QIcon(":/resourses/zoom_minus.png"), "", this);

    buttonCustomIncreaseScale = new QPushButton(QIcon(":/resourses/zoom_plus.png"), "", this);
    buttonCustomIncreaseScale->setToolTip("Увеличить масштаб");
    buttonCustomIncreaseScale->setFixedSize(50, 50);
    buttonCustomIncreaseScale->setIconSize(QSize(44, 44));
    connect(buttonCustomIncreaseScale, &QPushButton::clicked, this, &WindowBmp24::onCustomIncreaseScale);
    toolBar->addWidget(buttonCustomIncreaseScale);

    buttonCustomDecreaseScale->setToolTip("Уменьшить масштаб");
    buttonCustomDecreaseScale->setFixedSize(50, 50);
    buttonCustomDecreaseScale->setIconSize(QSize(44, 44));
    buttonCustomDecreaseScale->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    connect(buttonCustomDecreaseScale, &QPushButton::clicked, this, &WindowBmp24::onCustomDecreaseScale);
    toolBar->addWidget(buttonCustomDecreaseScale);


    fileMenu = menuBar()->addMenu("Файл");
    actionOpen = new QAction("Открыть", this);
    connect(actionOpen, &QAction::triggered, this, &WindowBmp24::onOpenImage);
    fileMenu->addAction(actionOpen);

    actionSaveAs = new QAction("Сохранить как...", this);
    connect(actionSaveAs, &QAction::triggered, this, &WindowBmp24::onSaveImage);
    fileMenu->addAction(actionSaveAs);
    fileMenu->addSeparator();
    actionExit = new QAction("Выход", this);
    connect(actionExit, &QAction::triggered, this, &QWidget::close);
    fileMenu->addAction(actionExit);

    editMenu = menuBar()->addMenu("Правка");
    actionIncreaseScale = new QAction("Увеличение масштаба", this);
    connect(actionIncreaseScale, &QAction::triggered, this, &WindowBmp24::onIncreaseScale);
    editMenu->addAction(actionIncreaseScale);

    actionDecreaseScale = new QAction("Уменьшение масштаба", this);
    connect(actionDecreaseScale, &QAction::triggered, this, &WindowBmp24::onDecreaseScale);
    editMenu->addAction(actionDecreaseScale);

    actionCustomIncreaseScale = new QAction("Кастомное увеличение масштаба", this);
    connect(actionCustomIncreaseScale, &QAction::triggered, this, &WindowBmp24::onCustomIncreaseScale);
    editMenu->addAction(actionCustomIncreaseScale);

    actionCustomDecreaseScale = new QAction("Кастомное уменьшение масштаба", this);
    connect(actionCustomDecreaseScale, &QAction::triggered, this, &WindowBmp24::onCustomDecreaseScale);
    editMenu->addAction(actionCustomDecreaseScale);

    windowMenu = menuBar()->addMenu("Окно");

    actionCascade = new QAction("Каскад", windowMenu);
    actionCascade->setCheckable(true);
    actionCascade->setChecked(true);
    windowMenu->addAction(actionCascade);

    actionMosaic = new QAction("Мозаика", windowMenu);
    actionMosaic->setCheckable(true);
    windowMenu->addAction(actionMosaic);

    actionTabs = new QAction("Вкладки", windowMenu);
    actionTabs->setCheckable(true);
    windowMenu->addAction(actionTabs);

    connect(actionCascade, &QAction::triggered, this, &WindowBmp24::onCascade);
    connect(actionMosaic, &QAction::triggered, this, &WindowBmp24::onMosaic);
    connect(actionTabs, &QAction::triggered, this, &WindowBmp24::onTabs);

    helpMenu = menuBar()->addMenu("Справка");
    actionAbout = new QAction("О программе", this);
    connect(actionAbout, &QAction::triggered, this, &WindowBmp24::showAbout);
    helpMenu->addAction(actionAbout);

    statusBar = new QStatusBar(this);
    qLabelStatusBar = new QLabel();
    statusBar->addWidget(this->qLabelStatusBar);
    this->setStatusBar(statusBar);
    updateMenusStatusBarAndWindows();
    this->setMinimumSize(800, 600);
}

OperationStatus WindowBmp24::loadImageFromFileDialog(QImage& qImage, QString& fileName) {
    fileName = QFileDialog::getOpenFileName(nullptr, "Открыть изображение", "", "Изображение (*.bmp)");
    if (!fileName.isEmpty()) {
        Bitmap24Image bitmap24Image;
        if (readImageHeadersToStructure(fileName.toLocal8Bit().toStdString(), bitmap24Image) == OperationStatus::Failed) {
            return OperationStatus::Failed;
        }
        qImage = QImage(bitmap24Image.bitmapInfoHeaderV3.biWidth, bitmap24Image.bitmapInfoHeaderV3.biHeight, QImage::Format_BGR888);
        readImagePixelsToBuffer(fileName.toLocal8Bit().toStdString(), qImage.bits(), bitmap24Image);
        if (qImage.isNull()) {
            return OperationStatus::Failed;
        }
        return OperationStatus::Success;
    }
    return OperationStatus::Alternative;
}

Bitmap24Image getBitmap24ImageFromQImage(const QImage& qImage) {
    Bitmap24Image bitmap24Image;
    int rowSizeWithoutPadding = qImage.width() * 3;
    int rowSizeWithPadding = getRowSizeWithPadding(rowSizeWithoutPadding);
    int imageSize = rowSizeWithPadding * qImage.height();

    bitmap24Image.bitmapFileHeader.bfType = 0x4d42;
    bitmap24Image.bitmapFileHeader.bfSize = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3) + imageSize;
    bitmap24Image.bitmapFileHeader.bfReserved1 = 0;
    bitmap24Image.bitmapFileHeader.bfReserved2 = 0;
    bitmap24Image.bitmapFileHeader.bfOffBits = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeaderV3);

    bitmap24Image.bitmapInfoHeaderV3.biWidth = qImage.width();
    bitmap24Image.bitmapInfoHeaderV3.biHeight = qImage.height();
    bitmap24Image.bitmapInfoHeaderV3.biSize = sizeof(BitmapInfoHeaderV3);
    bitmap24Image.bitmapInfoHeaderV3.biPlanes = 1;
    bitmap24Image.bitmapInfoHeaderV3.biBitCount = 24;
    bitmap24Image.bitmapInfoHeaderV3.biCompression = 0;
    bitmap24Image.bitmapInfoHeaderV3.biSizeImage = imageSize;
    bitmap24Image.bitmapInfoHeaderV3.biXPelsPerMeter = 2835;
    bitmap24Image.bitmapInfoHeaderV3.biYPelsPerMeter = 2835;
    bitmap24Image.bitmapInfoHeaderV3.biClrUsed = 0;
    bitmap24Image.bitmapInfoHeaderV3.biClrImportant = 0;

    return bitmap24Image;
}


OperationStatus WindowBmp24::saveImageToFile() {
    QImage qImage;
    if (!getQImageFromCurrentSubWindow(qImage)) {
        QMessageBox::warning(nullptr, "Предупреждение", "Нет открытого изображения для сохранения!", QMessageBox::Ok);
        return OperationStatus::Alternative;
    }
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить изображение", "", "Изображение (*.bmp)");
    if (!fileName.isEmpty()) {
        Bitmap24Image bitmap24Image = getBitmap24ImageFromQImage(qImage);
        OperationStatus status = writeImageStructureToFile(bitmap24Image, (uint8_t*)qImage.bits(), fileName.toLocal8Bit().toStdString());
        if (status == OperationStatus::Success) {
            getCurrentImageWidget()->setWindowTitle(fileName);
            return status;
        }
    } else {
        return OperationStatus::Alternative;
    }
    QMessageBox::critical(nullptr, "Ошибка", "Не удалось сохранить изображение!", QMessageBox::Ok);
    return OperationStatus::Failed;
}

ImageBmp24Widget* WindowBmp24::getCurrentImageWidget() {
    QMdiSubWindow *currentSubWindow = qMdiArea->currentSubWindow();
    if (currentSubWindow) {
        ImageBmp24Widget *imageWidget = dynamic_cast<ImageBmp24Widget*>(currentSubWindow->widget());
        return imageWidget;
    }
    return nullptr;
}

bool WindowBmp24::getQImageFromCurrentSubWindow(QImage& qImage) {
    ImageBmp24Widget *imageWidget = getCurrentImageWidget();
    if (imageWidget) {
        qImage = imageWidget->getQImage();
    }
    return (imageWidget != nullptr);
}


OperationStatus WindowBmp24::resizeImage(WorkMode workMode, uint64_t coeff) {
    QImage inputQImage;
    if (!getQImageFromCurrentSubWindow(inputQImage)) {
        QMessageBox::warning(nullptr, "Предупреждение", "Нет открытого изображения для редактирования!", QMessageBox::Ok);
        return OperationStatus::Alternative;
    }

    int32_t outputWidthPx = workMode == WorkMode::INCREASE? inputQImage.width() * coeff : divideWithCeil(inputQImage.width(), coeff);
    int32_t outputHeightPx = workMode == WorkMode::INCREASE? inputQImage.height() * coeff : divideWithCeil(inputQImage.height(), coeff);
    QImage outputQImage(outputWidthPx, outputHeightPx, QImage::Format_BGR888);

    if (workMode == WorkMode::INCREASE) {
        for (int i = 0; i < inputQImage.height(); i++) {
            auto inputRow = inputQImage.scanLine(i);
            for (int32_t duplication = 0; duplication < coeff; duplication++) {
                auto outputRow = outputQImage.scanLine(i * coeff + duplication);
                increaseResolution((Bitmap24Pixel*)inputRow, (Bitmap24Pixel*)outputRow, inputQImage.width(), outputQImage.width() * coeff, coeff);
            }
        }
    } else if (workMode == WorkMode::DECREASE) {
        for (uint32_t i = 0; i < outputHeightPx; i++) {
            auto inputRow = inputQImage.scanLine(i * coeff);
            auto outputRow = outputQImage.scanLine(i);
            decreaseResolution((Bitmap24Pixel*)inputRow, (Bitmap24Pixel*)outputRow, inputQImage.width(), outputWidthPx, coeff);
        }
    }
    getCurrentImageWidget()->updateImage(outputQImage);
    return OperationStatus::Success;
}

void WindowBmp24::customResizeImage(WorkMode workMode) {
    if (getCurrentImageWidget()) {
        auto resizeDialog = new ResizeImageDialog(workMode, this);
        if (resizeDialog->exec() == QDialog::Accepted) {
            resizeImage(workMode, resizeDialog->getScaleValue());
        }
    } else {
        QMessageBox::warning(nullptr, "Предупреждение", "Нет открытого изображения для редактирования!", QMessageBox::Ok);
    }
}


OperationStatus WindowBmp24::createNewSubWindow() {
    QImage qImage;
    QString fileName;
    OperationStatus loadResult = loadImageFromFileDialog(qImage, fileName);
    if (loadResult == OperationStatus::Failed) {
        QMessageBox::critical(nullptr, "Ошибка", "Не удалось загрузить изображение!", QMessageBox::Ok);
        return loadResult;
    }
    if (loadResult == OperationStatus::Alternative) {
        return loadResult;
    }
    QSize mdiAreaSize = qMdiArea->viewport()->size();
    auto imageBmp24Widget = new ImageBmp24Widget(qImage);
    QMdiSubWindow* subWindow = this->qMdiArea->addSubWindow(imageBmp24Widget);
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    connect(subWindow, &QMdiSubWindow::destroyed, this, &WindowBmp24::updateMenusStatusBarAndWindows);
    subWindow->setWindowTitle(fileName);
    QSize minSize =subWindow->minimumSizeHint();
    subWindow->resize(std::min(std::max(qImage.width(), minSize.width()),
                               mdiAreaSize.width() * 8 / 10),
                      std::min(std::max(qImage.height(), minSize.height()),
                               mdiAreaSize.height() * 8 / 10));
    // subWindow->resize(std::min(qImage.width() + minSize.width(),
    //                            mdiAreaSize.width() * 8 / 10),
    //                   std::min(qImage.height() + minSize.height(),
    //                            mdiAreaSize.height() * 8 / 10));
    subWindow->show();
    return OperationStatus::Success;
}

void WindowBmp24::updateMenusStatusBarAndWindows()
{
    int windowCount = qMdiArea->subWindowList().size();
    this->qLabelStatusBar->setText(QString("Открыто окон: %1").arg(windowCount));
    if (windowCount == 0) {
        buttonIncreaseScale->setDisabled(true);
        buttonDecreaseScale->setDisabled(true);
        buttonCustomIncreaseScale->setDisabled(true);
        buttonCustomDecreaseScale->setDisabled(true);
        editMenu->setDisabled(true);
    } else {
        buttonIncreaseScale->setEnabled(true);
        buttonDecreaseScale->setEnabled(true);
        buttonCustomIncreaseScale->setEnabled(true);
        buttonCustomDecreaseScale->setEnabled(true);
        editMenu->setEnabled(true);
    }
    if (actionMosaic->isChecked()) {
        this->qMdiArea->tileSubWindows();
    }
}

/*Slots*/

void WindowBmp24::onCascade() {
    checkAction();
    this->qMdiArea->setViewMode(QMdiArea::SubWindowView);
    this->qMdiArea->cascadeSubWindows();

    QList<QMdiSubWindow*> subWindows = this->qMdiArea->findChildren<QMdiSubWindow*>();
    QSize mdiAreaSize = qMdiArea->viewport()->size();
    for (QMdiSubWindow* subWindow : subWindows) {
        ImageBmp24Widget* subWindowWidget = qobject_cast<ImageBmp24Widget*>(subWindow->widget());
        if (subWindowWidget) {
            QSize minSize =subWindow->minimumSizeHint();
            subWindow->resize(std::min(std::max(subWindowWidget->getQImage().width(), minSize.width()),
                                       mdiAreaSize.width() * 8 / 10),
                              std::min(std::max(subWindowWidget->getQImage().height(), minSize.height()),
                                       mdiAreaSize.height() * 8 / 10));
        }
    }
}

void WindowBmp24::onOpenImage() {
    createNewSubWindow();
    updateMenusStatusBarAndWindows();
}

void WindowBmp24::onSaveImage() {
    saveImageToFile();
}

void WindowBmp24::onIncreaseScale() {
    resizeImage(WorkMode::INCREASE, 2);
}

void WindowBmp24::onDecreaseScale() {
    resizeImage(WorkMode::DECREASE, 2);
}

void WindowBmp24::onCustomIncreaseScale() {
    customResizeImage(WorkMode::INCREASE);
}

void WindowBmp24::onCustomDecreaseScale() {
    customResizeImage(WorkMode::DECREASE);
}

void WindowBmp24::showAbout() {
    QMessageBox::information(nullptr, "О программе", "Учебная программа по масштабированию bmp24\nРазработчик: Соколов Егор");
}

void WindowBmp24::onMosaic() {
    checkAction();
    this->qMdiArea->setViewMode(QMdiArea::SubWindowView);
    this->qMdiArea->tileSubWindows();
}

void WindowBmp24::onTabs() {
    checkAction();
    this->qMdiArea->setViewMode(QMdiArea::TabbedView);
    this->qMdiArea->setTabsClosable(true);
    this->qMdiArea->setTabsMovable(true);
}

void WindowBmp24::checkAction() {
    deleteChecks();
    QAction *senderAction = qobject_cast<QAction*>(sender());
    if (senderAction) {
        senderAction->setChecked(true);
    }
}

void WindowBmp24::deleteChecks() {
    QList<QAction*> actions = this->windowMenu->findChildren<QAction*>();
    {
        for (QAction *action : actions) {
            action->setChecked(false);
        }
    }
}
