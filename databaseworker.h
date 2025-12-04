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

signals:
    void loginResult(int id,QString name,QString password,QString permission);
    void qryAllUsersResult(QVector<userDataModel>&vecUsers);
    void showAddNewUser(QString name,QString password,QString permission);
    void showEditUser(int row,QString name,QString password,QString permission);
    void showDeleteUser(int row);
    void qryAllPrjsResult(QVector<projectDataModel>&vecPrjs);
    void qryProjectByIdResult(projectDataModel& onePrj);

public slots:
    void handleLoginRequest(QString name,QString password);
    void handleQueryAllUsers();
    void handleNewUserRequest(QString name,QString password,QString permission);
    void handleEditUserRequest(int row,QString preName,QString name,QString password,QString permission);
    void handleDeleteUserRequest(int row,QString name);
    void handleQueryAllProjects();
    void handleQueryProjectById(int id);

private:

};

#endif // DATABASEWORKER_H
