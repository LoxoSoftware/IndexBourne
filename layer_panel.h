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

private slots:
    void on_lstLayers_currentRowChanged(int currentRow);
    void on_lstLayers_currentTextChanged(const QString &currentText);
    void on_btnNew_clicked();
    void on_btnDelete_clicked();
    void on_btnMergeDown_clicked();

private:
    Ui::LayerPanel *ui;

    MainWindow* main_window;
};

#endif // LAYER_PANEL_H
