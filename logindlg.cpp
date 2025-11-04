#include "logindlg.h"
#include "ui_logindlg.h"
#include "QThread"
#include "QDebug"

loginDlg::loginDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::loginDlg)
{
    ui->setupUi(this);
    this->setWindowTitle("登录窗口");
    this->setFixedSize(476,283);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->passwordEdit->setEchoMode(QLineEdit::Password);
    ui->loginButton->setEnabled(false);
    ui->tooltip_1->setStyleSheet("color:red");
    ui->tooltip_2->setStyleSheet("color:red");
    connect(ui->nameEdit,&QLineEdit::textChanged,this,&loginDlg::handletextChanged);
    connect(ui->passwordEdit,&QLineEdit::textChanged,this,&loginDlg::handletextChanged);
}

loginDlg::~loginDlg()
{
    delete ui;
}

void loginDlg::handletextChanged()
{
    bool nameeditValid = (ui->nameEdit->text().size() != 0);
    bool passwordeditValid = (ui->passwordEdit->text().size() != 0);
    if(!nameeditValid)
    {
        ui->tooltip_1->setText("是必须的！");
    }
    else {
        ui->tooltip_1->setText("");
    }
    if(!passwordeditValid)
    {
        ui->tooltip_2->setText("是必须的！");
    }
    else {
        ui->tooltip_2->setText("");
    }
    if(nameeditValid && passwordeditValid)
    {
        ui->loginButton->setEnabled(true);
    }
    else {
        ui->loginButton->setEnabled(false);
    }
}

void loginDlg::on_loginButton_clicked()
{
    QString name = ui->nameEdit->text();
    QString password = ui->passwordEdit->text();
    emit loginRequest(name,password);
    accept();
}

void loginDlg::on_cancelButton_clicked()
{
    reject();
}

void loginDlg::showEvent(QShowEvent *event)
{
    QDialog::showEvent(event);
    ui->nameEdit->setText("");
    ui->passwordEdit->setText("");
    ui->tooltip_1->setText("");
    ui->tooltip_2->setText("");

    //调试时临时使用，未来删除
    ui->nameEdit->setText("admin");
    ui->passwordEdit->setText("123");
}
