#include "projectdlg.h"
#include "ui_projectdlg.h"
#include <QDir>
#include "msgbox.h"
#include <QFileDialog>

projectDlg::projectDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::projectDlg)
{
    ui->setupUi(this);
    this->setWindowTitle("新建项目");
    this->setFixedSize(579,234);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->okButton->setEnabled(false);
    ui->tooltip_1->setStyleSheet("color:red");
    ui->tooltip_2->setStyleSheet("color:red");
    ui->tooltip_3->setStyleSheet("color:red");
    ui->tooltip_4->setStyleSheet("color:red");
    connect(ui->prjNameEdit,&QLineEdit::textChanged,this,&projectDlg::handletextChanged);
    connect(ui->thicknessEdit,&QLineEdit::textChanged,this,&projectDlg::handletextChanged);
    connect(ui->intervalEdit,&QLineEdit::textChanged,this,&projectDlg::handletextChanged);
    connect(ui->datapathEdit,&QLineEdit::textChanged,this,&projectDlg::handletextChanged);
    m_row = -1;
}

projectDlg::~projectDlg()
{
    delete ui;
}

void projectDlg::trans2newDlg()
{
    this->setWindowTitle("新建项目");
}

void projectDlg::setCurrentUser(userDataModel userData)
{
    m_userData = userData;
}

void projectDlg::trans2editDlg(int projectId, QString prjName, QString prjDiscribe, QString thicknessType, double sampleinterval, QString datapath,int row)
{
    this->setWindowTitle("修改项目");
    m_prjId = projectId;
    m_row = row;
    ui->prjNameEdit->setText(prjName);
    ui->discribeEdit->setText(prjDiscribe);
    ui->thicknessEdit->setText(thicknessType);
    ui->intervalEdit->setText(QString::number(sampleinterval));
    ui->datapathEdit->setText(datapath);
}

void projectDlg::handletextChanged()
{
    bool nameeditValid = (ui->prjNameEdit->text().size() != 0);
    bool thicknessValid = (ui->thicknessEdit->text().size() != 0);
    bool intervalValid = (ui->intervalEdit->text().size() != 0);
    bool datapathValid = (ui->datapathEdit->text().size() != 0);
    QString sampleInterval = ui->intervalEdit->text();
    bool ok;
    double value = sampleInterval.toDouble(&ok);
    if(!nameeditValid)
    {
        ui->tooltip_1->setText("是必须的！");
    }
    else {
        ui->tooltip_1->setText("");
    }
    if(!thicknessValid)
    {
        ui->tooltip_2->setText("是必须的！");
    }
    else {
        ui->tooltip_2->setText("");
    }
    if(!intervalValid)
    {
        ui->tooltip_4->setText("是必须的！");
    }
    else {
        if(ok && value>0)//验证数字合法性
        {
            ui->tooltip_4->setText("");
        }
        else{
            ui->tooltip_4->setText("非大于0的数值");
        }
    }
    if(!datapathValid)
    {
        ui->tooltip_3->setText("是必须的！");
    }
    else {
        ui->tooltip_3->setText("");
    }
    if(nameeditValid && thicknessValid && ok && value>0 && datapathValid)
    {
        ui->okButton->setEnabled(true);
    }
    else {
        ui->okButton->setEnabled(false);
    }
}

void projectDlg::on_okButton_clicked()
{
    projectDataModel oneProjectData;
    oneProjectData.name = ui->prjNameEdit->text();
    oneProjectData.discript = ui->discribeEdit->text();
    oneProjectData.wallthicknesstype = ui->thicknessEdit->text();
    oneProjectData.sampleinterval = ui->intervalEdit->text().toDouble();
    oneProjectData.datapath = ui->datapathEdit->text();
    //确保datapath末尾有符号'\'
    if((!oneProjectData.datapath.isEmpty() && !oneProjectData.datapath.endsWith('\\')) ||
            (!oneProjectData.datapath.isEmpty() && !oneProjectData.datapath.endsWith('/')))
    {
        oneProjectData.datapath.append('/');
    }
    oneProjectData.creator = m_userData.id;
    oneProjectData.creatorName = m_userData.name;
    //项目路径合法性检查
    QDir dir(oneProjectData.datapath);
    if(!dir.exists())
    {
        msgBox::show("告警","路径验证错误，文件夹不存在",2);
        return;
    }
    QVector<QString>boxDirPath;
    //获取文件夹下box子文件夹，及box文件夹内的文件名
    QFileInfoList folderInfos = dir.entryInfoList(QDir::Dirs|QDir::NoDotAndDotDot,
                                                  QDir::Name|QDir::IgnoreCase);
    for(const QFileInfo &info:folderInfos)
    {
        QString absolutePath = info.absoluteFilePath()+"/";
        boxDirPath.append(absolutePath);
    }
    int boxNum = boxDirPath.size();
    if(0 == boxNum)
    {
        msgBox::show("告警","路径验证错误，盒子文件夹不存在",2);
        return;
    }
    for(int i=0;i<boxNum;i++)
    {
        QString boxPath = boxDirPath[i];
        QDir boxDir(boxPath);
        QStringList filters = {"*.bin"};
        QFileInfoList files = boxDir.entryInfoList(filters,QDir::Files,QDir::Name);
        if(files.isEmpty())
        {
            msgBox::show("告警",QString("路径验证错误，盒子:%1路径下无数据文件").arg(boxPath),2);
            return;
        }
    }
    //emit(需用户id)
    if("新建项目" == this->windowTitle())
    {
        emit newProjectRequest(oneProjectData);
    }
    else if("修改项目" == this->windowTitle())
    {
        if(-1 != m_row)
        {
            oneProjectData.id = m_prjId;
            emit editProjectRequest(m_row,oneProjectData);
        }
    }
    accept();
}

void projectDlg::on_cancelButton_clicked()
{
    reject();
}

QString projectDlg::on_datapathButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(nullptr,"选择文件夹",
                                                           QDir::homePath(),
                                                           QFileDialog::ShowDirsOnly|QFileDialog::DontResolveSymlinks);
    ui->datapathEdit->setText(folderPath);
    return folderPath;
}
