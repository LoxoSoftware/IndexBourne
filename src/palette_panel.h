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

#ifndef PALETTE_PANEL_H
#define PALETTE_PANEL_H

#include <QDockWidget>
#include <QTableWidget>

class MainWindow;
class PalettePanel;

class ColorTable : public QTableWidget
{
public:
    ColorTable(QWidget* parent) { this->setParent(parent); }

    void SetDragStartIndex(int index) { drag_src_index= index; }

private:
    PalettePanel* parent_dock= nullptr;
    int drag_src_index= 0;

    void mouseReleaseEvent(QMouseEvent* event);
    void dropEvent(QDropEvent* event);
};

namespace Ui {
class PalettePanel;
}

class PalettePanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit PalettePanel(QWidget *parent, MainWindow* main_window);
    ~PalettePanel();

    void Update();
    void UpdateColorStatus(bool force=true);
    void on_UpdateAPosition(QPoint pos);
    void on_UpdateBPosition(QPoint pos);

    bool isBlockingPalUpdates() { return block_pal_updates; }

public slots:
    void on_colorChange();

private slots:
    void on_tblPalette_cellPressed(int row, int column);

private:
    Ui::PalettePanel *ui;
    ColorTable tblPalette= nullptr;
    bool block_pal_updates= false;
    MainWindow* main_window;

#if QT_VERSION_MAJOR > 5
    void enterEvent(QEnterEvent* event);
#else
    void enterEvent(QEvent* event);
#endif
    void leaveEvent(QEvent* event);
};

#endif // PALETTE_PANEL_H
