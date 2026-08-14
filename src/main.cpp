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
#include "src/project.h"

#include <QApplication>
#include <QMessageBox>

MainWindow* main_window;
extern Project* current_project;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    a.setApplicationDisplayName("IndexBourne");
    main_window= &w;
    w.show();
    if (current_project && argc >= 2)
    {
        //Open project from launch argument
        if (current_project->LoadProject(argv[1]))
            current_project->SetFileName(argv[1]);
        else if (current_project->ImportBitmap(QImage(argv[1]), Consent_Force, Consent_Force))
            current_project->SetFileName(argv[1]);
        else
            QMessageBox::critical(w.CentralWidget(), "Cannot open project", "Unable to open project \""+QString(argv[1])+"\"");
    }
    return QApplication::exec();
}
