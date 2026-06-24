#include "layer_panel.h"
#include "ui_layer_panel.h"
#include <QMessageBox>
#include "project.h"
#include <QLabel>

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
    if (block_redraw)
        return;

    //Deallocate all widgets
    for (int iy=0; iy<ui->lstLayers->rowCount(); iy++)
        for (int ix=0; ix<ui->lstLayers->columnCount(); ix++)
            if (ui->lstLayers->cellWidget(iy, ix))
                delete ui->lstLayers->cellWidget(iy, ix);

    ui->lstLayers->clear();

    if (!current_project)
        return;

    block_index_updates= true;

    Frame* frame= current_project->CurrentFrame();

    ui->lstLayers->setRowCount(frame->LayerCount());

    for (int il=0; il<frame->LayerCount(); il++)
    {
        int irow= frame->LayerCount()-il-1;
        //QListWidgetItem* item= new QListWidgetItem("Layer "+QString::number(il), ui->lstLayers);
        QPushButton* btn_visible= new QPushButton("V");
        btn_visible->setCheckable(true);
        btn_visible->setChecked(true);
        btn_visible->setToolTip("Visible?");
        QLabel* lbl_name= new QLabel("layer "+QString::number(il));
        QIcon icn_preview= QIcon(QPixmap::fromImage(frame->Layer(il)->copy().transformed(QTransform::fromScale(48,48))));
        //item->setIcon(QIcon(QPixmap::fromImage(frame->Layer(il)->copy())));
        ui->lstLayers->setCellWidget(irow, 0, btn_visible);
        ui->lstLayers->setItem(irow, 1, new QTableWidgetItem(icn_preview, ""));
        ui->lstLayers->setCellWidget(irow, 2, lbl_name);

        //ui->lstLayers->addItem(item);
    }

    //ui->lstLayers->setCurrentRow(LayerIndex(current_project->CurrentLayerIndex()));
    ui->lstLayers->setCurrentCell(LayerIndex(current_project->CurrentLayerIndex()), 0);

    block_index_updates= false;
}

int LayerPanel::LayerIndex(int row)
{
    if (row < 0)
        return - 1;

    return ui->lstLayers->rowCount() - row - 1;
}

int LayerPanel::CurrentLayerIndex()
{
    return LayerIndex(ui->lstLayers->currentRow());
}

void LayerPanel::on_lstLayers_currentCellChanged(int currentRow, int currentColumn, int prevRow, int prevColumn)
{
    if (block_index_updates)
        return;
    if (!current_project)
        return;
    if (currentRow < 0 || currentRow == prevRow)
        return;
    if (currentRow != prevRow && currentColumn == 0)
    {
        //Stop it from deallocating controls and crashing
        block_index_updates= true;
        ui->lstLayers->setCurrentCell(prevRow, 0);
        block_index_updates= false;
        return;
    }

    current_project->SetCurrentLayerIndex(CurrentLayerIndex());
    //Update();
}

void LayerPanel::on_btnNew_clicked()
{
    if (!current_project)
        return;

    current_project->CurrentFrame()->InsertLayerAt(CurrentLayerIndex()+1);
    current_project->SetCurrentLayerIndex(CurrentLayerIndex()+1);

    //Update();
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

    //Update();
}

void LayerPanel::on_btnMergeDown_clicked()
{

}

