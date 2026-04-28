#ifndef PROJECTDLG_H
#define PROJECTDLG_H

#include <QDialog>
#include "define.h"

namespace Ui {
class projectDlg;
}

class projectDlg : public QDialog
{
    Q_OBJECT

public:
    explicit projectDlg(QWidget *parent = nullptr);
    ~projectDlg();

    void trans2newDlg();
    void setCurrentUser(userDataModel userData);
    void trans2editDlg(int projectId, QString prjName, QString prjDiscribe, QString thicknessType,
                       double sampleinterval, double thicknessNumber, double outerDiameter, QString datapath, int row);
    void trans2detailDlg(projectDataModel& oneDatamodel);

signals:
    void newProjectRequest(projectDataModel& projectData);
    void editProjectRequest(int row,projectDataModel& projectData);

private slots:
    void handletextChanged();

    void on_okButton_clicked();

    void on_cancelButton_clicked();

    QString on_datapathButton_clicked();

private:
    Ui::projectDlg *ui;
    userDataModel m_userData;//new时用
    int m_prjId; //edit时用
    int m_row;//edit时用
};

#endif // PROJECTDLG_H
