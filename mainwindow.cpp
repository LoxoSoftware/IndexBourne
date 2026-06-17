#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "project.h"
#include <QMessageBox>
#include <QFileDialog>
#include "newprojectdialog.h"

QList<Project> open_projects;
Project* current_project= nullptr;

void NewProject(MainWindow* parent, QSize size = QSize(64,64))
{
    open_projects.clear();
    current_project= nullptr;
    open_projects += Project(parent, size);
    current_project= &open_projects.last();
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dckPaletteEdit= new PalettePanel(this->CentralWidget(), this);
    addDockWidget(Qt::RightDockWidgetArea, dckPaletteEdit);
    dckToolPanel= new ToolPanel(this->CentralWidget(), this);
    addDockWidget(Qt::LeftDockWidgetArea, dckToolPanel);

    NewProject(this);
    dckPaletteEdit->Update();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QIcon MainWindow::ColorizeIcon(QString fname, QString color)
{
    QFile src_file= QFile(fname);
    if (!src_file.open(QFile::ReadOnly))
        return QIcon(fname); //Error
    QString data= src_file.readAll();
    //data= data.replace("\"#000000\"", "\""+color+"\"");
    data= data.replace("#000000", color);
    src_file.close();
    QIcon new_icon= QIcon(QPixmap::fromImage(QImage::fromData(data.toLocal8Bit())));
    return new_icon;
}

void MainWindow::on_actionZoom_in_triggered()
{
    if (!current_project)
        return;

    current_project->Canvas()->ZoomIn();
}

void MainWindow::on_actionZoom_out_triggered()
{
    if (!current_project)
        return;

    current_project->Canvas()->ZoomOut();
}

void MainWindow::on_actionQuit_triggered()
{
    this->close();
}

void MainWindow::on_actionNewProject_triggered()
{
    NewProjectDialog dial= NewProjectDialog(this);

    if (dial.GetAccepted())
    {
        NewProject(this, dial.CanvasSize());
    }
}

void MainWindow::on_actionOpenProject_triggered()
{
    if (!current_project)
        NewProject(this);

    QString ifile_name= QFileDialog::getOpenFileName(this, "Open project or import bitmap", "", "Supported formats ("/**.gfx */"*.bmp *.png)");

    if (ifile_name == "")
        return;

    QImage imported_image= QImage(ifile_name);

    if (imported_image == QImage())
    {
        //Open project file
        QMessageBox::critical(this, "Error", "File format is unsupported");
        return;
    }
    else
    {
        //Import image in new project
        if (!current_project->ImportBitmap(imported_image, Consent_Force, Consent_Force))
            return; //Error
    }

    current_project->SetFileName(ifile_name);
}

void MainWindow::on_actionSaveProject_triggered()
{
    if (!current_project)
        return;
    if (current_project->FileName() == "")
    {
        on_actionSaveProjectAs_triggered();
        return;
    }

    if (!_QuickSaveBitmap(current_project->FileName()))
        return; //Error
}

void MainWindow::on_actionSaveProjectAs_triggered()
{
    if (!current_project)
        return;

    //QString ofile_name= QFileDialog::getSaveFileName(this, "Save project or bitmap", "", "Supported formats ("/**.gfx */"*.bmp *.png)");
    QString ofile_name= QFileDialog::getSaveFileName(this, "Save project or bitmap", "", "Windows bitmap (*.bmp);;Portable Network Graphics (*.png)");

    if (ofile_name == "")
        return;

    if (ofile_name.endsWith(".bmp", Qt::CaseInsensitive) || ofile_name.endsWith(".png", Qt::CaseInsensitive))
        _QuickSaveBitmap(ofile_name);
    else if (ofile_name.endsWith(".gfx", Qt::CaseSensitive))
    {
        QMessageBox::critical(this, "Not implemented", "Saving projects is not currently implemented");
        return;
    }
    else
    {
        QMessageBox::warning(this, "Undefined format", "Output format was not defined, defaulting to BMP");
        _QuickSaveBitmap(ofile_name+".bmp");
    }
}

bool MainWindow::_QuickSaveBitmap(QString new_fname)
{
    if (!current_project->RenderBitmap().save(new_fname, nullptr, 100))
    {
        QMessageBox::critical(this, "Write error", "Failed to open output file for writing");
        return false;
    }
    else
        current_project->SetFileName(new_fname);

    return true;
}

void MainWindow::on_actionPaletteQuickSwap_triggered()
{
    current_project->SwapColorIndex(current_project->PaltableAIndex(), current_project->PaltableBIndex());
}

void MainWindow::on_actionPaletteQuickFill_triggered()
{
    QRgb fill_color= current_project->Palette().at(current_project->PaltableAIndex());
    //current_project->FillPaletteLinear(current_project->PaltableAIndex(), current_project->PaltableBIndex(), fill_color);
    current_project->FillPaletteRect(current_project->PaltableAPosition(), current_project->PaltableBPosition(), fill_color);
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About EZGFX", "EZGFX is a free and open source image editor aimed at"
                                            "game developement for retro platforms."
                                            "\n\nCopyright (c)2026 LoxoSoftware");
}

void MainWindow::on_actionAboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_actionUndo_triggered()
{
    current_project->CurrentFrame()->Undo();
    current_project->Canvas()->SetFrame(current_project->CurrentFrame());
}

void MainWindow::on_actionRedo_triggered()
{
    current_project->CurrentFrame()->Redo();
    current_project->Canvas()->SetFrame(current_project->CurrentFrame());
}

void MainWindow::on_actionTogglePixelGrid_triggered(bool checked)
{
    current_project->Canvas()->EnablePixelGrid(checked);
}

void MainWindow::on_actionToggleTileGrid_triggered(bool checked)
{
    current_project->Canvas()->EnableTileGrid(checked);
}

