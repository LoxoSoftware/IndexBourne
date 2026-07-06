#include "tool_panel.h"
#include "ui_tool_panel.h"
#include "mainwindow.h"
#include "project.h"

#define COL_LIGHT_ICON  "#C0C0C0"

ToolPanel::ToolPanel(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::ToolPanel)
{
    ui->setupUi(this);

    this->main_window= main_window;

    if (IS_DARK_THEME)
    {
        ui->btnPencil->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/pencil", COL_LIGHT_ICON));
        ui->btnEyedropper->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/eyedropper", COL_LIGHT_ICON));
        ui->btnPencilSelect->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/pencil-select", COL_LIGHT_ICON));
        ui->btnRectangleSelect->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/rectangle-select", COL_LIGHT_ICON));
        ui->btnMoveGfx->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/move", COL_LIGHT_ICON));
    }
}

ToolPanel::~ToolPanel()
{
    delete ui;
}

Tool ToolPanel::GetCurrentTool()
{
    Tool new_tool;

    if (ui->btnPencil->isChecked())
        new_tool.type= Tool_Pencil;
    if (ui->btnEyedropper->isChecked())
        new_tool.type= Tool_Eyedropper;
    if (ui->btnPencilSelect->isChecked())
        new_tool.type= Tool_PencilSelect;
    if (ui->btnRectangleSelect->isChecked())
        new_tool.type= Tool_RectSelect;
    if (ui->btnMoveGfx->isChecked())
        new_tool.type= Tool_Transform;

    new_tool.diameter_a= ui->spbPencilDotSizeA->value();
    new_tool.diameter_b= ui->spbPencilDotSizeB->value();

    return new_tool;
}

void ToolPanel::SetCurrentToolType(int type)
{
    switch(type)
    {
    case Tool_Pencil:
        ui->btnPencil->setChecked(true);
        break;
    case Tool_Eyedropper:
        ui->btnEyedropper->setChecked(true);
        break;
    case Tool_PencilSelect:
        ui->btnPencilSelect->setChecked(true);
        break;
    case Tool_RectSelect:
        ui->btnRectangleSelect->setChecked(true);
        break;
    case Tool_Transform:
        ui->btnMoveGfx->setChecked(true);
        break;
    default:
        break;
    }
}