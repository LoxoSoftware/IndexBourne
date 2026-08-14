#include "src/tool_panel.h"
#include "src/ui_tool_panel.h"
#include "src/mainwindow.h"
#include "src/project.h"

#define COL_LIGHT_ICON  "#C0C0C0"

extern Project* current_project;

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
        ui->btnTransformGfx->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/move", COL_LIGHT_ICON));
        ui->btnFloodSelect->setIcon(MainWindow::ColorizeIcon(":/icons/scalable/magic-wand", COL_LIGHT_ICON));
    }

    connect(ui->btnPencil, &QToolButton::clicked, this, &ToolPanel::on_CurrentToolTypeChanged);
    connect(ui->btnEyedropper, &QToolButton::clicked, this, &ToolPanel::on_CurrentToolTypeChanged);
    connect(ui->btnPencilSelect, &QToolButton::clicked, this, &ToolPanel::on_CurrentToolTypeChanged);
    connect(ui->btnRectangleSelect, &QToolButton::clicked, this, &ToolPanel::on_CurrentToolTypeChanged);
    connect(ui->btnTransformGfx, &QToolButton::clicked, this, &ToolPanel::on_CurrentToolTypeChanged);
    connect(ui->btnFloodSelect, &QToolButton::clicked, this, &ToolPanel::on_CurrentToolTypeChanged);
    connect(ui->chkApplyOpaque, &QCheckBox::checkStateChanged, this, &ToolPanel::on_CanvasInvalidate);
    connect(ui->chkForceIntegerScale, &QCheckBox::checkStateChanged, this, &ToolPanel::on_CanvasInvalidate);
}

ToolPanel::~ToolPanel()
{
    delete ui;
}

Tool ToolPanel::GetCurrentTool()
{
    Tool new_tool;

    if (ui->btnPencil->isChecked())
    {
        new_tool.type= Tool_Pencil;
        ui->grpToolSettings->setCurrentWidget(ui->grpToolSettings_pencil);
    }
    if (ui->btnEyedropper->isChecked())
    {
        new_tool.type= Tool_Eyedropper;
        ui->grpToolSettings->setCurrentWidget(ui->grpToolSettings_pencil);
    }
    if (ui->btnPencilSelect->isChecked())
    {
        new_tool.type= Tool_PencilSelect;
        ui->grpToolSettings->setCurrentWidget(ui->grpToolSettings_pencil);
    }
    if (ui->btnRectangleSelect->isChecked())
    {
        new_tool.type= Tool_RectSelect;
        ui->grpToolSettings->setCurrentWidget(ui->grpToolSettings_pencil);
    }
    if (ui->btnTransformGfx->isChecked())
    {
        new_tool.type= Tool_Transform;
        ui->grpToolSettings->setCurrentWidget(ui->grpToolSettings_transform);
    }
    if (ui->btnFloodSelect->isChecked())
    {
        new_tool.type= Tool_FloodSelect;
        ui->grpToolSettings->setCurrentWidget(ui->grpToolSettings_pencil);
    }

    new_tool.diameter_a= ui->spbPencilDotSizeA->value();
    new_tool.diameter_b= ui->spbPencilDotSizeB->value();
    new_tool.opaque_apply_mode= ui->chkApplyOpaque->isChecked();
    new_tool.force_integer_scale= ui->chkForceIntegerScale->isChecked();

    return new_tool;
}

void ToolPanel::SetCurrentToolType(int type)
{
    block_tooltype_change_signal= true;

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
        ui->btnTransformGfx->setChecked(true);
        break;
    case Tool_FloodSelect:
        ui->btnFloodSelect->setChecked(true);
        break;
    default:
        break;
    }

    block_tooltype_change_signal= false;
    on_CurrentToolTypeChanged();
}

void ToolPanel::on_CurrentToolTypeChanged()
{
    if (block_tooltype_change_signal)
        return;

    //GetCurrentTool();

    current_project->Canvas()->UpdateMode();
}

void ToolPanel::on_CanvasInvalidate()
{
    current_project->Canvas()->UpdateCurrentTool();
    current_project->Canvas()->Redraw();
}