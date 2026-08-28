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

#ifndef PROJECTSETTINGS_DIALOG_H
#define PROJECTSETTINGS_DIALOG_H

#include <QDialog>

namespace Ui {
class ProjectSettingsDialog;
}

class ProjectSettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProjectSettingsDialog(QWidget *parent = nullptr,
                                   QSize default_size= QSize(64, 64), int default_fps= 5);
    ~ProjectSettingsDialog();

    bool GetAccepted();
    QSize NewSize();
    int AnimationFPS();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::ProjectSettingsDialog *ui;

    bool is_accepted= false;
};

#endif // PROJECTSETTINGS_DIALOG_H
