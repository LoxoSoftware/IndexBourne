#ifndef LAYER_PANEL_H
#define LAYER_PANEL_H

#include <QDockWidget>
#include <QModelIndex>
#include <QTableWidget>
#include <QMessageBox>

class MainWindow;

class LayerTable : public QTableWidget
{
public:
    LayerTable(QWidget* parent) { this->setParent(parent); }

    void Redraw();

    int LayerIndex(int row);
    int CurrentLayerIndex();

private:
    int drag_src_index= -1;
    bool mouse_moving= false;

    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void dropEvent(QDropEvent* event);

    void on_currentCellChanged(int, int, int, int);
    void on_layerVisibilityChanged(bool visible);
};

namespace Ui {
class LayerPanel;
}

class LayerPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit LayerPanel(QWidget *parent, MainWindow* main_window);
    ~LayerPanel();

    void Update();


private slots:
    void on_btnNew_clicked();
    void on_btnDelete_clicked();
    void on_btnMergeDown_clicked();

private:
    Ui::LayerPanel *ui;

    MainWindow* main_window;
};

#endif // LAYER_PANEL_H
