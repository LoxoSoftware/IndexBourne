#include "src/animation_panel.h"
#include "src/ui_animation_panel.h"
#include "project.h"
#include "mainwindow.h"

extern Project* current_project;

FrameList::FrameList(QWidget* parent)
{
    this->setParent(parent);
}

void FrameList::Redraw()
{
    clear();

    for (int ifr=0; ifr<current_project->FrameCount(); ifr++)
    {
        Frame* tframe= current_project->FrameAt(ifr);
        QListWidgetItem* new_item= new QListWidgetItem();

        QIcon el_icon= QIcon(QPixmap::fromImage(tframe->RenderBitmap().scaled(50,50)));

        new_item->setText(QString::number(ifr));
        new_item->setIcon(el_icon);

        this->addItem(new_item);
    }

    setCurrentRow(0);
}

AnimationPanel::AnimationPanel(QWidget *parent, MainWindow* main_window)
    : QDockWidget(parent)
    , ui(new Ui::AnimationPanel)
{
    ui->setupUi(this);

    this->main_window= main_window;

    Update();
}

AnimationPanel::~AnimationPanel()
{
    delete ui;
}

void AnimationPanel::Update()
{
    if (!current_project)
        return;

    ui->lstFrames->Redraw();
}