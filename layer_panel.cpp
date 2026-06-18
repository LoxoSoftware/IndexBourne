#include "layer_panel.h"
#include "ui_layer_panel.h"
#include <QMessageBox>

LayerPanel::LayerPanel(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::LayerPanel)
{
    ui->setupUi(this);

    this->main_window= main_window;
}

LayerPanel::~LayerPanel()
{
    delete ui;
}

void LayerPanel::on_lstLayers_currentRowChanged(int currentRow)
{

}

void LayerPanel::on_lstLayers_currentTextChanged(const QString &currentText)
{

}

void LayerPanel::on_btnNew_clicked()
{

}

void LayerPanel::on_btnDelete_clicked()
{

}

void LayerPanel::on_btnMergeDown_clicked()
{

}

