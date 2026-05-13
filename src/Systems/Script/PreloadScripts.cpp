#include "PreloadScripts.h"
#include "JsScriptRunner.h"
#include "LuaScriptRunner.h"
#include "ScriptRunnerInstanceController.h"

#include "Systems/DatabaseManager.h"
#include "Systems/Database/Resource.h"

#include <map>

using tWorkerFactoryMap = std::map<QString, std::function<std::unique_ptr<CScriptRunnerInstanceWorkerBase>(const QString&,std::weak_ptr<CScriptRunnerSignalContext>)>>;

namespace
{
  tWorkerFactoryMap& GetWorkerFactory()
  {
    static tWorkerFactoryMap creatorMap =
        {
         {SScriptDefinitionData::c_sScriptTypeJs, [](const QString& sName, std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext){
            return std::make_unique<CJsScriptRunnerInstanceWorker>(sName, false, std::weak_ptr<CScriptRunnerSignalContext>{});
          }},
         {SScriptDefinitionData::c_sScriptTypeLua, [](const QString& sName, std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext){
            return std::make_unique<CLuaScriptRunnerInstanceWorker>(sName, false, std::weak_ptr<CScriptRunnerSignalContext>{});
          }},
         };
    return creatorMap;
  }
}

//----------------------------------------------------------------------------------------
//
namespace preload_scripts
{
  QStringList AvailableScriptSuffixes()
  {
    const tWorkerFactoryMap& creatorMap = GetWorkerFactory();
    QStringList out;
    for (const auto& [key, _] : creatorMap)
    {
      out << key;
    }
    return out;
  }

  //--------------------------------------------------------------------------------------
  //
  void RunPreLoadScript(const tspProject& spProject, std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext)
  {
    const tWorkerFactoryMap& creatorMap = GetWorkerFactory();

    QReadLocker l(&spProject->m_rwLock);
    if (!spProject->m_sPreLoadScript.isEmpty())
    {
      auto spRes = CDatabaseManager::FindResourceInProject(spProject, spProject->m_sPreLoadScript);
      if (nullptr != spRes)
      {
        QReadLocker resLocker(&spRes->m_rwLock);
        const QString sSuffix = spRes->m_sPath.Suffix();
        QString sPath = spRes->ResourceToAbsolutePath();
        resLocker.unlock();

        QFileInfo scriptFileInfo(sPath);
        if (scriptFileInfo.exists())
        {
          QFile scriptFile(sPath);
          if (scriptFile.open(QIODevice::ReadOnly))
          {
            QString sScript = QString::fromUtf8(scriptFile.readAll());
            auto it = creatorMap.find(sSuffix);
            if (creatorMap.end() != it)
            {
              resLocker.unlock();
              l.unlock();
              auto spRunner = it->second("Preload", wpSignalEmitterContext);
              spRunner->Init();
              spRunner->RunScript(sScript, nullptr, spRes);
              spRunner->Deinit();
            }
          }
          else
          {
            QString sError = QObject::tr("Script resource file could not be opened.");
            qWarning() << sError;
            return;
          }
        }
        else
        {
          QString sError = QObject::tr("Script resource file does not exist.");
          qWarning() << sError;
          return;
        }
      }
    }
  }
}
