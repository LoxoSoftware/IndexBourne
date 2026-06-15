#include "tool_panel.h"
#include "ui_tool_panel.h"
#include "mainwindow.h"
#include "project.h"

ToolPanel::ToolPanel(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::ToolPanel)
{
    ui->setupUi(this);

    this->main_window= main_window;

    if (IS_DARK_THEME)
    {
        ui->btnPencil->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/pencil", "#C0C0C0"));
        ui->btnEyedropper->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/eyedropper", "#C0C0C0"));
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
    else if (ui->btnEyedropper->isChecked())
        new_tool.type= Tool_Eyedropper;
    new_tool.diameter_a= ui->spbPencilDotSizeA->value();
    new_tool.diameter_b= ui->spbPencilDotSizeB->value();

    return new_tool;
}