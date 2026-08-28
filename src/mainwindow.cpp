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

#include "src/mainwindow.h"
#include "src/ui_mainwindow.h"
#include "src/project.h"
#include "src/newproject_dialog.h"
#include "src/projectsettings_dialog.h"
#include "config.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QLabel>

Project* current_project= nullptr;

void NewProject(MainWindow* parent, QSize size = QSize(64,64))
{
    current_project= nullptr;
    current_project= new Project(parent, size);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    dckLayerPanel= new LayerPanel(this->CentralWidget(), this);
    addDockWidget(Qt::RightDockWidgetArea, dckLayerPanel);
    dckPaletteEdit= new PalettePanel(this->CentralWidget(), this);
    addDockWidget(Qt::RightDockWidgetArea, dckPaletteEdit);
    dckToolPanel= new ToolPanel(this->CentralWidget(), this);
    addDockWidget(Qt::LeftDockWidgetArea, dckToolPanel);
    dckAnimPanel= new AnimationPanel(this->CentralWidget(), this);
    addDockWidget(Qt::BottomDockWidgetArea, dckAnimPanel);

    NewProject(this);
    dckPaletteEdit->Update();
    dckLayerPanel->Update();
    dckAnimPanel->Update();

    UpdateWindowTitle();
    QLabel* lbl_version= new QLabel(APP_VERSION);
    this->menuBar()->setCornerWidget(lbl_version, Qt::TopRightCorner);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (!current_project)
    {
        event->accept();
        return;
    }
    if (current_project->IsSaved())
    {
        event->accept();
        return;
    }

    QMessageBox::StandardButton answer;
    answer= QMessageBox::question(this, "Unsaved changes",
                                   "There may be unsaved changes in your project, \nwhat would you like to do?",
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

    switch(answer)
    {
    case QMessageBox::Save:
        on_actionSaveProject_triggered();
        break;
    case QMessageBox::Cancel:
        event->ignore();
        return;
    case QMessageBox::Discard:
    default:
        break;
    }

    event->accept();
}

QIcon MainWindow::ColorizeIcon(QString fname, QString color, QString fname_on)
{
    QFile src_file= QFile(fname);
    QFile src_file_on= QFile(fname_on);

    if (!src_file.open(QFile::ReadOnly))
        return QIcon(fname); //Error

    QString data= src_file.readAll();
    //data= data.replace("\"#000000\"", "\""+color+"\"");
    data= data.replace("#000000", color);
    src_file.close();

    QIcon new_icon= QIcon();
    new_icon.addPixmap(QPixmap::fromImage(QImage::fromData(data.toLocal8Bit())));

    if (fname_on != "") if (src_file_on.open(QFile::ReadOnly))
    {
        data= src_file_on.readAll();
        //data= data.replace("\"#000000\"", "\""+color+"\"");
        data= data.replace("#000000", color);
        src_file_on.close();

        new_icon.addPixmap(QPixmap::fromImage(QImage::fromData(data.toLocal8Bit())), QIcon::Normal, QIcon::On);
    }

    return new_icon;
}

void MainWindow::UpdateWindowTitle()
{
    QString project_name= "<new project>";
    bool notsaved= true;

    if (current_project)
    {
        QString fname= current_project->FileName();

        if (fname != "")
            project_name= fname.mid(fname.lastIndexOf('/')+1);

        notsaved= !current_project->IsSaved();
    }

    if (project_name == QString())
        project_name= "<>";

    setWindowTitle(project_name + ( notsaved ? " *" : "" ));
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

    QString ifile_name= QFileDialog::getOpenFileName(this, "Open project or import bitmap", "", "Supported formats (*.ora *.gfx *.bmp *.png)");

    if (ifile_name == "")
        return;

    if (ifile_name.endsWith(".bmp", Qt::CaseInsensitive) || ifile_name.endsWith(".png", Qt::CaseInsensitive))
    {
        if (!current_project->ImportBitmap(QImage(ifile_name), Consent_Force, Consent_Force))
            return;
        current_project->SetFileName(ifile_name);
    }
    else if (ifile_name.endsWith(".gfx", Qt::CaseSensitive) || ifile_name.endsWith(".ora", Qt::CaseSensitive))
    {
        if (current_project->LoadProject(ifile_name))
            current_project->SetFileName(ifile_name);
        else
        {
            QMessageBox::critical(this, "Error", "An error occoured while opening the project");
            return;
        }
    }
    else
    {
        QMessageBox::critical(this, "Error", "Input format is not supported");
        return;
    }
}

void MainWindow::on_actionSaveProject_triggered()
{
    if (!current_project)
        return;

    QString ofile_name= current_project->FileName();

    if (ofile_name == "")
    {
        on_actionSaveProjectAs_triggered();
        return;
    }

    if (!(ofile_name.endsWith(".gfx", Qt::CaseSensitive) || ofile_name.endsWith(".ora", Qt::CaseSensitive)))
    {
        QMessageBox::information(this, "Saving as new project", "Input image will be saved as a new IndexBourne project.\n"
                                 "You can then select Image -> Export to save your image as a regular bitmap");
        ofile_name= ofile_name.left(ofile_name.size()-4);
        ofile_name += ".ora";
    }

    if (!current_project->SaveProject(ofile_name))
    {
        QMessageBox::critical(this, "Error", "An error occoured while saving the project");
        return;
    }
    else
        current_project->SetFileName(ofile_name);
}

void MainWindow::on_actionSaveProjectAs_triggered()
{
    if (!current_project)
        return;

    QString ofile_name= QFileDialog::getSaveFileName(this, "Save project", "", "IndexBourne project (*.ora *.gfx)");

    if (ofile_name == "")
        return;

    if (!(ofile_name.endsWith(".gfx", Qt::CaseSensitive) || ofile_name.endsWith(".ora", Qt::CaseSensitive)))
    {
        QMessageBox::warning(this, "Invalid format", "Output format is invalid, defaulting to ORA");
        ofile_name += ".ora";
    }

    if (current_project->SaveProject(ofile_name))
        current_project->SetFileName(ofile_name);
    else
    {
        QMessageBox::critical(this, "Error", "An error occoured while saving the project");
        return;
    }
}

void MainWindow::on_actionPaletteQuickSwap_triggered()
{
    current_project->SwapColorIndex(current_project->PaltableAIndex(), current_project->PaltableBIndex(), true);
}

void MainWindow::on_actionPaletteQuickSwapDestructive_triggered()
{
    current_project->SwapColorIndex(current_project->PaltableAIndex(), current_project->PaltableBIndex(), false);
}

void MainWindow::on_actionPaletteQuickFill_triggered()
{
    QRgb fill_color= current_project->Palette().at(current_project->PaltableAIndex());
    //current_project->FillPaletteLinear(current_project->PaltableAIndex(), current_project->PaltableBIndex(), fill_color);
    current_project->FillPaletteRect(current_project->PaltableAPosition(), current_project->PaltableBPosition(), fill_color);
}

void MainWindow::on_actionAbout_triggered()
{
    QWidget tw= QWidget(this);
    tw.setWindowIcon(QIcon(":/icons/bitmap/about-banner"));
    QMessageBox::about(&tw, "About IndexBourne", "IndexBourne is a free and open source image editor aimed at "
                                            "game developement for retro platforms."
                                            "\n\nCopyright (C) 2026 LoxoSoftware\nVersion "+QString(APP_VERSION));
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

void MainWindow::on_actionProjectProperties_triggered()
{
    if (!current_project)
        return;
    dckAnimPanel->SetPlaybackStatus(false);

    ProjectSettingsDialog dial= ProjectSettingsDialog(this,
                                                       current_project->ImageSize(), current_project->AnimationFPS());

    if (dial.GetAccepted())
    {
        if (dial.NewSize() != current_project->ImageSize())
        {
            if (QMessageBox::question(this, "Resize canvas", "Do you wish to resize the project canvas?\nThis cannot be undone",
                                      QMessageBox::Yes | QMessageBox::Cancel) == QMessageBox::Yes)
                current_project->SetImageSize(dial.NewSize());
        }
        current_project->SetAnimationFPS(dial.AnimationFPS());
    }
}

void MainWindow::on_actionApply_transform_triggered()
{
    if (!current_project)
        return;

    current_project->Canvas()->ApplyFloatingLayer(current_project->CurrentTool().opaque_apply_mode);
}

void MainWindow::on_actionDiscard_temp_changes_triggered()
{
    if (!current_project)
        return;

    current_project->Canvas()->DiscardFloatingLayer(false, true);
}

void MainWindow::on_actionImportPalette_triggered()
{
    if (!current_project)
        return;

    QString ifile_name= QFileDialog::getOpenFileName(this, "Load palette...", "",
                                                      "RIFF palette (*.pal);;Indexed image (*.png *.bmp)");

    if (ifile_name == "")
        return;

    if (!ifile_name.endsWith(".pal", Qt::CaseInsensitive)
        && !ifile_name.endsWith(".bmp", Qt::CaseInsensitive) && !ifile_name.endsWith(".png", Qt::CaseInsensitive))
    {
        QMessageBox::critical(this, "Error loading palette", "Format is not supported");
        return;
    }

    current_project->LoadPalette(ifile_name);
}

void MainWindow::on_actionExportPalette_triggered()
{
    if (!current_project)
        return;

    QString ofile_name= QFileDialog::getSaveFileName(this, "Save palette as...", "",
                                                      "Windows palette (*.pal)");

    if (ofile_name == "")
        return;

    if (!ofile_name.endsWith(".pal", Qt::CaseInsensitive))
    {
        QMessageBox::critical(this, "Error saving palette", "Output format is not supported");
        return;
    }

    current_project->SavePalette(ofile_name);
}

void MainWindow::on_actionSharedPalette_triggered(bool checked)
{
    if (!current_project)
        return;

    if (checked)
    {
        QString ifile_name= QFileDialog::getSaveFileName(this, "Set shared palette to file...", "", "Windows palette (*.pal)");

        if (ifile_name == "")
        {
            SetSharedPaletteCheckStatus(false);
            return;
        }

        if (!ifile_name.endsWith(".pal", Qt::CaseInsensitive))
        {
            QMessageBox::critical(this, "Error loading palette", "Format is not supported");
            SetSharedPaletteCheckStatus(false);
            return;
        }

        current_project->SetSharedPalette(ifile_name);
    }
    else
    {
        current_project->SetSharedPalette("");
        SetSharedPaletteCheckStatus(false);
    }
}

void MainWindow::on_actionReloadSharedPalette_triggered()
{
    if (!current_project)
        return;
    if (current_project->SharedPalette() == "")
        return;

    current_project->LoadPalette(current_project->SharedPalette());
}

void MainWindow::on_actionDeleteGfx_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->FillSelection(0);
    current_project->Canvas()->DiscardFloatingLayer(true);
}

void MainWindow::on_actionFillSelection_A_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->FillSelection(current_project->PaltableAIndex());
}

void MainWindow::on_actionFillSelection_B_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->FillSelection(current_project->PaltableBIndex());
}

void MainWindow::on_actionSwapABSelection_triggered()
{
    if (!current_project)
        return;
    QPoint tpos= current_project->PaltableAPosition();
    current_project->SetPaltableAPosition(current_project->PaltableBPosition());
    current_project->SetPaltableBPosition(tpos);
}

void MainWindow::on_actionExport_triggered()
{
    if (!current_project)
        return;
    current_project->GoExport(this);
}

void MainWindow::on_actionVertical_flip_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->FlipSelection(false, true);
}

void MainWindow::on_actionHorizontal_flip_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->FlipSelection(true, false);
}

void MainWindow::on_actionSelectAll_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->RectangleSelect(current_project->CurrentLayer()->rect(), true);
}

void MainWindow::on_actionSelectNone_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->RectangleSelect(current_project->CurrentLayer()->rect(), false);
}

void MainWindow::on_actionInvertSelection_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->InvertSelectionRegion();
}

void MainWindow::on_actionCopy_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->CopySelected();
}

void MainWindow::on_actionPaste_triggered()
{
    if (!current_project)
        return;
    if (!current_project->Canvas())
        return;
    current_project->Canvas()->Paste();
}

void MainWindow::on_actionBugReport_triggered()
{
    QDesktopServices::openUrl(QUrl("https://github.com/LoxoSoftware/IndexBourne/issues"));
}

