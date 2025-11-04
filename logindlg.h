#ifndef LOGINDLG_H
#define LOGINDLG_H

#include <QDialog>

namespace Ui {
class loginDlg;
}

class loginDlg : public QDialog
{
    Q_OBJECT

public:
    explicit loginDlg(QWidget *parent = nullptr);
    ~loginDlg();

signals:
    void loginRequest(QString name,QString password);

private slots:
    void handletextChanged();
    void on_loginButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::loginDlg *ui;

protected:
    void showEvent(QShowEvent *event) override;
};

#endif // LOGINDLG_H
