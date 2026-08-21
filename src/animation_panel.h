#ifndef ANIMATION_PANEL_H
#define ANIMATION_PANEL_H

#include <QDockWidget>
#include <QListWidget>

class MainWindow;

class FrameList : public QListWidget
{
public:
    FrameList(QWidget* parent);

    void Redraw();
};

namespace Ui {
class AnimationPanel;
}

class AnimationPanel : public QDockWidget
{
    Q_OBJECT

public:
    explicit AnimationPanel(QWidget *parent, MainWindow* main_window);
    ~AnimationPanel();

    void Update();

private:
    Ui::AnimationPanel *ui;
    MainWindow* main_window= nullptr;
};

#endif // ANIMATION_PANEL_H
