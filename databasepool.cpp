#include "databasepool.h"

DatabasePool* DatabasePool::m_instance = nullptr;
QMutex DatabasePool::m_mutex;

DatabasePool* DatabasePool::getInstance()
{
    QMutexLocker locker(&m_mutex);
    if (!m_instance) {
        m_instance = new DatabasePool();
    }
    return m_instance;
}

DatabasePool::DatabasePool(QObject *parent) : QObject(parent)
{
    m_maxConnections = 10;
    m_idleTime = 30000;  // 30秒

    // 初始化清理定时器
    m_cleanupTimer = new QTimer(this);
    connect(m_cleanupTimer, &QTimer::timeout, this, &DatabasePool::cleanupIdleConnections);
    m_cleanupTimer->start(10000);  // 每10秒检查一次
}

DatabasePool::~DatabasePool()
{
    destroy();
}

void DatabasePool::configure(const QString &driver, const QString &host, const QString &databaseName,
                            const QString &username, const QString &password,
                            int port, int maxConnections, int idleTime)
{
    QMutexLocker locker(&m_poolMutex);

    m_driver = driver;
    m_host = host;
    m_databaseName = databaseName;
    m_username = username;
    m_password = password;
    m_port = port;
    m_maxConnections = maxConnections;
    m_idleTime = idleTime;
}

QSqlDatabase DatabasePool::acquireConnection(const QString &connectionName)
{
    QMutexLocker locker(&m_poolMutex);

    QString name = connectionName;
    if (name.isEmpty()) {
        // 生成唯一的连接名称
        static int counter = 0;
        name = QString("Connection_%1").arg(++counter);
    }

    // 检查是否有可用的连接
    while (!m_availableConnections.isEmpty()) {
        QString availableName = m_availableConnections.dequeue();
        ConnectionInfo &info = m_connectionInfo[availableName];

        // 检查连接是否有效
        QSqlDatabase db = QSqlDatabase::database(availableName, false);
        if (db.isOpen() && db.isValid()) {
            info.inUse = true;
            info.lastUsed = QDateTime::currentDateTime();
            return db;
        } else {
            // 连接无效，移除并关闭
            QSqlDatabase::removeDatabase(availableName);
            m_connectionInfo.remove(availableName);
        }
    }

    // 没有可用连接，检查是否可以创建新连接
    if (m_connectionInfo.size() < m_maxConnections) {
        QSqlDatabase db = createConnection(name);
        ConnectionInfo info;
        info.inUse = true;
        info.lastUsed = QDateTime::currentDateTime();
        m_connectionInfo[name] = info;
        return db;
    }

    // 达到最大连接数，无法创建新连接
    qWarning() << "DatabasePool: 达到最大连接数，无法获取连接";
    return QSqlDatabase();
}

void DatabasePool::releaseConnection(const QString &connectionName)
{
    QMutexLocker locker(&m_poolMutex);

    if (m_connectionInfo.contains(connectionName)) {
        ConnectionInfo &info = m_connectionInfo[connectionName];
        info.inUse = false;
        info.lastUsed = QDateTime::currentDateTime();
        m_availableConnections.enqueue(connectionName);
    } else {
        qWarning() << "DatabasePool: 尝试释放不存在的连接:" << connectionName;
    }
}

QSqlDatabase DatabasePool::createConnection(const QString &connectionName)
{
    //此处修改为odbc连接数据库
    QSqlDatabase db = QSqlDatabase::addDatabase(m_driver, connectionName);

    QString dsn = QString("Driver={MySQL ODBC 8.4 Unicode Driver};SERVER=%1;"
                          "DATABASE=%2;UID=%3;"
                          "PWD=%4;").arg(m_host).arg(m_databaseName).arg(m_username).arg(m_password);
    db.setDatabaseName(dsn);
    if (m_port > 0) {
        db.setPort(m_port);
    }

    if (!db.open()) {
        qCritical() << "DatabasePool: 无法创建数据库连接:" << db.lastError().text();
        QSqlDatabase::removeDatabase(connectionName);
        return QSqlDatabase();
    }

    return db;
}

void DatabasePool::cleanupIdleConnections()
{
    QMutexLocker locker(&m_poolMutex);

    QDateTime now = QDateTime::currentDateTime();
    QList<QString> connectionsToRemove;

    // 查找所有空闲时间超过阈值的连接
    for (auto it = m_connectionInfo.begin(); it != m_connectionInfo.end(); ++it) {
        const QString &connectionName = it.key();
        const ConnectionInfo &info = it.value();

        if (!info.inUse && info.lastUsed.msecsTo(now) > m_idleTime) {
            connectionsToRemove.append(connectionName);
        }
    }

    // 移除并关闭这些连接
    for (const QString &connectionName : connectionsToRemove) {
        // 从可用队列中移除
        m_availableConnections.removeAll(connectionName);

        // 关闭并移除数据库连接
        QSqlDatabase::database(connectionName, false).close();
        QSqlDatabase::removeDatabase(connectionName);

        // 从连接信息中移除
        m_connectionInfo.remove(connectionName);
    }
}

void DatabasePool::destroy()
{
    QMutexLocker locker(&m_poolMutex);

    // 停止清理定时器
    m_cleanupTimer->stop();

    // 关闭并移除所有数据库连接
    for (const QString &connectionName : m_connectionInfo.keys()) {
        QSqlDatabase::database(connectionName, false).close();
        QSqlDatabase::removeDatabase(connectionName);
    }

    // 清空所有数据结构
    m_availableConnections.clear();
    m_connectionInfo.clear();
}

// 初始化连接池
void initDatabasePool()
{
    DatabasePool::getInstance()->configure(
        "QODBC",               // 数据库驱动
        "127.0.0.1",            // 主机名
        "pipeanalyse",           // 数据库名
        "pipeanalyse_user",             // 用户名
        "123456",             // 密码
        3306,                   // 端口
        10,                     // 最大连接数
        30000                   // 空闲超时时间（毫秒）
    );
}


DatabaseConnection::DatabaseConnection(const QString &connectionName)
{
    m_db = DatabasePool::getInstance()->acquireConnection(connectionName);
}

DatabaseConnection::~DatabaseConnection()
{
    if (m_db.isOpen()) {
        DatabasePool::getInstance()->releaseConnection(m_db.connectionName());
    }
}

QSqlDatabase &DatabaseConnection::database()
{
    return m_db;
}

bool DatabaseConnection::isValid() const
{
    return m_db.isOpen();
}
