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

#ifndef EXPORT_DIALOG_H
#define EXPORT_DIALOG_H

#include <QDialog>

typedef enum
{
    Regfmt_PNG = 1,
    Regfmt_BMP = 2,
    Regfmt_RGBAMode = 0x40,
    Regfmt_0Trans = 0x80,
} export_regular_setings_t;

namespace Ui {
class ExportDialog;
}

class ExportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportDialog(QWidget *parent = nullptr);
    ~ExportDialog();

    QString GetOutputFileName();
    bool IsExportingRegular();
    bool IsExportingSource();
    bool IsSpritesheet();
    bool IsCurrentFrameOnly();
    QList<QString> GritFlags();
    unsigned int RegularExportSettings();
    int SpritesheetColumns(); // >0: columns, <0: auto

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();
    void on_chkAutoColumns_stateChanged(int state);
    void on_chkCurrentFrameOnly_stateChanged(int state);
    void on_spbColumns_valueChanged(int val);

private:
    Ui::ExportDialog *ui;

    bool is_accepted= false;
    QString out_format= "";
};

#endif // EXPORT_DIALOG_H
