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
};

#endif // TOOL_PANEL_H
