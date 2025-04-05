#ifndef WINDOWBMP24_H
#define WINDOWBMP24_H

#include <QMainWindow>
#include <QLabel>
#include <QList>
#include <QMdiArea>
#include <QPushButton>
#include "imagebmp24widget.h"
#include "bitmap_util.h"
#include "operationstatus.h"

class WindowBmp24 : public QMainWindow
{
    Q_OBJECT
    QMdiArea *qMdiArea;
    QLabel *qLabelStatusBar;

public:
    explicit WindowBmp24(QWidget *parent = nullptr);

private slots:
    OperationStatus createNewSubWindow();
    OperationStatus loadImageFromFileDialog(QImage& qImage, QString& fileName);
    OperationStatus saveImageToFile();
    OperationStatus resizeImage(WorkMode workMode, uint64_t coeff);

    void onOpenImage();
    void onSaveImage();
    void onIncreaseScale();
    void onDecreaseScale();
    void onCustomIncreaseScale();
    void onCustomDecreaseScale();
    void showAbout();

    void onCascade();
    void onMosaic();
    void onTabs();
    void checkAction();
private:

    bool getQImageFromCurrentSubWindow(QImage& qImage);
    void resizeImageGUI(WorkMode workMode, uint64_t coeff);
    ImageBmp24Widget *getCurrentImageWidget();
    void customResizeImage(WorkMode workMode);
    void updateMenusStatusBarAndWindows();
    void deleteChecks();

    /* --- */
    QToolBar *toolBar;
    QPushButton *buttonOpenImage;
    QPushButton *buttonSaveImage;
    QPushButton *buttonIncreaseScale;
    QPushButton *buttonDecreaseScale;
    QPushButton *buttonCustomDecreaseScale;
    QPushButton *buttonCustomIncreaseScale;
    QMenu *fileMenu;
    QAction *actionOpen;
    QAction *actionSaveAs;
    QAction *actionExit;
    QMenu *editMenu;
    QAction *actionIncreaseScale;
    QAction *actionDecreaseScale;
    QAction *actionCustomIncreaseScale;
    QAction *actionCustomDecreaseScale;
    QMenu *windowMenu;
    QAction *actionCascade;
    QAction *actionMosaic;
    QAction *actionTabs;
    QMenu *helpMenu;
    QAction *actionAbout;
    QStatusBar *statusBar;
};

#endif // WINDOWBMP24_H
