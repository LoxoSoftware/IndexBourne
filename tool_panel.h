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

private:
    Ui::ToolPanel *ui;

    MainWindow* main_window;
};

#endif // TOOL_PANEL_H
