#include "layer_panel.h"
#include "ui_layer_panel.h"
#include <QMessageBox>
#include "project.h"
#include <QLabel>

extern Project* current_project;

void LayerTable::mousePressEvent(QMouseEvent* event)
{
    mouse_moving= false;
}

void LayerTable::mouseReleaseEvent(QMouseEvent* event)
{
    QTableWidgetItem* item= itemAt(event->pos());
    if (!item)
        return;

    if (!mouse_moving)
    {
        //current_project->SetCurrentLayerIndex(LayerIndex(item->row()));
        on_currentCellChanged(item->row(), 1, currentRow(), currentColumn());
    }

    mouse_moving= false;
}

void LayerTable::mouseMoveEvent(QMouseEvent* event)
{
    // if (event->button() != Qt::LeftButton)
    //     return;

    QTableWidgetItem* item= itemAt(event->pos());
    if (!item)
        return;

    if (!mouse_moving)
    {
        drag_src_index= item->row();
        on_currentCellChanged(item->row(), 1, currentRow(), currentColumn());
        startDrag(Qt::MoveAction);
        mouse_moving= true;
    }
}

void LayerTable::dropEvent(QDropEvent* event)
{
    mouse_moving= false;
    if (drag_src_index < 0)
        return;
    QTableWidgetItem* item= itemAt(event->position().toPoint());
    if (!item)
        return;

    current_project->SwapLayerIndex(LayerIndex(item->row()), LayerIndex(drag_src_index));

    on_currentCellChanged(item->row(), 1, currentRow(), currentColumn());
}

void LayerTable::Redraw()
{
    //Deallocate all widgets
    for (int iy=0; iy<rowCount(); iy++)
        for (int ix=0; ix<columnCount(); ix++)
            if (cellWidget(iy, ix))
                delete cellWidget(iy, ix);

    clear();

    Frame* frame= current_project->CurrentFrame();

    setRowCount(frame->LayerCount());

    for (int il=0; il<frame->LayerCount(); il++)
    {
        int irow= frame->LayerCount()-il-1;
        //QListWidgetItem* item= new QListWidgetItem("Layer "+QString::number(il), ui->lstLayers);
        QPushButton* btn_visible= new QPushButton("V");
        btn_visible->setCheckable(true);
        btn_visible->setChecked(frame->LayerAt(il)->visible);
        btn_visible->setToolTip("Visible?");
        connect(btn_visible, &QPushButton::clicked, this, &LayerTable::on_layerVisibilityChanged);
        QLabel* lbl_name= new QLabel("layer "+QString::number(il));
        QIcon icn_preview= QIcon(QPixmap::fromImage(frame->LayerAt(il)->image.copy().transformed(QTransform::fromScale(48,48))));
        //item->setIcon(QIcon(QPixmap::fromImage(frame->Layer(il)->copy())));
        setCellWidget(irow, 0, btn_visible);
        setCellWidget(irow, 2, lbl_name);
        setItem(irow, 0, new QTableWidgetItem());
        setItem(irow, 1, new QTableWidgetItem(icn_preview, ""));
        setItem(irow, 2, new QTableWidgetItem());
    }

    setCurrentCell(LayerIndex(current_project->CurrentLayerIndex()), 0);
    //setSelection(QRect(currentColumn(), currentRow(), 1, 1), QItemSelectionModel::ClearAndSelect);
}

void LayerTable::on_currentCellChanged(int currentRow, int currentColumn, int prevRow, int prevColumn)
{
    if (!current_project)
        return;
    if (currentRow < 0 || currentRow == prevRow)
        return;
    if (currentRow != prevRow && currentColumn == 0)
    {
        //Stop it from deallocating controls and crashing
        setCurrentCell(prevRow, 0);
        Redraw();
        return;
    }

    current_project->SetCurrentLayerIndex(LayerIndex(currentRow));
    //Update();
}

void LayerTable::on_layerVisibilityChanged(bool visible)
{
    if (!current_project)
        return;

    int layer_index= CurrentLayerIndex();

    current_project->CurrentFrame()->LayerAt(layer_index)->visible= visible;
    Redraw();
    if (current_project->Canvas())
        current_project->Canvas()->Redraw();
}

int LayerTable::LayerIndex(int row)
{
    if (row < 0)
        return - 1;

    return rowCount() - row - 1;
}

int LayerTable::CurrentLayerIndex()
{
    return LayerIndex(currentRow());
}

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
    if (!current_project)
        return;

    ui->lstLayers->Redraw();
}

void LayerPanel::on_btnNew_clicked()
{
    if (!current_project)
        return;

    current_project->CurrentFrame()->InsertLayerAt(ui->lstLayers->CurrentLayerIndex()+1);
    current_project->SetCurrentLayerIndex(ui->lstLayers->CurrentLayerIndex()+1);

    //Update();
}

void LayerPanel::on_btnDelete_clicked()
{
    if (!current_project)
        return;

    current_project->CurrentFrame()->RemoveLayer(ui->lstLayers->CurrentLayerIndex());
    if (ui->lstLayers->CurrentLayerIndex()-1 > 0)
        current_project->SetCurrentLayerIndex(ui->lstLayers->CurrentLayerIndex()-1);
    else
        current_project->SetCurrentLayerIndex(0);

    //Update();
}

void LayerPanel::on_btnMergeDown_clicked()
{
    if (!current_project)
        return;

    current_project->CurrentFrame()->MergeLayerDown(ui->lstLayers->CurrentLayerIndex());
    if (ui->lstLayers->CurrentLayerIndex()-1 > 0)
        current_project->SetCurrentLayerIndex(ui->lstLayers->CurrentLayerIndex()-1);
    else
        current_project->SetCurrentLayerIndex(0);
}
