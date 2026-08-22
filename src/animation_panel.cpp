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

        new_item->setText(QString::number(ifr+1));
        new_item->setIcon(el_icon);

        this->addItem(new_item);
    }

    setCurrentRow(current_project->CurrentFrameIndex());
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

    block_index_updates= true;
    ui->lstFrames->Redraw();
    block_index_updates= false;
}

void AnimationPanel::on_lstFrames_currentRowChanged(int currentRow)
{
    if (!current_project || block_index_updates)
        return;

    current_project->SetCurrentFrameIndex(ui->lstFrames->currentRow());
    Update();
}

void AnimationPanel::on_btnPrev_clicked()
{
    if (!current_project)
        return;

    if (current_project->CurrentFrameIndex() > 0)
        current_project->SetCurrentFrameIndex(current_project->CurrentFrameIndex()-1);
    else
        current_project->SetCurrentFrameIndex(current_project->FrameCount()-1);

    Update();
}

void AnimationPanel::on_btnNext_clicked()
{
    if (!current_project)
        return;

    if (current_project->CurrentFrameIndex() < current_project->FrameCount()-1)
        current_project->SetCurrentFrameIndex(current_project->CurrentFrameIndex()+1);
    else
        current_project->SetCurrentFrameIndex(0);

    Update();
}

void AnimationPanel::on_btnAddFrame_clicked()
{
    if (!current_project)
        return;

    current_project->InsertFrameAt(ui->lstFrames->currentRow()+1);
    current_project->SetCurrentFrameIndex(ui->lstFrames->currentRow()+1);
    Update();
}

void AnimationPanel::on_btnRemoveFrame_clicked()
{
    if (!current_project)
        return;

    current_project->RemoveFrame(ui->lstFrames->currentRow());
    Update();
}

void AnimationPanel::on_btnDuplicateFrame_clicked()
{
    if (!current_project)
        return;

    current_project->CloneFrame(ui->lstFrames->currentRow());
    current_project->SetCurrentFrameIndex(ui->lstFrames->currentRow()+1);
    Update();
}

