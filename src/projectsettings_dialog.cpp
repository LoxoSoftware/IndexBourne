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

#include "src/projectsettings_dialog.h"
#include "src/ui_projectsettings_dialog.h"

ProjectSettingsDialog::ProjectSettingsDialog(QWidget *parent,
                                             QSize default_size, int default_fps)
    : QDialog(parent)
    , ui(new Ui::ProjectSettingsDialog)
{
    ui->setupUi(this);

    this->setModal(true);

    ui->spbWidth->setValue(default_size.width());
    ui->spbHeight->setValue(default_size.height());
    ui->spbFramerate->setValue(default_fps);
}

ProjectSettingsDialog::~ProjectSettingsDialog()
{
    delete ui;
}

bool ProjectSettingsDialog::GetAccepted()
{
    exec();
    return is_accepted;
}

QSize ProjectSettingsDialog::NewSize()
{
    return QSize(ui->spbWidth->value(), ui->spbHeight->value());
}

int ProjectSettingsDialog::AnimationFPS()
{
    return ui->spbFramerate->value();
}

void ProjectSettingsDialog::on_buttonBox_accepted()
{
    is_accepted= true;
}

void ProjectSettingsDialog::on_buttonBox_rejected()
{
    is_accepted= false;
}

