#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#include <QString>
#include <QVariant>

/**
 * @brief 解析JSON字符串为QVariant对象
 * @param json 符合JSON格式的字符串
 * @param ok 可选输出参数，解析成功时为true，失败时为false
 * @param errorMsg 可选输出参数，存储详细的错误信息
 * @return 解析成功返回对应的QVariant（可以是QVariantMap、QVariantList或基本类型），
 *         失败时返回空QVariant
 */
QVariant parseJson(const QString &json, bool *ok = nullptr, QString *errorMsg = nullptr);

/**
 * @brief 从QVariant对象构建JSON字符串
 * @param data 要序列化的数据，支持QVariantMap、QVariantList、基本类型等
 * @param compact 是否输出紧凑格式（无空格换行），false则输出带缩进的格式化字符串
 * @param ok 可选输出参数，构建成功时为true，失败时为false
 * @return 构建成功返回JSON字符串，失败返回空字符串
 */
QString buildJson(const QVariant &data, bool compact = false, bool *ok = nullptr);

/**
 * @brief 安全地将 QVariant 转换为 QVariantMap
 * @param var 待转换的变体
 * @return 如果 var 可以转换为 QVariantMap 则返回映射，否则返回空映射
 */
QVariantMap toMap(const QVariant &var);

/**
 * @brief 安全地将 QVariant 转换为 QVariantList
 * @param var 待转换的变体
 * @return 如果 var 可以转换为 QVariantList 则返回列表，否则返回空列表
 */
QVariantList toList(const QVariant &var);

/**
 * @brief 安全地将 QVariant 转换为 QString
 * @param var 待转换的变体
 * @return 字符串表示形式（支持数字、布尔、null 等）
 */
QString toString(const QVariant &var);

/**
 * @brief 安全地将 QVariant 转换为 double
 * @param var 待转换的变体
 * @param ok  可选输出参数，转换成功为 true，失败为 false
 * @return 转换后的 double 值，失败返回 0.0
 */
double toDouble(const QVariant &var, bool *ok);

/**
 * @brief 安全地将 QVariant 转换为 bool
 * @param var 待转换的变体
 * @param ok  可选输出参数，转换成功为 true，失败为 false
 * @return 转换后的 bool 值，失败返回 false
 */
bool toBool(const QVariant &var, bool *ok = nullptr);

/**
 * @brief 判断 QVariant 是否为 null 或无效
 * @param var 待判断的变体
 * @return 如果 var 是 null 或类型为 QVariant::Invalid 则返回 true
 */
bool isNullVariant(const QVariant &var);

// 通用模板：直接使用 QVariant 的转换构造函数
template <typename T>
QVariant toVariant(const T &value) {
    return QVariant(value);
}

// 特化处理 nullptr（表示 Invalid）
template <>
inline QVariant toVariant<std::nullptr_t>(const std::nullptr_t&) {
    return QVariant();
}

// 可选：针对 QVariant 自身，直接返回
template <>
inline QVariant toVariant<QVariant>(const QVariant &var) {
    return var;
}

#endif // JSON_UTILS_H
