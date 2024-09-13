#ifndef COMMONSCRIPTHELPERS_H
#define COMMONSCRIPTHELPERS_H

#include <QColor>
#include <QVariant>

#include <memory>
#include <optional>

class CDatabaseManager;
typedef std::shared_ptr<struct SProject> tspProject;

namespace script
{
  bool CouldBeListFromLua(const QVariantMap& map);
  QVariant ConvertLuaVariant(const QVariant& value);

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
}

#endif // COMMONSCRIPTHELPERS_H
