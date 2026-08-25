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
#include "src/project.h"
#include <QFileDialog>
#include <QMessageBox>
#include <cmath>

extern Project* current_project;

ExportDialog::ExportDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ExportDialog)
{
    if (!current_project)
        return;

    ui->setupUi(this);

    this->setModal(true);

    ui->chkCurrentFrameOnly->setChecked(current_project->FrameCount() == 1);
    on_chkCurrentFrameOnly_stateChanged(current_project->FrameCount() == 1? 2 : 0);
    ui->chkCurrentFrameOnly->setEnabled(current_project->FrameCount() > 1);
    ui->spbColumns->setMaximum(current_project->FrameCount());
    on_spbColumns_valueChanged(ui->spbColumns->value());
}

ExportDialog::~ExportDialog()
{
    delete ui;
}

bool ExportDialog::IsExportingRegular() { return ui->grpBitmapExport->isChecked(); }

bool ExportDialog::IsExportingSource() { return ui->grpGritExport->isChecked(); }

bool ExportDialog::IsSpritesheet() { return ui->grpSpritesheetSettings->isChecked(); }

bool ExportDialog::IsCurrentFrameOnly() { return ui->chkCurrentFrameOnly->isChecked(); }

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
        result += "-ps" + QString::number(current_project->PaltableAIndex());
        result += "-pe" + QString::number(current_project->PaltableBIndex()+1);
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

int ExportDialog::SpritesheetColumns()
{
    if (ui->chkAutoColumns->isChecked())
        return -1;
    return ui->spbColumns->value();
}

void ExportDialog::on_buttonBox_accepted()
{
    is_accepted= true;
}

void ExportDialog::on_buttonBox_rejected()
{
    is_accepted= false;
}

void ExportDialog::on_chkAutoColumns_stateChanged(int state)
{
    ui->spbColumns->setEnabled(!state);
    ui->spbRows->setEnabled(!state);
}

void ExportDialog::on_chkCurrentFrameOnly_stateChanged(int state)
{
    ui->grpSpritesheetSettings->setEnabled(!state);
}

void ExportDialog::on_spbColumns_valueChanged(int val)
{
    ui->spbRows->setValue(std::ceil((float)current_project->FrameCount()/(float)val));
}

