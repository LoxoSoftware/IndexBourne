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

#ifndef TOOL_PANEL_H
#define TOOL_PANEL_H

#include <QDockWidget>

class MainWindow;
struct Tool;

namespace Ui {
class ToolPanel;
}

class ToolPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit ToolPanel(QWidget *parent, MainWindow* main_window);
    ~ToolPanel();

    Tool GetCurrentTool();
    void SetCurrentToolType(int type); //tooltype_t

private:
    Ui::ToolPanel *ui;

    MainWindow* main_window;
    bool block_tooltype_change_signal= false;

    void on_CurrentToolTypeChanged();
    void on_CanvasInvalidate();
};

#endif // TOOL_PANEL_H
