#include "exportdialog.h"
#include "ui_exportdialog.h"
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

