#include "layer_panel.h"
#include "ui_layer_panel.h"
#include <QMessageBox>
#include "project.h"

extern Project* current_project;

LayerPanel::LayerPanel(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::LayerPanel)
{
    ui->setupUi(this);

    this->main_window= main_window;

    Update();
}

LayerPanel::~LayerPanel()
{
    delete ui;
}

void LayerPanel::Update()
{
    ui->lstLayers->clear();

    if (!current_project)
        return;

    block_index_updates= true;

    Frame* frame= current_project->CurrentFrame();

    for (int il=frame->LayerCount()-1; il>=0; il--)
    {
        QListWidgetItem* item= new QListWidgetItem("Layer "+QString::number(il), ui->lstLayers);
        item->setIcon(QIcon(QPixmap::fromImage(frame->Layer(il)->copy())));

        ui->lstLayers->addItem(item);
    }

    ui->lstLayers->setCurrentRow(LayerIndex(current_project->CurrentLayerIndex()));

    block_index_updates= false;
}

int LayerPanel::LayerIndex(int row)
{
    if (row < 0)
        return - 1;

    return ui->lstLayers->count() - row - 1;
}

int LayerPanel::CurrentLayerIndex()
{
    return LayerIndex(ui->lstLayers->currentRow());
}

void LayerPanel::on_lstLayers_currentRowChanged(int currentRow)
{
    if (block_index_updates)
        return;
    if (!current_project)
        return;
    if (currentRow < 0)
        return;

    current_project->SetCurrentLayerIndex(CurrentLayerIndex());
    Update();
}

void LayerPanel::on_lstLayers_currentTextChanged(const QString &currentText)
{

}

void LayerPanel::on_btnNew_clicked()
{
    if (!current_project)
        return;

    current_project->CurrentFrame()->InsertLayerAt(CurrentLayerIndex()+1);
    current_project->SetCurrentLayerIndex(CurrentLayerIndex()+1);

    Update();
}

void LayerPanel::on_btnDelete_clicked()
{
    if (!current_project)
        return;

    current_project->CurrentFrame()->RemoveLayer(CurrentLayerIndex());
    if (CurrentLayerIndex()-1 > 0)
        current_project->SetCurrentLayerIndex(CurrentLayerIndex()-1);
    else
        current_project->SetCurrentLayerIndex(0);

    Update();
}

void LayerPanel::on_btnMergeDown_clicked()
{

}

