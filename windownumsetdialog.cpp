#include "windownumsetdialog.h"
#include "ui_windownumsetdialog.h"

windowNumSetDialog::windowNumSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::windowNumSetDialog)
{
    ui->setupUi(this);
    this->setWindowTitle("窗口设置");
    this->setFixedSize(483,416);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    ui->sensorType_1->setCurrentIndex(0);
    ui->sensorType_2->setCurrentIndex(1);
    ui->sensorType_3->setCurrentIndex(2);
    ui->sensorType_4->setCurrentIndex(3);
    ui->windowNumSet->setCurrentIndex(0);
}

windowNumSetDialog::~windowNumSetDialog()
{
    delete ui;
}

void windowNumSetDialog::handleInitalWinNum()
{
    ui->sensorType_1->setCurrentIndex(0);
    ui->sensorType_2->setCurrentIndex(1);
    ui->sensorType_3->setCurrentIndex(2);
    ui->sensorType_4->setCurrentIndex(3);
    ui->plotType_1->setCurrentIndex(0);
    ui->plotType_2->setCurrentIndex(0);
    ui->plotType_3->setCurrentIndex(0);
    ui->plotType_4->setCurrentIndex(0);
    ui->windowNumSet->setCurrentIndex(0);
}

void windowNumSetDialog::on_okButton_clicked()
{
    int windNum = ui->windowNumSet->currentIndex()+1;
    int* windPlotType = new int[4];
    int* windSensorType = new int[4];
    windPlotType[0] = ui->plotType_1->currentIndex()+1;
    windPlotType[1] = ui->plotType_2->currentIndex()+1;
    windPlotType[2] = ui->plotType_3->currentIndex()+1;
    windPlotType[3] = ui->plotType_4->currentIndex()+1;
    windSensorType[0] = ui->sensorType_1->currentIndex()+1;
    windSensorType[1] = ui->sensorType_2->currentIndex()+1;
    windSensorType[2] = ui->sensorType_3->currentIndex()+1;
    windSensorType[3] = ui->sensorType_4->currentIndex()+1;
    emit windowNumSetData(windNum,windPlotType,windSensorType);
    accept();
}

void windowNumSetDialog::on_noButton_clicked()
{
    reject();
}
