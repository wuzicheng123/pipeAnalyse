#ifndef NEWUSERDLG_H
#define NEWUSERDLG_H

#include <QDialog>

namespace Ui {
class newuserdlg;
}

//修改可与新建共用同一界面
class newuserdlg : public QDialog
{
    Q_OBJECT

public:
    explicit newuserdlg(QWidget *parent = nullptr);
    ~newuserdlg();

    void trans2editDlg(int row,QString name, QString permission);

signals:
    void newUserRequest(QString name,QString password,QString permission);
    void editUserRequest(int row,QString name,QString password,QString permission);

private slots:
    void handletextChanged();

    void on_okButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::newuserdlg *ui;

    int m_row;
};

#endif // NEWUSERDLG_H
