#ifndef RESIZEDIALOG_H
#define RESIZEDIALOG_H

#include <QDialog>

namespace Ui {
class ResizeDialog;
}

class ResizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ResizeDialog(QWidget *parent = nullptr, QSize default_size = QSize(64, 64));
    ~ResizeDialog();

    bool GetAccepted();
    QSize NewSize();

private slots:
    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

private:
    Ui::ResizeDialog *ui;

    bool is_accepted= false;
};

#endif // RESIZEDIALOG_H
