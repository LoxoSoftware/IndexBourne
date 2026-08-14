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

#include "src/palette_panel.h"
#include "src/ui_palette_panel.h"
#include "src/project.h"
#include "src/mainwindow.h"
#include <QMessageBox>

extern Project* current_project;

void ColorTable::mouseReleaseEvent(QMouseEvent* event)
{
    QModelIndex clicked= indexAt(event->pos());

    switch (event->button())
    {
    case Qt::RightButton:
        current_project->SetPaltableBPosition(QPoint(clicked.column(), clicked.row()));
        break;
    case Qt::LeftButton:
        current_project->SetPaltableAPosition(QPoint(clicked.column(), clicked.row()));
        break;
    default:
        return;
    }
}

void ColorTable::dropEvent(QDropEvent* event)
{
#if QT_VERSION_MAJOR > 5
    int drop_dest_index= indexAt(event->position().toPoint()).column()+
                         indexAt(event->position().toPoint()).row()*PALETTE_W;
#else
    int drop_dest_index= indexAt(event->pos()).column()+
                         indexAt(event->pos()).row()*PALETTE_W;
#endif
    //QMessageBox::information(this, "debug", QString::number(drag_src_index)+" <---> "+QString::number(drop_dest_index));

    current_project->SwapColorIndex(drag_src_index, drop_dest_index);
}

PalettePanel::PalettePanel(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::PalettePanel)
{
    ui->setupUi(this);
    this->main_window= main_window;

    ui->tblPalette->setCurrentCell(0,0);

    connect(ui->sliRedChannel, &QSlider::valueChanged, this, &PalettePanel::on_colorChange);
    connect(ui->sliBlueChannel, &QSlider::valueChanged, this, &PalettePanel::on_colorChange);
    connect(ui->sliGreenChannel, &QSlider::valueChanged, this, &PalettePanel::on_colorChange);
    connect(ui->spbRedChannel, QOverload<int>::of(&QSpinBox::valueChanged), this, &PalettePanel::on_colorChange);
    connect(ui->spbBlueChannel, QOverload<int>::of(&QSpinBox::valueChanged), this, &PalettePanel::on_colorChange);
    connect(ui->spbGreenChannel, QOverload<int>::of(&QSpinBox::valueChanged), this, &PalettePanel::on_colorChange);
}

PalettePanel::~PalettePanel()
{
    delete ui;
}

void PalettePanel::Update()
{
    ui->tblPalette->clear();
    for (int iy=0; iy<PALETTE_H; iy++)
    {
        for (int ix=0; ix<PALETTE_W; ix++)
        {
            QTableWidgetItem* item= new QTableWidgetItem();
            QBrush bru_bg;
            QBrush bru_fg;

            if (ix+iy*PALETTE_W < current_project->Palette().count())
                bru_bg.setColor(current_project->Palette()[ix+iy*PALETTE_W]);
            else
            {
                //Color index is outside of palette bounds (shouldn't normally happen)
                int lumarand= rand()%32;
                bru_bg.setColor(QColor::fromRgb(lumarand*8, lumarand*8, lumarand*8));
            }

            if (bru_bg.color().black() > 64)
            {
                bru_fg.setColor(QColor::fromRgb(255,255,255));
            }
            else
            {
                bru_fg.setColor(QColor::fromRgb(0,0,0));
            }

            bru_bg.setStyle(Qt::SolidPattern);
            bru_fg.setStyle(Qt::SolidPattern);

            if (current_project->BppFormat() == Format_4bpp)
            {
                if (current_project->PaltableAPosition().y() == iy)
                    bru_bg.setStyle(Qt::Dense1Pattern);
                else
                    bru_bg.setStyle(Qt::SolidPattern);
            }

            if (current_project->PaltableAPosition().y() == iy && current_project->PaltableAPosition().x() == ix)
                item->setText("A");
            else if (current_project->PaltableBPosition().y() == iy && current_project->PaltableBPosition().x() == ix)
                item->setText("B");

            item->setBackground(bru_bg);
            item->setForeground(bru_fg);

            if (current_project->BppFormat() == Format_4bpp)
                item->setToolTip("Pal #"+QString::number(iy)+": "+QString::number(ix));
            else
                item->setToolTip("Index: "+QString::number(ix+iy*PALETTE_W));
            ui->tblPalette->setItem(iy, ix, item);
        }
    }

    switch (current_project->BppFormat())
    {
    case Format_4bpp:
        setWindowTitle("Palettes (4bpp)");
        break;
    case Format_8bpp:
        setWindowTitle("Palette (8bpp)");
        break;
    default:
        setWindowTitle("Palette (invalid tile format!)");
        break;
    }

    if (current_project->SharedPalette() != "")
    {
        ui->lblPaletteName->setText("Shared file: " +
            current_project->SharedPalette().mid(current_project->SharedPalette().lastIndexOf('/')+1));
        ui->lblPaletteName->setToolTip(current_project->SharedPalette());
    }
    else
    {
        ui->lblPaletteName->setText("Internal data");
        ui->lblPaletteName->setToolTip("Not using a shared palette");
    }
}

void PalettePanel::on_UpdateAPosition(QPoint pos)
{
    int palind= pos.x()+pos.y()*PALETTE_W;
    if (palind >= current_project->Palette().size())
        return;
    block_pal_updates= true;
    ui->sliRedChannel->setValue(qRed(current_project->Palette()[palind])/8);
    ui->sliGreenChannel->setValue(qGreen(current_project->Palette()[palind])/8);
    ui->sliBlueChannel->setValue(qBlue(current_project->Palette()[palind])/8);
    UpdateColorStatus(true);
    Update();
    block_pal_updates= false;
}

void PalettePanel::on_UpdateBPosition(QPoint pos)
{
    int palind= pos.x()+pos.y()*PALETTE_W;
    if (palind >= current_project->Palette().size())
        return;
    block_pal_updates= true;
    Update();
    block_pal_updates= false;
}

void PalettePanel::UpdateColorStatus(bool force)
{
    if (ui->sliRedChannel->hasFocus() || force)
        ui->spbRedChannel->setValue(ui->sliRedChannel->value());
    if (ui->sliGreenChannel->hasFocus() || force)
        ui->spbGreenChannel->setValue(ui->sliGreenChannel->value());
    if (ui->sliBlueChannel->hasFocus() || force)
        ui->spbBlueChannel->setValue(ui->sliBlueChannel->value());

    if (ui->spbRedChannel->hasFocus() && !force)
        ui->sliRedChannel->setValue(ui->spbRedChannel->value());
    if (ui->spbGreenChannel->hasFocus() && !force)
        ui->sliGreenChannel->setValue(ui->spbGreenChannel->value());
    if (ui->spbBlueChannel->hasFocus() && !force)
        ui->sliBlueChannel->setValue(ui->spbBlueChannel->value());

    ui->widPrimaryColor->setStyleSheet("border: 1px solid black; background-color: rgb("
                                       + QString::number(ui->spbRedChannel->value()*8) + ","
                                       + QString::number(ui->spbGreenChannel->value()*8) + ","
                                       + QString::number(ui->spbBlueChannel->value()*8) + ");");

    int paltable_index= current_project->PaltableAPosition().x()+current_project->PaltableAPosition().y()*PALETTE_W;
    QRgb new_col= QColor(ui->spbRedChannel->value()*8,
                          ui->spbGreenChannel->value()*8, ui->spbBlueChannel->value()*8).rgb();

    if (paltable_index < current_project->Palette().count() && !force)
    {
        palette_t new_palette= current_project->Palette();
        new_palette[paltable_index]= new_col;
        current_project->SetPalette(new_palette);
        if (block_pal_updates) return;
        Update();
    }
}

#if QT_VERSION_MAJOR > 5
void PalettePanel::enterEvent(QEnterEvent* event)
#else
void PalettePanel::enterEvent(QEvent* event)
#endif
{
    // if (!current_project->statusbar)
    //     return;
    // current_project->statusbar->showMessage("Click a color cell in the table to select it. In 4bpp tile mode, the current row will be used as the draw palette");
}

void PalettePanel::leaveEvent(QEvent* event)
{
    // if (!current_project->statusbar)
    //     return;
    // current_project->statusbar->clearMessage();
}

void PalettePanel::on_colorChange()
{
    if (block_pal_updates)
        return;
    UpdateColorStatus(false);
    //current_project->Canvas()->Redraw();
}

void PalettePanel::on_tblPalette_cellPressed(int row, int column)
{
    ui->tblPalette->SetDragStartIndex(column+row*PALETTE_W);
}

