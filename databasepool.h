#ifndef DATABASEPOOL_H
#define DATABASEPOOL_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QTimer>
#include <QQueue>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QDebug>

class DatabasePool : public QObject
{
    Q_OBJECT
public:
    static DatabasePool* getInstance();

    // 获取数据库连接
    QSqlDatabase acquireConnection(const QString &connectionName = QString());

    // 释放数据库连接
    void releaseConnection(const QString &connectionName);

    // 配置数据库连接参数
    void configure(const QString &driver, const QString &host, const QString &databaseName,
                  const QString &username, const QString &password,
                  int port = 0, int maxConnections = 10, int idleTime = 30000);

    // 销毁连接池
    void destroy();

private:
    explicit DatabasePool(QObject *parent = nullptr);
    ~DatabasePool();

    // 禁止拷贝和赋值
    DatabasePool(const DatabasePool&) = delete;
    DatabasePool& operator=(const DatabasePool&) = delete;

    // 创建新连接
    QSqlDatabase createConnection(const QString &connectionName);

    // 清理空闲连接
    void cleanupIdleConnections();

private:
    static DatabasePool* m_instance;
    static QMutex m_mutex;

    QString m_driver;
    QString m_host;
    QString m_databaseName;
    QString m_username;
    QString m_password;
    int m_port;
    int m_maxConnections;
    int m_idleTime;

    struct ConnectionInfo {
        QDateTime lastUsed;
        bool inUse;
    };

    QQueue<QString> m_availableConnections;
    QHash<QString, ConnectionInfo> m_connectionInfo;
    QMutex m_poolMutex;
    QTimer *m_cleanupTimer;
};

// 初始化连接池
void initDatabasePool();

class DatabaseConnection {
public:
    explicit DatabaseConnection(const QString &connectionName = QString());

    ~DatabaseConnection();

    QSqlDatabase& database();

    bool isValid() const;

private:
    QSqlDatabase m_db;
};

#endif // DATABASEPOOL_H
