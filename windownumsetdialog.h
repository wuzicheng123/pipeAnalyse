#ifndef WINDOWNUMSETDIALOG_H
#define WINDOWNUMSETDIALOG_H

#include <QDialog>

namespace Ui {
class windowNumSetDialog;
}

class windowNumSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit windowNumSetDialog(QWidget *parent = nullptr);
    ~windowNumSetDialog();
signals:
    void windowNumSetData(int windNum,int* windPlotType,int* windSensorType);

private slots:
    void on_okButton_clicked();

    void on_noButton_clicked();

public slots:
    void handleInitalWinNum();

private:
    Ui::windowNumSetDialog *ui;
};

#endif // WINDOWNUMSETDIALOG_H
