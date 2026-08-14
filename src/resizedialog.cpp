/*
    IndexBourne - an image editor for indexed mode
    Copyright (C) 2026  LoxoSoftware

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

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

