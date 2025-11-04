#include "databaseworker.h"
#include "QSqlQuery"
#include "QSqlResult"
#include "QSqlRecord"
#include "QThread"

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
    }
    QSqlDatabase db = conn.database();
    QSqlQuery query(db);
    QString sql = "use pipeanalyse";
    bool ret = query.exec(sql);
    if(!ret)
    {
        qDebug()<<"use pipeanalyse err;"<<query.lastError().text();
    }
    sql = QString("select id,name,password,permission from users where "
                  "name='%1' and password='%2'").arg(name).arg(password);
    ret = query.exec(sql);
    QString retPermisson="";
    QString id="";
    if(ret)
    {
        if(query.next())
        {
            id = query.value(0).toString();
            retPermisson = query.value(3).toString();
        }
    }
    emit loginResult(id,name,password,retPermisson);
}
