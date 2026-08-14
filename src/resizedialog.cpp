#include "src/resizedialog.h"
#include "src/ui_resizedialog.h"

ResizeDialog::ResizeDialog(QWidget *parent, QSize default_size)
    : QDialog(parent)
    , ui(new Ui::ResizeDialog)
{
    ui->setupUi(this);

    this->setModal(true);

    ui->spbWidth->setValue(default_size.width());
    ui->spbHeight->setValue(default_size.height());
}

ResizeDialog::~ResizeDialog()
{
    delete ui;
}

bool ResizeDialog::GetAccepted()
{
    exec();
    return is_accepted;
}

QSize ResizeDialog::NewSize()
{
    return QSize(ui->spbWidth->value(), ui->spbHeight->value());
}

void ResizeDialog::on_buttonBox_accepted()
{
    is_accepted= true;
}

void ResizeDialog::on_buttonBox_rejected()
{
    is_accepted= false;
}

