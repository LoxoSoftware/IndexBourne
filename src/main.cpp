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
