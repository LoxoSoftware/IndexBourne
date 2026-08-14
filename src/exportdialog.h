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

#ifndef EXPORTDIALOG_H
#define EXPORTDIALOG_H

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
    explicit ExportDialog(QWidget *parent = nullptr, int palindex_start= 0, int palindex_end= 256);
    ~ExportDialog();

    QString GetOutputFileName();
    bool IsExportingRegular();
    bool IsExportingSource();
    QList<QString> GritFlags();
    unsigned int RegularExportSettings();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::ExportDialog *ui;

    bool is_accepted= false;
    QString out_format= "";
    int palindex_start, palindex_end;
};

#endif // EXPORTDIALOG_H
