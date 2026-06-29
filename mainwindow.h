#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include "palette_panel.h"
#include "tool_panel.h"
#include "layer_panel.h"
#include "ui_mainwindow.h"

#define IS_DARK_THEME       (this->palette().window().color().black() >= 128)

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    PalettePanel* dckPaletteEdit= nullptr;
    ToolPanel* dckToolPanel= nullptr;
    LayerPanel* dckLayerPanel= nullptr;

    bool _QuickSaveBitmap(QString new_fname);

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    QScrollArea* CanvasContainer() { return ui->scr_canvas; }
    QWidget* CentralWidget() { return ui->centralwidget; }
    PalettePanel** PalettePanelPtr() { return &dckPaletteEdit; }
    ToolPanel** ToolPanelPtr() { return &dckToolPanel; }
    LayerPanel** LayerPanelPtr() { return &dckLayerPanel; }

    void UpdateWindowTitle();

    static QIcon ColorizeIcon(QString filename, QString color);

private slots:
    void on_actionZoom_in_triggered();
    void on_actionZoom_out_triggered();
    void on_actionQuit_triggered();
    void on_actionNewProject_triggered();
    void on_actionOpenProject_triggered();
    void on_actionSaveProject_triggered();
    void on_actionSaveProjectAs_triggered();
    void on_actionPaletteQuickSwap_triggered();
    void on_actionPaletteQuickFill_triggered();
    void on_actionAboutQt_triggered();
    void on_actionAbout_triggered();
    void on_actionUndo_triggered();
    void on_actionRedo_triggered();
    void on_actionTogglePixelGrid_triggered(bool checked);
    void on_actionToggleTileGrid_triggered(bool checked);
    void on_actionResize_triggered();
};

#endif // MAINWINDOW_H
