#include "newuserdlg.h"
#include "ui_newuserdlg.h"

newuserdlg::newuserdlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::newuserdlg)
{
    ui->setupUi(this);
    this->setWindowTitle("新建用户");
    this->setFixedSize(505,357);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->okButton->setEnabled(false);
    ui->tooltip_1->setStyleSheet("color:red");
    ui->tooltip_2->setStyleSheet("color:red");
    connect(ui->nameEdit,&QLineEdit::textChanged,this,&newuserdlg::handletextChanged);
    connect(ui->passwordEdit,&QLineEdit::textChanged,this,&newuserdlg::handletextChanged);
    m_row = -1;
    m_name= "";
}

newuserdlg::~newuserdlg()
{
    delete ui;
}

void newuserdlg::trans2editDlg(int row, QString name, QString permission)
{
    this->setWindowTitle("修改用户");
    ui->title->setText("修改用户");
    ui->nameEdit->setText(name);
    if("操作员" == permission)
    {
        ui->permisssionComboBox->setCurrentIndex(0);
    }
    else{
        ui->permisssionComboBox->setCurrentIndex(1);
    }
    m_row = row;
    m_name = name;
}

void newuserdlg::trans2newDlg()
{
    this->setWindowTitle("新建用户");
    ui->title->setText("新建用户");
    ui->permisssionComboBox->setCurrentIndex(0);
}

void newuserdlg::handletextChanged()
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
        ui->okButton->setEnabled(true);
    }
    else {
        ui->okButton->setEnabled(false);
    }
}

void newuserdlg::on_okButton_clicked()
{
    QString name = ui->nameEdit->text();
    QString password = ui->passwordEdit->text();
    QString permission = ui->permisssionComboBox->currentText();
    if("新建用户" == ui->title->text())
    {
        emit newUserRequest(name,password,permission);
    }
    else {
        if(m_row > -1 && "" != m_name)
        {
            emit editUserRequest(m_row,m_name,name,password,permission);
        }
    }
    accept();
}

void newuserdlg::on_cancelButton_clicked()
{
    reject();
}

