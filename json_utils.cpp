#include "json_utils.h"
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDebug>

QVariant parseJson(const QString &json, bool *ok, QString *errorMsg)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);

    if (error.error != QJsonParseError::NoError) {
        if (ok) *ok = false;
        if (errorMsg) *errorMsg = QString("JSON parse error at offset %1: %2")
                                  .arg(error.offset)
                                  .arg(error.errorString());
        return QVariant();
    }

    if (doc.isNull()) {
        if (ok) *ok = false;
        if (errorMsg) *errorMsg = QString("Null JSON document");
        return QVariant();
    }

    if (ok) *ok = true;
    if (errorMsg) errorMsg->clear();
    return doc.toVariant();
}

QString buildJson(const QVariant &data, bool compact, bool *ok)
{
    QJsonDocument doc = QJsonDocument::fromVariant(data);

    if (doc.isNull()) {
        if (ok) *ok = false;
        qWarning() << "buildJson: failed to convert QVariant to JSON document";
        return QString();
    }

    if (ok) *ok = true;
    return compact ? doc.toJson(QJsonDocument::Compact)
                   : doc.toJson(QJsonDocument::Indented);
}

QVariantMap toMap(const QVariant &var)
{
    if (var.type() == QVariant::Map || var.canConvert<QVariantMap>()) {
        return var.toMap();
    }
    return QVariantMap();
}

QVariantList toList(const QVariant &var)
{
    if (var.type() == QVariant::List || var.canConvert<QVariantList>()) {
        return var.toList();
    }
    return QVariantList();
}

QString toString(const QVariant &var)
{
    return var.toString();
}

double toDouble(const QVariant &var, bool *ok)
{
    bool success = false;
    double result = var.toDouble(&success);
    if (ok) *ok = success;
    return result;
}

bool toBool(const QVariant &var, bool *ok)
{
    // 注意：QVariant::toBool() 会自动将数字 0->false, 非0->true
    // 如果类型无法转换，会返回 false 并将 ok 置为 false
    if(var.isNull())
    {
        *ok = false;
        return false;
    }
    bool result = var.toBool();
    return result;
}

bool isNullVariant(const QVariant &var)
{
    return var.isNull() || var.type() == QVariant::Invalid;
}
