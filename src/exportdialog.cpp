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

#include "src/exportdialog.h"
#include "src/ui_exportdialog.h"
#include <QFileDialog>
#include <QMessageBox>

ExportDialog::ExportDialog(QWidget *parent, int palindex_start, int palindex_end)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
{
    ui->setupUi(this);

    this->setModal(true);
    this->palindex_start= palindex_start;
    this->palindex_end= palindex_end;
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

bool ExportDialog::IsExportingRegular() { return ui->grpBitmapExport->isChecked(); }

bool ExportDialog::IsExportingSource() { return ui->grpGritExport->isChecked(); }

QString ExportDialog::GetOutputFileName()
{
    this->exec();

    if (!IsExportingRegular() && !IsExportingSource())
    {
        QMessageBox::information(this, "Cannot export", "Please select at least one export option");
        is_accepted= false;
    }

    if (!is_accepted)
        return "";

    QString filter= IsExportingSource()?
        "ASM source (*.s);;C source (*.c);;Binary (*.bin);;GRF (*.grf)"
        : "Regular image only (*.png *.bmp)";

    QString ofname= QFileDialog::getSaveFileName(this, "Export image as file...", QString(),
                    filter);

    if (ofname.isEmpty())
        is_accepted= false;

    return ofname;
}

QList<QString> ExportDialog::GritFlags()
{
    QList<QString> result;
    int gfxbpp= ui->cmbBitDepth->currentText().left(ui->cmbBitDepth->currentText().indexOf(' ')).toInt();
    QString datatype= ui->cmbDataType->currentText();

    result += "-g";
    result += "-g" + datatype;
    result += "-gB" + QString::number(gfxbpp);

    if (ui->cmbImageFormat->currentIndex() == 0) //Tile
        result += "-gt";
    else
        result += "-gb";

    if (ui->cmbMapFormat->currentIndex())
        result += "-m", result += "-m" + datatype;
    switch(ui->cmbMapFormat->currentIndex())
    {
    case 0: //No map
        result += "-m!";
        break;
    case 1: //Normal reduced
        result += "-mLf", result += QString( gfxbpp == 8 ? "-mR8" : "-mR4" );
        break;
    case 2: //Normal full
        result += "-mLf", result += "-mR!";
        break;
    case 3: //Affine reduced
        result += "-mLa", result += "-mRa";
        break;
    case 4: //Affine full
        result += "-mLa", result += "-mR!";
        break;
    default:
        break;
    }

    if (ui->cmbPalette->currentIndex())
        result += "-p", result += "-p" + datatype;
    switch (ui->cmbPalette->currentIndex())
    {
    case 0: //None
        result += "-p!";
        break;
    case 1: //Selected area
        result += "-ps" + QString::number(this->palindex_start);
        result += "-pe" + QString::number(this->palindex_end);
        break;
    case 2: //Full
        result += "-pn256";
        break;
    default:
        break;
    }

    if (ui->chkCreateHeader->isChecked())
        result += "-fh";
    else
        result += "-fh!";

    return result;
}

unsigned int ExportDialog::RegularExportSettings()
{
    unsigned int result= 0;

    switch (ui->cmbRegularFormat->currentIndex())
    {
    case 0: //Indexed PNG
        result |= Regfmt_PNG;
        break;
    case 1: //Indexed BMP
        result |= Regfmt_BMP;
        break;
    case 2: //RGBA PNG
        result |= Regfmt_RGBAMode | Regfmt_PNG;
        break;
    case 3: //RGBA BMP
        result |= Regfmt_RGBAMode | Regfmt_BMP;
        break;
    default:
        break;
    }
    result |= ui->chk0ColorTrans->isChecked()? Regfmt_0Trans : 0;

    return result;
}

void ExportDialog::on_buttonBox_accepted()
{
    is_accepted= true;
}

void ExportDialog::on_buttonBox_rejected()
{
    is_accepted= false;
}

