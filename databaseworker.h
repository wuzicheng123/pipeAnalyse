#ifndef DATABASEWORKER_H
#define DATABASEWORKER_H

#include <QObject>
#include "databasepool.h"

//数据库业务逻辑
class databaseWorker : public QObject
{
    Q_OBJECT
public:
    explicit databaseWorker(QObject *parent = nullptr);

signals:
    void loginResult(QString id,QString name,QString password,QString permission);

public slots:
    void handleLoginRequest(QString name,QString password);

private:

};

#endif // DATABASEWORKER_H
