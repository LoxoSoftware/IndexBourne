#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"

NewProjectDialog::NewProjectDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    this->setModal(true);
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

bool NewProjectDialog::GetAccepted()
{
    exec();

    return is_accepted;
}

QSize NewProjectDialog::CanvasSize()
{
    return QSize(ui->spbWidth->value(), ui->spbHeight->value());
}

void NewProjectDialog::on_buttonBox_accepted()
{
    is_accepted= true;
}

void NewProjectDialog::on_buttonBox_rejected()
{
    is_accepted= false;
}

