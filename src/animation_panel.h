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

private slots:
    void on_btnPrev_clicked();
    void on_btnNext_clicked();
    void on_btnAddFrame_clicked();
    void on_lstFrames_currentRowChanged(int currentRow);
    void on_btnRemoveFrame_clicked();
    void on_btnDuplicateFrame_clicked();
    void on_btnPlayToggle_toggled(bool checked);

private:
    Ui::AnimationPanel *ui;
    MainWindow* main_window= nullptr;
    bool block_index_updates= false;
    int playback_timer_id= -1;

    void timerEvent(QTimerEvent* event);
};

#endif // ANIMATION_PANEL_H
