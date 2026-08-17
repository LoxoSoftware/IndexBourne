/*
    IndexBourne - an image editor for indexed mode
    Copyright (C) 2026  LoxoSoftware

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "src/layer_panel.h"
#include "src/ui_layer_panel.h"
#include <QMessageBox>
#include "project.h"
#include "mainwindow.h"
#include <QLabel>

extern Project* current_project;

LayerTable::LayerTable(QWidget* parent)
{
    this->setParent(parent);
}

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
#if QT_VERSION_MAJOR > 5
    QTableWidgetItem* item= itemAt(event->position().toPoint());
#else
    QTableWidgetItem* item= itemAt(event->pos());
#endif
    if (!item)
        return;

    current_project->SwapAllLayers(LayerIndex(item->row()), LayerIndex(drag_src_index));

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
        QPushButton* btn_visible= new QPushButton();
        QLabel* lbl_preview= new QLabel();
        QLabel* lbl_name= new QLabel("layer "+QString::number(il));

        btn_visible->setCheckable(true);
        btn_visible->setChecked(current_project->LayerInfo(il)->visible);
        btn_visible->setToolTip("Toggle layer visibility");
        btn_visible->setIconSize(QSize(32,32));
        if (IS_DARK_THEME)
            btn_visible->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/layer-invisible", "#C0C0C0", ":/icons/scalable/layer-visible"));
        else
            btn_visible->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/layer-invisible", "#202020", ":/icons/scalable/layer-visible"));
        connect(btn_visible, &QPushButton::clicked, this, &LayerTable::on_layerVisibilityChanged);

        lbl_preview->setPixmap(QPixmap::fromImage(frame->LayerAt(il)->copy().scaled(QSize(48,48), Qt::KeepAspectRatio)));

        btn_visible->setStyleSheet("* { margin:8px; } ");
        lbl_preview->setStyleSheet("* { margin:0px; } ");
        lbl_name->setStyleSheet("* { margin:4px; } ");
        setCellWidget(irow, 0, btn_visible);
        setCellWidget(irow, 1, lbl_preview);
        setCellWidget(irow, 2, lbl_name);
        setItem(irow, 0, new QTableWidgetItem());
        setItem(irow, 1, new QTableWidgetItem());
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

    current_project->LayerInfo(layer_index)->visible= visible;
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

    current_project->InsertLayerAt(ui->lstLayers->CurrentLayerIndex()+1);
    current_project->SetCurrentLayerIndex(ui->lstLayers->CurrentLayerIndex()+1);

    //Update();
}

void LayerPanel::on_btnDelete_clicked()
{
    if (!current_project)
        return;

    current_project->RemoveLayer(ui->lstLayers->CurrentLayerIndex());
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
