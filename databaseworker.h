#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include "databasepool.h"
#include "define.h"

//数据库业务逻辑
class databaseWorker : public QObject
{
    Q_OBJECT
public:
    explicit databaseWorker(QObject *parent = nullptr);
    //检查表是否存在
    bool tableExists(QSqlQuery& query, const QString& tableName);

signals:
    void loginResult(int id,QString name,QString password,QString permission);
    void qryAllUsersResult(QVector<userDataModel>&vecUsers);
    void showAddNewUser(QString name,QString password,QString permission);
    void showEditUser(int row,QString name,QString password,QString permission);
    void showDeleteUser(int row);
    void qryAllPrjsResult(QVector<projectDataModel>&vecPrjs);
    void qryProjectByIdResult(projectDataModel& onePrj);
    void showAddNewProject(projectDataModel& onePrj);
    void showEditProject(int row,projectDataModel& onePrj);
    void showDeleteProject(int row);
    void showDetailProject(projectDataModel& onePrj);
    void showDefectsInAxial(QVector<DefectEvent>vecDefects,int openPrjId);

public slots:
    void handleLoginRequest(QString name,QString password);
    void handleQueryAllUsers();
    void handleNewUserRequest(QString name,QString password,QString permission);
    void handleEditUserRequest(int row,QString preName,QString name,QString password,QString permission);
    void handleDeleteUserRequest(int row,QString name);
    void handleQueryAllProjects();
    void handleQueryProjectById(int id,int type);
    void handleNewProjectRequest(projectDataModel& projectData);
    void handleEditProjectRequest(int row,projectDataModel& projectData);
    void handleDeleteProjectRequest(int row,int projectId);
    void handleAddNewdefects(QVector<DefectEvent> resultBlockEvents, int currentPrjId, bool bfirst);
    void handleQueryDefectsInAxial(int openPrjId,double x_start,double x_end,double y_start,double y_end);

private:

};

#endif // DATABASEWORKER_H
