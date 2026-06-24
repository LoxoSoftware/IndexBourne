#ifndef LAYER_PANEL_H
#define LAYER_PANEL_H

#include <QDockWidget>
#include <QModelIndex>

class MainWindow;

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
    void on_lstLayers_currentCellChanged(int, int, int, int);
    void on_btnNew_clicked();
    void on_btnDelete_clicked();
    void on_btnMergeDown_clicked();

private:
    Ui::LayerPanel *ui;

    MainWindow* main_window;
    bool block_index_updates= false;
    bool block_redraw= false;

    int LayerIndex(int row);
    int CurrentLayerIndex();
};

#endif // LAYER_PANEL_H
