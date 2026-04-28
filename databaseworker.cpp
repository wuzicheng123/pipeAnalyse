#include "databaseworker.h"
#include <QSqlQuery>
#include <QSqlResult>
#include <QSqlRecord>
#include <QThread>
#include "msgbox.h"

databaseWorker::databaseWorker(QObject *parent) : QObject(parent)
{
    initDatabasePool();
}

void databaseWorker::handleLoginRequest(QString name, QString password)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("select id,name,password,permission from users where "
                  "name='%1' and password='%2'").arg(name).arg(password);
    ret = query.exec(sql);
    QString retPermisson="";
    int id=-1;
    if(ret)
    {
        if(query.next())
        {
            id = query.value(0).toInt();
            retPermisson = query.value(3).toString();
        }
    }
    emit loginResult(id,name,password,retPermisson);
}

void databaseWorker::handleQueryAllUsers()
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("select name,permission from users");
    QVector<userDataModel>vecUsers;
    ret = query.exec(sql);
    if(ret)
    {
        int nField = query.record().count();
        while(query.next())
        {
            if(2 == nField)
            {
                userDataModel oneUser;
                oneUser.name = query.value(0).toString();
                oneUser.permission = query.value(1).toString();
                vecUsers.append(oneUser);
            }
        }
        emit qryAllUsersResult(vecUsers);
    }
}

void databaseWorker::handleNewUserRequest(QString name, QString password, QString permission)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("insert into users(name,password,permission) values "
                  "('%1','%2','%3')").arg(name).arg(password).arg(permission);
    ret = query.exec(sql);
    if(ret)
    {
        //tableview添加显示
        emit showAddNewUser(name,password,permission);
    }
    else {
        msgBox::show("警告","已存在同名用户",2);
    }
}

void databaseWorker::handleEditUserRequest(int row, QString preName, QString name, QString password, QString permission)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    int index = -1;
    sql = QString("select id from users where name='%1'").arg(preName);
    ret = query.exec(sql);
    if(ret)
    {
        while (query.next()) {
            index = query.value(0).toInt();
        }
        if(-1 != index)
        {
            sql = QString("update users set name='%1',password='%2',permission='%3' where "
                          "id=%4").arg(name).arg(password).arg(permission).arg(index);
            ret = query.exec(sql);
            if(ret)
            {
                //tableview刷新显示
                emit showEditUser(row,name,password,permission);
            }
            else {
                msgBox::show("警告","用户修改失败",2);
            }
        }
    }
}

void databaseWorker::handleDeleteUserRequest(int row, QString name)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("delete from users where name='%1'").arg(name);
    ret = query.exec(sql);
    if(ret)
    {
        emit showDeleteUser(row);
    }
    else {
        msgBox::show("警告","用户删除失败",2);
    }
}

void databaseWorker::handleQueryAllProjects()
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("select name,discript,wallthicknesstype,sampleinterval,wallthicknessnumber,"
                  "outerDiameter,createtime,creator,id from project");
    ret = query.exec(sql);
    QVector<projectDataModel>vecPrjs;
    if(ret)
    {
        int nfield = query.record().count();
        while(query.next())
        {
            if(9 == nfield)
            {
                projectDataModel oneProject;
                oneProject.name = query.value(0).toString();
                oneProject.discript = query.value(1).toString();
                oneProject.wallthicknesstype = query.value(2).toString();
                oneProject.sampleinterval = query.value(3).toDouble();
                oneProject.dwallthickness = query.value(4).toDouble();
                oneProject.outerDiameter = query.value(5).toDouble();
                oneProject.createtime = query.value(6).toString();
                oneProject.creator = query.value(7).toInt();
                oneProject.id = query.value(8).toInt();
                vecPrjs.append(oneProject);
            }
        }
        int iSize = vecPrjs.size();
        for(int i=0;i<iSize;i++)
        {
            sql = QString("select name from users where id=%1").arg(vecPrjs[i].creator);
            ret = query.exec(sql);
            if(ret)
            {
                if(query.next())
                {
                    vecPrjs[i].creatorName = query.value(0).toString();
                }
            }
        }
        //emit到prj显示界面
        emit qryAllPrjsResult(vecPrjs);
    }
}

void databaseWorker::handleQueryProjectById(int id,int type)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    if(1 == type)
    {
        sql = QString("select name,sampleinterval,wallthicknessnumber,"
                      "outerDiameter,datapath from project where id=%1").arg(id);
        ret = query.exec(sql);
        if(ret)
        {
            int nField = query.record().count();
            if(query.next())
            {

                if(5 == nField)
                {
                    projectDataModel oneProject;
                    oneProject.name = query.value(0).toString();
                    oneProject.sampleinterval = query.value(1).toDouble();
                    oneProject.dwallthickness = query.value(2).toDouble();
                    oneProject.outerDiameter = query.value(3).toDouble();
                    oneProject.datapath = query.value(4).toString();
                    oneProject.id = id;
                    //emit
                    emit qryProjectByIdResult(oneProject);
                }
            }
        }
    }
    else if(2 == type)
    {
        sql = QString("select name,discript,wallthicknesstype,sampleinterval,wallthicknessnumber,"
                      "outerDiameter,datapath from project where id=%1").arg(id);
        ret = query.exec(sql);
        if(ret)
        {
            int nField = query.record().count();
            if(query.next())
            {
                if(7 == nField)
                {
                    projectDataModel oneProject;
                    oneProject.name = query.value(0).toString();
                    oneProject.discript = query.value(1).toString();
                    oneProject.wallthicknesstype = query.value(2).toString();
                    oneProject.sampleinterval = query.value(3).toDouble();
                    oneProject.dwallthickness = query.value(4).toDouble();
                    oneProject.outerDiameter = query.value(5).toDouble();
                    oneProject.datapath = query.value(6).toString();
                    //emit
                    emit showDetailProject(oneProject);
                }
            }
        }
    }
}

void databaseWorker::handleNewProjectRequest(projectDataModel &projectData)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("insert into project (name,discript,sampleinterval,wallthicknessnumber,outerDiameter,wallthicknesstype,"
                  "datapath,creator) values('%1','%2',%3,'%4','%5','%6',(?),%8)").arg(projectData.name)
            .arg(projectData.discript).arg(projectData.sampleinterval).arg(projectData.dwallthickness).arg(projectData.outerDiameter)
            .arg(projectData.wallthicknesstype).arg(projectData.creator);
    //使用占位符,Qt自动处理转义
    query.prepare(sql);
    query.addBindValue(projectData.datapath);
    ret = query.exec();
    if(ret)
    {
        //tableview添加显示
        //查询已添加的结果
        sql = QString("select name,discript,sampleinterval,wallthicknessnumber,"
                      "outerDiameter,wallthicknesstype,createtime,"
                      "creator,id from project where name='%1' order by createtime desc")
                .arg(projectData.name);
        ret = query.exec(sql);
        if(ret)
        {
            int nField = query.record().count();
            if(query.next())
            {
                if(9 == nField)
                {
                    projectDataModel oneprjDM;
                    oneprjDM.name = query.value(0).toString();
                    oneprjDM.discript =query.value(1).toString();
                    oneprjDM.sampleinterval = query.value(2).toDouble();
                    oneprjDM.dwallthickness = query.value(3).toDouble();
                    oneprjDM.outerDiameter = query.value(4).toDouble();
                    oneprjDM.wallthicknesstype = query.value(5).toString();
                    oneprjDM.createtime = query.value(6).toString();
                    oneprjDM.creatorName = projectData.creatorName;
                    oneprjDM.id = query.value(8).toInt();
                    emit showAddNewProject(oneprjDM);
                }
            }
        }
    }
}

void databaseWorker::handleEditProjectRequest(int row, projectDataModel &projectData)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("update project set name='%1',discript='%2',sampleinterval=%3,wallthicknessnumber=%4,"
                  "outerDiameter=%5,wallthicknesstype='%6',datapath='%7' where id=%8").arg(projectData.name)
            .arg(projectData.discript).arg(projectData.sampleinterval).arg(projectData.dwallthickness)
            .arg(projectData.outerDiameter).arg(projectData.wallthicknesstype)
            .arg(projectData.datapath).arg(projectData.id);
    //使用占位符,Qt自动处理转义
    query.prepare(sql);
    query.addBindValue(projectData.datapath);
    ret = query.exec();
    if(ret)
    {
        //emit 更新列表显示
        emit showEditProject(row,projectData);
    }
    else {
        msgBox::show("警告","项目修改失败",2);
    }
}

void databaseWorker::handleDeleteProjectRequest(int row, int projectId)
{
    DatabaseConnection conn;
    if(!conn.isValid())
    {
        qDebug()<<"数据库连接池获取失败";
        return;
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
        return;
    }
    sql = QString("delete from project where id=%1").arg(projectId);
    ret = query.exec(sql);
    if(ret)
    {
        emit showDeleteProject(row);
    }
    else {
        msgBox::show("警告","项目删除失败",2);
    }
}
