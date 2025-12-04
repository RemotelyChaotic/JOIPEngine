#ifndef COMMONSCRIPTHELPERS_H
#define COMMONSCRIPTHELPERS_H

#include <QtLua/Value>

#include <QColor>
#include <QJSValue>
#include <QVariant>

#include <memory>
#include <optional>
#include <variant>

class CDatabaseManager;
typedef std::shared_ptr<struct SProject> tspProject;

namespace script
{
  using tCallbackValue = std::variant<QJSValue, QtLua::Value>;

  bool CouldBeListFromLua(const QVariantMap& map);
  QVariant ConvertLuaVariant(const QVariant& value);

  bool CallCallback(QJSValue& callback, const QJSValueList& args,
                    QString* sError);
  bool CallCallback(QtLua::Value& callback, const QVariantList& args,
                    QString* sError);

  std::optional<QColor> ParseColorFromScriptVariant(const QVariant& var,
                                                    qint32 iDefaultAlpha,
                                                    const QString& sContext,
                                                    QString* sError);
  std::optional<std::vector<QColor>>
                        ParseColorsFromScriptVariant(const QVariant& var,
                                                     qint32 iDefaultAlpha,
                                                     const QString& sContext,
                                                     QString* sError);

  std::optional<QString> ParseResourceFromScriptVariant(const QVariant& var,
                                         std::shared_ptr<CDatabaseManager> spDbManager,
                                         tspProject spProject,
                                         const QString& sContext,
                                         QString* sError);
  std::optional<QStringList> ParseResourceListFromScriptVariant(const QVariant& var,
                                         std::shared_ptr<CDatabaseManager> spDbManager,
                                         tspProject spProject,
                                         const QString& sContext,
                                         QString* sError);
  std::optional<QString> ParseSceneFromScriptVariant(const QVariant& var,
                                         std::shared_ptr<CDatabaseManager> spDbManager,
                                         tspProject spProject,
                                         const QString& sContext,
                                         QString* sError);

  std::optional<QStringList> ParseStringListFromScriptVariant(const QVariant& var,
                                         const QString& sContext,
                                         QString* sError);

  std::optional<QStringList> ParseTagListFromScriptVariant(const QVariant& var,
                                                           std::shared_ptr<CDatabaseManager> spDbManager,
                                                           tspProject spProject,
                                                           const QString& sContext,
                                                           QString* sError);
}

#endif // COMMONSCRIPTHELPERS_H
