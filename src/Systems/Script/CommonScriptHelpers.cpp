#include "CommonScriptHelpers.h"
#include "ScriptDbWrappers.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Project.h"
#include "Systems/Resource.h"
#include "Systems/Scene.h"

#include <QDebug>
#include <QJSValue>

namespace script
{
  bool CouldBeListFromLua(const QVariantMap& map)
  {
    // lua starts counting at 1
    qint32 iCounter = 1;
    for (auto it = map.begin(); map.end() != it; ++it)
    {
      bool bOk = false;
      if (iCounter == it.key().toInt(&bOk) && bOk)
      {
        iCounter++;
      }
      else
      {
        return false;
      }
    }
    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  QVariant ConvertLuaVariant(const QVariant& var)
  {
    QVariant varConverted;
    switch (var.type())
    {
      case QVariant::Map:
      {
        QVariantList convertedList;
        QVariantMap map = var.toMap();
        if (script::CouldBeListFromLua(map))
        {
          // we need to convert these types of maps to a list
          for (const auto& v : qAsConst(map))
          {
            convertedList << ConvertLuaVariant(v);
          }
          varConverted = convertedList;
        }
        else
        {
          QVariantMap convertedMap;
          for (auto it = map.begin(); map.end() != it; ++it)
          {
            convertedMap.insert(it.key(), ConvertLuaVariant(it.value()));
          }
          varConverted = convertedMap;
        }
      } break;
      case QVariant::List:
      {
        QVariantList convertedList;
        QVariantList list = var.toList();
        for (const auto& v : qAsConst(list))
        {
          convertedList << ConvertLuaVariant(v);
        }
        varConverted = convertedList;
      } break;
      default:
      {
        varConverted = var;
      } break;
    }
    return varConverted;
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<QColor> ParseColorValsList(const std::vector<qint32>& viColorComponents,
                                           qint32 iDefaultAlpha,
                                           const QString& sContext,
                                           QString* sError)
  {
    if (viColorComponents.size() != 4 && viColorComponents.size() != 3)
    {
      if (nullptr != sError)
      {
        *sError =
          QObject::tr("Argument error in %1(). Array of three or four numbers or string was expected.")
          .arg(sContext);
      }
      return std::nullopt;
    }
    else
    {
      if (viColorComponents.size() == 4)
      {
        QColor col(viColorComponents[0], viColorComponents[1],
            viColorComponents[2], viColorComponents[3]);
        return col;
      }
      else
      {
        // if no alpha is given, make it half transparent
        return QColor(viColorComponents[0], viColorComponents[1],
            viColorComponents[2], iDefaultAlpha);
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<QColor> ParseColorFromScriptVariant(const QVariant& var,
                                                    qint32 iDefaultAlpha,
                                                    const QString& sContext,
                                                    QString* sError)
  {
    // js arrays and objects are not converted correctly, so we handle them manually
    QJSValue valFromJS = var.value<QJSValue>();
    if (var.type() == QVariant::String || var.type() == QVariant::ByteArray)
    {
      QColor col(var.toString());
      return col;
    }
    else if (var.type() == QVariant::List)
    {
      QVariantList vsList = var.toList();
      std::vector<qint32> viColorComponents;
      const qint32 iLength = vsList.length();
      for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
      {
        viColorComponents.push_back(vsList[iIndex].toInt());
      }

      return ParseColorValsList(viColorComponents, iDefaultAlpha, sContext, sError);
    }
    // lua tables can currently only be converted to VariantMap
    else if (var.type() == QVariant::Map)
    {
      QVariantMap map = var.toMap();
      std::vector<qint32> viColorComponents;
      for (auto it = map.begin(); map.end() != it; ++it)
      {
        viColorComponents.push_back(it.value().toInt());
      }

      return ParseColorValsList(viColorComponents, iDefaultAlpha, sContext, sError);
    }
    // not currently possible, but we handle it because we can
    else if (var.type() == QVariant::Color)
    {
      QColor col(var.value<QColor>());
      return col;
    }
    else if (valFromJS.isArray())
    {
      std::vector<qint32> viColorComponents;
      const qint32 iLength = valFromJS.property("length").toInt();
      for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
      {
        viColorComponents.push_back(valFromJS.property(static_cast<quint32>(iIndex)).toInt());
      }

      return ParseColorValsList(viColorComponents, iDefaultAlpha, sContext, sError);
    }
    else
    {
      if (nullptr != sError)
      {
        *sError =
          QObject::tr("Wrong argument-type to %1(). Array of three or four numbers or string was expected.")
          .arg(sContext);
      }
      return std::nullopt;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<std::vector<QColor>>
                        ParseColorsFromScriptVariant(const QVariant& var,
                                                     qint32 iDefaultAlpha,
                                                     const QString& sContext,
                                                     QString* sError)
  {
    std::vector<QColor> volors;

    // js arrays and objects are not converted correctly, so we handle them manually
    QJSValue valFromJS = var.value<QJSValue>();
    if (var.type() == QVariant::List)
    {
      QVariantList vsList = var.toList();
      for (QVariant vCol : qAsConst(vsList))
      {
        std::optional<QColor> optCol =
          ParseColorFromScriptVariant(vCol, iDefaultAlpha, sContext, sError);
        if (optCol.has_value())
        {
          volors.push_back(optCol.value());
        }
        else
        {
          return std::nullopt;
        }
      }
      return volors;
    }
    // lua tables can currently only be converted to VariantMap
    else if (var.type() == QVariant::Map)
    {
      QVariantMap map = var.toMap();
      for (QVariant vCol : qAsConst(map))
      {
        std::optional<QColor> optCol =
          ParseColorFromScriptVariant(vCol, iDefaultAlpha, sContext, sError);
        if (optCol.has_value())
        {
          volors.push_back(optCol.value());
        }
        else
        {
          return std::nullopt;
        }
      }
      return volors;
    }
    else if (valFromJS.isArray())
    {
      QVariantList viColors;
      const qint32 iLength = valFromJS.property("length").toInt();
      for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
      {
        std::optional<QColor> optCol =
          ParseColorFromScriptVariant(
              valFromJS.property(static_cast<quint32>(iIndex)).toVariant(),
              iDefaultAlpha, sContext, sError);
        if (optCol.has_value())
        {
          volors.push_back(optCol.value());
        }
        else
        {
          return std::nullopt;
        }
      }
      return volors;
    }
    else
    {
      if (nullptr != sError)
      {
        *sError =
          QObject::tr("Wrong argument-type to %1(). Array of arrays of three or four numbers or array of strings was expected.")
          .arg(sContext);
      }
      return std::nullopt;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  namespace
  {
    template<typename T> struct SWraperToType : public std::false_type {};
    template<> struct SWraperToType<SResource> { using type = CResourceScriptWrapper;
                                                 static constexpr char c_sName[] = "Resource"; };
    template<> struct SWraperToType<SScene> { using type = CSceneScriptWrapper;
                                              static constexpr char c_sName[] = "Scene";};

    template<typename T> std::shared_ptr<T> FindInProject(
        std::shared_ptr<CDatabaseManager>,
        tspProject, const QString&)
    { return nullptr; }
    template<> std::shared_ptr<SResource> FindInProject<SResource>(
        std::shared_ptr<CDatabaseManager> spDbManager,
        tspProject spProject, const QString& sName)
    { return spDbManager->FindResourceInProject(spProject, sName); }
    template<> std::shared_ptr<SScene> FindInProject<SScene>(
        std::shared_ptr<CDatabaseManager> spDbManager,
        tspProject spProject, const QString& sName)
    { return spDbManager->FindScene(spProject, sName); }
  }

  //--------------------------------------------------------------------------------------
  //
  template<typename T, typename U = typename SWraperToType<T>::type>
  std::optional<QString> ParseItemFromScriptVariant(const QVariant& var,
                                         std::shared_ptr<CDatabaseManager> spDbManager,
                                         tspProject spProject,
                                         const QString& sContext,
                                         QString* sError)
  {
    using tspItem = std::shared_ptr<T>;
    if (nullptr != spDbManager)
    {
      if (var.type() == QVariant::String || var.type() == QVariant::ByteArray)
      {
        QString sFoundItem = var.toString();
        if (sFoundItem.isEmpty())
        {
          return sFoundItem;
        }
        else
        {
          tspItem spItem = FindInProject<T>(spDbManager, spProject, sFoundItem);
          if (nullptr != spItem)
          {
            return sFoundItem;
          }
          else
          {
            if (nullptr != sError)
            {
              *sError = QObject::tr("%2 %1 not found.").arg(sFoundItem)
                  .arg(SWraperToType<T>::c_sName);
            }
            return std::nullopt;
          }
        }
      }
      else if (var.isNull())
      {
        return QString();
      }
      else
      {
        U* pItemWrapper = dynamic_cast<U*>(var.value<QObject*>());
        if (nullptr != pItemWrapper)
        {
          tspItem spItem = pItemWrapper->Data();
          if (nullptr != spItem)
          {
            QString sName = pItemWrapper->getName();
            return sName;
          }
          else
          {
            if (nullptr != sError)
            {
              *sError =
                QObject::tr("%2 in %1() holds no data.").arg(sContext)
                  .arg(SWraperToType<T>::c_sName);
            }
            return std::nullopt;
          }
        }
        else
        {
          if (nullptr != sError)
          {
            *sError =
                QObject::tr("Wrong argument-type to %1(). String, %2 or null was expected.")
                .arg(sContext)
                .arg(SWraperToType<T>::c_sName);
          }
          return std::nullopt;
        }
      }
    }
    else
    {
      qCritical() << QObject::tr("CDatabaseManager was null in %1()").arg(sContext);
      return std::nullopt;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<QString> ParseResourceFromScriptVariant(const QVariant& var,
                                         std::shared_ptr<CDatabaseManager> spDbManager,
                                         tspProject spProject,
                                         const QString& sContext,
                                         QString* sError)
  {
    return ParseItemFromScriptVariant<SResource>(
          var, spDbManager, spProject, sContext, sError);
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<QStringList> ParseResourceListFromScriptVariant(const QVariant& var,
                                         std::shared_ptr<CDatabaseManager> spDbManager,
                                         tspProject spProject,
                                         const QString& sContext,
                                         QString* sError)
  {
    QStringList vsOutValues;
    QJSValue valFromJS = var.value<QJSValue>();
    if (var.type() == QVariant::List)
    {
      QVariantList varList = var.toList();
      for (qint32 iIndex = 0; varList.size() > iIndex; iIndex++)
      {
        auto optRes = ParseItemFromScriptVariant<SResource>(
              varList[iIndex], spDbManager, spProject, sContext, sError);
        if (optRes.has_value())
        {
          vsOutValues.push_back(*optRes);
        }
        else
        {
          if (nullptr != sError)
          {
            *sError =
              QObject::tr("Wrong argument-type to %1(). Array of resources was expected.")
              .arg(sContext);
          }
          return std::nullopt;
        }
      }
    }
    // lua tables can currently only be converted to VariantMap
    else if (var.type() == QVariant::Map)
    {
      QVariantMap varList = var.toMap();
      for (auto it = varList.begin(); varList.end() != it; ++it)
      {
        auto optRes = ParseItemFromScriptVariant<SResource>(
              it.value(), spDbManager, spProject, sContext, sError);
        if (optRes.has_value())
        {
          vsOutValues.push_back(*optRes);
        }
        else
        {
          if (nullptr != sError)
          {
            *sError =
              QObject::tr("Wrong argument-type to %1(). Array of resources was expected.")
              .arg(sContext);
          }
          return std::nullopt;
        }
      }
    }
    else if (valFromJS.isArray())
    {
      const qint32 iLength = valFromJS.property("length").toInt();
      for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
      {
        QJSValue val = valFromJS.property(static_cast<quint32>(iIndex));
        auto optRes = ParseItemFromScriptVariant<SResource>(
              val.toVariant(), spDbManager, spProject, sContext, sError);
        if (optRes.has_value())
        {
          vsOutValues.push_back(*optRes);
        }
        else
        {
          if (nullptr != sError)
          {
            *sError =
              QObject::tr("Wrong argument-type to %1(). Array of resources was expected.")
              .arg(sContext);
          }
          return std::nullopt;
        }
      }
    }
    else
    {
      if (nullptr != sError)
      {
        *sError =
          QObject::tr("Wrong argument-type to %1(). Array of resources was expected.")
          .arg(sContext);
      }
      return std::nullopt;
    }
    return vsOutValues;
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<QString> ParseSceneFromScriptVariant(const QVariant& var,
                                         std::shared_ptr<CDatabaseManager> spDbManager,
                                         tspProject spProject,
                                         const QString& sContext,
                                         QString* sError)
  {
    return ParseItemFromScriptVariant<SScene>(
          var, spDbManager, spProject, sContext, sError);
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<QStringList> ParseStringListFromScriptVariant(const QVariant& var,
                                         const QString& sContext,
                                         QString* sError)
  {
    QStringList vsList;
    // js arrays and objects are not converted correctly, so we handle them manually
    QJSValue valFromJS = var.value<QJSValue>();
    if (var.type() == QVariant::List)
    {
      QVariantList vVarList = var.toList();
      for (QVariant v : qAsConst(vVarList))
      {
        vsList << v.toString();
      }
      return vsList;
    }
    // lua tables can currently only be converted to VariantMap
    else if (var.type() == QVariant::Map)
    {
      QVariantMap map = var.toMap();
      for (QVariant v : qAsConst(map))
      {
        vsList << v.toString();
      }
      return vsList;
    }
    else if (valFromJS.isArray())
    {
      const qint32 iLength = valFromJS.property("length").toInt();
      for (qint32 iIndex = 0; iLength > iIndex; iIndex++)
      {
        vsList << valFromJS.property(static_cast<quint32>(iIndex)).toString();
      }
      return vsList;
    }
    else
    {
      if (nullptr != sError)
      {
        *sError =
          QObject::tr("Wrong argument-type to %1(). Array of strings was expected.")
          .arg(sContext);
      }
      return std::nullopt;
    }
  }
}
