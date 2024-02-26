#include "LuaScriptRunner.h"
#include "Application.h"
#include "ScriptDbWrappers.h"
#include "ScriptNotification.h"
#include "ScriptTextBox.h"
#include "ScriptThread.h"
#include "ScriptRunnerInstanceController.h"
#include "ScriptRunnerSignalEmiter.h"

#include "Systems/DatabaseManager.h"

extern "C"
{
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}

#include <QtLua/State>
#include <QDebug>
#include <QFile>

#include <set>

char CScriptRunnerUtilsLua::sKeyThis;

//----------------------------------------------------------------------------------------
//
namespace
{
  //--------------------------------------------------------------------------------------
  //
  void RegisterEnum(QtLua::State* pState, const QMetaObject& metaType, const QString& sName,
                    std::set<QString>* pvGlobalMap)
  {
    QtLua::Value luavalue(QtLua::Value::new_table(pState));
    QMetaEnum enumWrapper = metaType.enumerator(0);
    for (qint32 i = 0; enumWrapper.keyCount() > i; ++i)
    {
      luavalue[enumWrapper.key(i)] = enumWrapper.value(i);

      if (nullptr != pvGlobalMap)
      {
        pvGlobalMap->insert(QString("%1.%2").arg(sName).arg(enumWrapper.key(i)));
      }
    }
    (*pState)[sName] = luavalue;
  }

  //--------------------------------------------------------------------------------------
  //
  QStringList GetGlobalValues(const QStringList& vsPath, const QVariantMap& variables)
  {
    QStringList vsRet;
    for (auto it = variables.begin(); variables.end() != it; ++it)
    {
      if (it.value().type() == QVariant::String)
      {
        const QString sValue = it.value().toString();
        vsRet.push_back(sValue + " = " + vsPath.join(".") +
                        (vsPath.isEmpty() ? "" : ".") + sValue);
      }
      else if (it.value().type() == QVariant::StringList)
      {
        QStringList vsChildrenRet;
        for (const QString& sChild : it.value().toStringList())
        {
          QVariantMap map;
          map[sChild] = sChild;
          QStringList vsReflRet = GetGlobalValues(QStringList() << vsPath << it.key(),
                                                  map);
          vsChildrenRet << vsReflRet;
        }
        vsRet.push_back(QString("%1 = { %2 }").arg(it.key())
                        .arg(vsChildrenRet.join(",")));
      }
      else
      {
        QVariantMap map = it.value().toMap();
        QStringList vsReflRet = GetGlobalValues(QStringList() << vsPath << it.key(), map);
        vsRet.push_back(QString("%1 = { %2 }").arg(it.key())
                        .arg(vsReflRet.join(",")));
      }
    }
    return vsRet;
  }
}

//--------------------------------------------------------------------------------------
//
namespace lua
{
  void LUAHookAbort(lua_State* pL, lua_Debug* pAr);
  int LUACustomSearcher(lua_State* pL);
  int LUAPrint(lua_State* pL);

  static const struct luaL_Reg printlib [] = {
    {"print", LUAPrint},
    {NULL, NULL} /* end of array */
  };

  void LuaopenLUAPrintlib(lua_State *L)
  {
    lua_getglobal(L, "_G");
    // luaL_register(L, NULL, printlib); // for Lua versions < 5.2
    luaL_setfuncs(L, printlib, 0);  // for Lua versions 5.2 or greater
    lua_pop(L, 1);
  }
}

//----------------------------------------------------------------------------------------
//
class CLuaScriptRunnerInstanceWorker : public CScriptRunnerInstanceWorkerBase
{
  Q_OBJECT
  friend int lua::LUACustomSearcher(lua_State* pL);

public:
  CLuaScriptRunnerInstanceWorker(const QString& sName,
                                std::weak_ptr<CScriptRunnerSignalContext> wpSignalEmitterContext) :
    CScriptRunnerInstanceWorkerBase(sName, wpSignalEmitterContext),
    m_pLuaState(nullptr),
    m_pScriptUtils(nullptr),
    m_pCurrentScene(nullptr),
    m_bLoadingInbuiltLibraries(false)
  {}
  ~CLuaScriptRunnerInstanceWorker()
  {
  }

public slots:
  //----------------------------------------------------------------------------------------
  //
  void HandleError(QtLua::String& sError)
  {
    m_bRunning = 0;

    auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
    if (nullptr == spSignalEmitterContext)
    {
      qCritical() << "SignalEmitter is null";
      return;
    }

    qint32 iLineNr = 0;

    QRegExp rxLine(":[0-9]+:");
    qint32 iPos = -1;
    if ((iPos = rxLine.lastIndexIn(sError, iPos)) != -1)
    {
      bool bOk = false;
      iLineNr = sError.mid(iPos+1, rxLine.matchedLength()-2).toInt(&bOk) - 2;
      if (!bOk)
      {
        iLineNr = 0;
      }
    }

    QString sStack;
    QString sErrorFormated = "Uncaught Lua exception: " + sError;
    qCritical() << sErrorFormated;

    emit spSignalEmitterContext->showError(sErrorFormated, QtMsgType::QtCriticalMsg);
    emit spSignalEmitterContext->executionError(sError, iLineNr, sStack);
  }

  //--------------------------------------------------------------------------------------
  //
  void Init() override
  {
    m_pLuaState = new QtLua::State();
    m_pLuaState->enable_qdebug_print(true);

    auto pInternalState = m_pLuaState->get_lua_state();
    RegisterAbortHook(pInternalState);

    bool bOk = true;
    bOk &= m_pLuaState->openlib(QtLua::BaseLib);
    bOk &= m_pLuaState->openlib(QtLua::CoroutineLib);
    bOk &= m_pLuaState->openlib(QtLua::PackageLib);
    bOk &= m_pLuaState->openlib(QtLua::StringLib);
    bOk &= m_pLuaState->openlib(QtLua::TableLib);
    bOk &= m_pLuaState->openlib(QtLua::MathLib);
    bOk &= m_pLuaState->openlib(QtLua::IoLib);
    bOk &= m_pLuaState->openlib(QtLua::OsLib);
    bOk &= m_pLuaState->openlib(QtLua::DebugLib);
    //bOk &= m_pLuaState->openlib(QtLua::Bit32Lib);
    //bOk &= m_pLuaState->openlib(QtLua::JitLib);
    //bOk &= m_pLuaState->openlib(QtLua::FfiLib);
    //bOk &= m_pLuaState->openlib(QtLua::QtLuaLib);
    //bOk &= m_pLuaState->openlib(QtLua::QtLib);

    RegisterCustomPackageSearcher(pInternalState);
    lua::LuaopenLUAPrintlib(pInternalState);

    m_bLoadingInbuiltLibraries = true;
    bOk &= OpenFileLibrary(m_pLuaState, "sandbox", ":/lua_sandbox/sandbox.lua");
    bOk &= OpenFileLibrary(m_pLuaState, "json", ":/lua_json/json.lua");
    m_bLoadingInbuiltLibraries = false;

    if (!bOk)
    {
      qWarning() << "Could not load an inbuilt Lua library.";
    }

    m_pScriptUtils = new CScriptRunnerUtilsLua(this, m_pLuaState,
                                               m_wpSignalEmiterContext.lock());
    connect(m_pScriptUtils, &CScriptRunnerUtilsLua::finishedScriptSignal,
            this, &CLuaScriptRunnerInstanceWorker::FinishedScript);

    (*m_pLuaState)["utils_1337"] = QtLua::Value(m_pLuaState, m_pScriptUtils.data(), false, false);

    RegisterEnum(m_pLuaState, IconAlignment::staticMetaObject, "IconAlignment", &m_vGlobalValues);
    RegisterEnum(m_pLuaState, TextAlignment::staticMetaObject, "TextAlignment", &m_vGlobalValues);
  }

  //--------------------------------------------------------------------------------------
  //
  void Deinit() override
  {
    InterruptExecution();

    ResetEngine();

    if (nullptr != m_pLuaState)
    {
      m_pLuaState->gc_collect();
    }

    delete m_pScriptUtils;
    m_pScriptUtils = nullptr;

    if (nullptr != m_pLuaState)
    {
      m_pLuaState->deleteLater();
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void InterruptExecution() override
  {
    m_bRunning = 0;
    if (nullptr != m_pLuaState)
    {
      emit SignalInterruptExecution();
      SetInterrupted(true);
    }
  }

  //----------------------------------------------------------------------------------------
  //
  void RunScript(const QString& sScript,
                 tspScene spScene, tspResource spResource) override
  {
    // set scene
    if (nullptr != m_pCurrentScene)
    {
      delete m_pCurrentScene;
    }
    if (nullptr != spScene)
    {
      m_pCurrentScene = new CSceneScriptWrapper(m_pLuaState, spScene);
      (*m_pLuaState)["scene"] = QtLua::Value(m_pLuaState, m_pCurrentScene.data(), false, false);
    }

    // set current Project
    {
      QReadLocker scriptLocker(&spResource->m_rwLock);
      m_spProject = spResource->m_spParent;
      for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
      {
        it->second->SetCurrentProject(spResource->m_spParent);
      }
      m_pScriptUtils->SetCurrentProject(spResource->m_spParent);
    }

    // resume engine if interrupetd
    SetInterrupted(false);
    m_bRunning = 1;

    auto spSignalEmitterContext = m_wpSignalEmiterContext.lock();
    if (nullptr == spSignalEmitterContext)
    {
      QtLua::String sDummy;
      HandleError(sDummy);
      return;
    }

    spSignalEmitterContext->SetScriptExecutionStatus(CScriptRunnerSignalEmiter::eRunning);

    // get scene name and set it as function, so error messages show the scene name in the error

    QString sSceneName = QString();
    {
      QReadLocker locker(&spResource->m_rwLock);
      sSceneName = QString(spResource->m_sName).replace(QRegExp("\\W"), "_");
    }
    if (nullptr != spScene)
    {
      QReadLocker locker(&spScene->m_rwLock);
      sSceneName = QString(spScene->m_sName).replace(QRegExp("\\W"), "_");
    }

    // create wrapper function to make syntax of scripts easier and handle return value
    // and be able to emit signal on finished
    QString sScriptEscaped = sScript;
    sScriptEscaped.replace("[[", "\\[\\[").replace("]]", "\\]\\]");
    QString sSkript = QString("local ret = sandbox.run([[local %1 = function();\n%2\nreturn nil;\nend;\nreturn %3();]], %4);"
                              "utils_1337:finishedScript(ret);")
        .arg(sSceneName)
        .arg(sScriptEscaped)
        .arg(sSceneName)
        .arg(GenerateEnvVariableString());

    try
    {
      if (!sSceneName.isEmpty())
      {
        emit SignalSceneLoaded(sSceneName);
      }

      m_pLuaState->exec_statements(sSkript);
      if (IsInterrupted())
      {
        m_bRunning = 0;
        emit HandleScriptFinish(false, QString());
      }
      m_pLuaState->gc_collect();
    }
    catch (QtLua::String& s)
    {
      if (!IsInterrupted())
      {
        m_bRunning = 0;
        HandleError(s);
        m_pLuaState->gc_collect();
      }
      else
      {
        m_bRunning = 0;
        emit HandleScriptFinish(false, QString());
      }
      m_pLuaState->gc_collect();
      return;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterNewComponent(const QString sName, CScriptRunnerSignalEmiter* pObject) override
  {
    auto it = m_objectMap.find(sName);
    if (m_objectMap.end() == it)
    {
      if (nullptr != pObject)
      {
        pObject->Initialize(m_wpSignalEmiterContext.lock());
        std::shared_ptr<CScriptObjectBase> spObject =
            pObject->CreateNewScriptObject(m_pLuaState);
        if (nullptr != spObject)
        {
          connect(this, &CLuaScriptRunnerInstanceWorker::SignalInterruptExecution,
                  spObject.get(), &CScriptObjectBase::SignalInterruptExecution, Qt::QueuedConnection);

          if (spObject->thread() != thread())
          {
            spObject->moveToThread(thread());
          }
          m_objectMap.insert({ sName, spObject });

          if (auto pNotification = dynamic_cast<CScriptNotification*>(spObject.get()))
          {
            connect(pNotification, &CScriptNotification::SignalOverlayCleared,
                    this, [this](){
              emit SignalClearThreads(EScriptRunnerType::eOverlay);
            });
            connect(pNotification, &CScriptNotification::SignalOverlayClosed,
                    this, &CLuaScriptRunnerInstanceWorker::SignalKill);
            connect(pNotification, &CScriptNotification::SignalOverlayRunAsync,
                    this, [this](const QString& sId, const QString& sScriptResource){
              emit SignalRunAsync(m_spProject, sId, sScriptResource, EScriptRunnerType::eOverlay);
            });
          }
          else if (auto pThread = dynamic_cast<CScriptThread*>(spObject.get()))
          {
            connect(pThread, &CScriptThread::SignalKill,
                    this, &CLuaScriptRunnerInstanceWorker::SignalKill);
            connect(pThread, &CScriptThread::SignalOverlayRunAsync,
                    this, [this](const QString& sId, const QString& sScriptResource){
              emit SignalRunAsync(m_spProject, sId, sScriptResource,
                                  EScriptRunnerType::eAsync);
            });
          }

          (*m_pLuaState)[sName] = QtLua::Value(m_pLuaState, spObject.get(), false, false);
        }
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void ResetEngine() override
  {
    m_pScriptUtils->SetCurrentProject(nullptr);

    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      it->second->Cleanup();
      it->second->SetCurrentProject(nullptr);
    }

    m_pLuaState->gc_collect();

    if (nullptr != m_pCurrentScene)
    {
      delete m_pCurrentScene;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  bool IsInterrupted() const
  {
    return m_bInterrupted == 1;
  }

  //--------------------------------------------------------------------------------------
  //
  tspProject CurrentProject() const
  {
    return m_spProject;
  }

private:
  //--------------------------------------------------------------------------------------
  //
  QString GenerateEnvVariableString()
  {
    QString sEnvVar = "{ env = { json = json, scene = scene, utils_1337 = utils_1337, sandbox = { run = sandbox.run } %1 } }";
    QStringList vsBindings;
    for (auto it = m_objectMap.begin(); m_objectMap.end() != it; ++it)
    {
      vsBindings << QString("%1 = %2").arg(it->first).arg(it->first);
    }
    QVariantMap globalMap;
    for (const QString& sGlobal : m_vGlobalValues)
    {
      QStringList vsGlobalElems = sGlobal.split(".");
      std::function<void(const QStringList&, QVariantMap&)> fnBla =
          [&fnBla](const QStringList& vsGlobalElems, QVariantMap& insertInto) {
        if (vsGlobalElems.size() == 0)
        {
          return;
        }
        if (vsGlobalElems.size() == 1)
        {
          const QString sKey = vsGlobalElems.front();
          insertInto[sKey] = sKey;
        }
        else
        {
          const QString sKey = vsGlobalElems.front();
          QStringList vsElemsMinOne = vsGlobalElems.mid(1);
          if (vsElemsMinOne.size() == 1)
          {
            QVariant var = insertInto[sKey];
            QStringList vsElems = var.toStringList() << vsElemsMinOne[0];
            insertInto[sKey] = vsElems;
          }
          else
          {
            QVariantMap childMap;
            fnBla(vsElemsMinOne, childMap);
            insertInto[sKey] = childMap;
          }
        }
      };
      fnBla(vsGlobalElems, globalMap);
    }
    QStringList vsGlobalsBindings = GetGlobalValues(QStringList(), globalMap);
    if (!vsGlobalsBindings.isEmpty())
    {
      vsBindings << vsGlobalsBindings;
    }
    return sEnvVar.arg(vsBindings.empty() ? "" : QString(",") + vsBindings.join(","));
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterAbortHook(lua_State* pState)
  {
    // pointer to this
    lua_pushlightuserdata(pState, &CScriptRunnerUtilsLua::sKeyThis);
    lua_pushlightuserdata(pState, this);
    lua_rawset(pState, LUA_REGISTRYINDEX);
    // register abort hook
    lua_sethook(pState, lua::LUAHookAbort, LUA_MASKLINE, 0);
  }

  //--------------------------------------------------------------------------------------
  //
  bool RegisterCustomPackageSearcher(lua_State* pState)
  {
    constexpr qint32 c_iNbrSearchers = 5;

    // hack: override all searchers with custom searcher, to remove possibility to load
    // from file directly
    for (qint32 i = 1; i < c_iNbrSearchers+1; ++i)
    {
      lua_getglobal(pState, LUA_LOADLIBNAME);
      if (!lua_istable(pState, -1))
      {
        return false;
      }

      lua_getfield(pState, -1, "searchers");
      if (!lua_istable(pState, -1))
      {
        return false;
      }

      lua_pushvalue(pState, -2);

      lua_pushcclosure(pState, lua::LUACustomSearcher, 1);
      lua_rawseti(pState, -2, i);

      lua_setfield(pState, -2, "searchers");
    }

    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  bool OpenFileLibrary(QtLua::State* pState, const QString& sGlobal, const QString& sFile)
  {
    try
    {
      pState->exec_statements(QString("%1 = require '%2';").arg(sGlobal).arg(sFile));
    }
    catch (QtLua::String& s)
    {
      qWarning() << tr("Could not load %1 library:").arg(sGlobal) << s;
      return false;
    }
    return true;
  }

  //--------------------------------------------------------------------------------------
  //
  void SetInterrupted(bool bInterrupted)
  {
    m_bInterrupted = bInterrupted ? 1 : 0;
  }

  tspProject                                     m_spProject;
  QPointer<QtLua::State>                         m_pLuaState;
  QPointer<CScriptRunnerUtilsLua>                m_pScriptUtils;
  QPointer<CSceneScriptWrapper>                  m_pCurrentScene;
  std::map<QString /*name*/,
           std::shared_ptr<CScriptObjectBase>>   m_objectMap;
  std::set<QString>                              m_vGlobalValues;
  QString                                        m_sName;
  QAtomicInt                                     m_bInterrupted;

protected:
  bool                                           m_bLoadingInbuiltLibraries;
};

//----------------------------------------------------------------------------------------
//
namespace lua
{
  void LUAHookAbort(lua_State* pL, lua_Debug* pAr)
  {
    if (LUA_HOOKLINE == pAr->event)
    {
      // get this CLuaScriptRunnerInstanceWorker from registry
      void *data;
      lua_pushlightuserdata(pL, &CScriptRunnerUtilsLua::sKeyThis);
      lua_rawget(pL, LUA_REGISTRYINDEX);
      data = lua_touserdata(pL, -1);
      lua_pop(pL, 1);
      CLuaScriptRunnerInstanceWorker* pContext =
          static_cast<CLuaScriptRunnerInstanceWorker*>(data);

      // Check the interrupt flag to know if we should abort
      if(pContext->IsInterrupted())
      {
        // Ok, let's abort the script
        lua_pushstring(pL, "HookRoutine: Abort requested!");
        lua_error(pL);
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  int LUACustomSearcher(lua_State* pL)
  {
    const char* sName = luaL_checkstring(pL, 1);
    QString s = QString(sName);
    QString sNameSanitized(s);
    QString sError;

    // get this CLuaScriptRunnerInstanceWorker from registry
    void *data;
    lua_pushlightuserdata(pL, &CScriptRunnerUtilsLua::sKeyThis);
    lua_rawget(pL, LUA_REGISTRYINDEX);
    data = lua_touserdata(pL, -1);
    lua_pop(pL, 1);
    CLuaScriptRunnerInstanceWorker* pContext =
        static_cast<CLuaScriptRunnerInstanceWorker*>(data);

    bool bInbuiltLoad = sNameSanitized.startsWith(":");
    bool bResourcesAllowed = pContext->m_bLoadingInbuiltLibraries;

    auto spDbManager = CApplication::Instance()->System<CDatabaseManager>().lock();
    tspProject spProj = pContext->CurrentProject();
    if ((nullptr != spProj && nullptr != spDbManager) ||
        (bResourcesAllowed && bInbuiltLoad))
    {
      tspResource spResource = spDbManager->FindResourceInProject(spProj, sNameSanitized);
      if (nullptr != spResource || (bResourcesAllowed && bInbuiltLoad))
      {
        EResourceType type = EResourceType::eImage;
        QString sFoundResourceName;
        QFile file;

        if (nullptr != spResource)
        {
          QReadLocker locker(&spResource->m_rwLock);
          type = spResource->m_type;
          sFoundResourceName = spResource->m_sName;
          locker.unlock();
          file.setFileName(PhysicalResourcePath(spResource));
        }
        else if (bResourcesAllowed && bInbuiltLoad)
        {
          type = EResourceType::eScript;
          sFoundResourceName = QFileInfo(sNameSanitized).fileName();
          file.setFileName(sNameSanitized);
        }

        if (EResourceType::eScript == type._to_integral() ||
            (bResourcesAllowed && bInbuiltLoad))
        {
          if (file.exists() &&
              file.open(QIODevice::ReadOnly))
          {
            QByteArray arr = file.readAll();

            // sandbox user modules as well
            if (!bInbuiltLoad)
            {
              // create wrapper function to make syntax of scripts easier and handle return value
              // and be able to emit signal on finished
              QString sScriptEscaped = QString::fromUtf8(arr);
              sScriptEscaped.replace("[[", "\\[\\[").replace("]]", "\\]\\]");
              QString sSkript = QString("return sandbox.run([[%1]], %2);")
                                    .arg(sScriptEscaped)
                                    .arg(pContext->GenerateEnvVariableString());
              arr = sSkript.toUtf8();
            }

            int iStatus = luaL_loadbuffer(pL, arr.data(), arr.length(),
                                          sFoundResourceName.toStdString().c_str());
            if (iStatus == LUA_OK)
            {
              qint32 iDotDelimiter = sFoundResourceName.indexOf(".");
              iDotDelimiter = -1 == iDotDelimiter ? sFoundResourceName.size() : iDotDelimiter;
              QString sModuleName =
                  sFoundResourceName.left(iDotDelimiter);
              sModuleName.replace(QRegExp("[^a-zA-Z0-9\\-\\_]"), "_");
              lua_pushstring(pL, sModuleName.toStdString().c_str());  /* will be 2nd argument to module */
              return 2;  /* return open function and file name */
            }
            else
            {
              sError = "Could not load module file.";
            }
          }
          else
          {
            sError = "Could not open module file.";
          }
        }
        else
        {
          sError = "Resource is not of 'Script' type.";
        }
      }
      else
      {
        sError = "Resource not found.";
      }
    }
    else
    {
      sError = "Project or Database manager not found.";
    }
    return luaL_error(pL, "Error loading module '%s': %s",
                          sName, sError.toStdString().data());
  }

  //--------------------------------------------------------------------------------------
  //
  int LUAPrint(lua_State* pL)
  {
    int iArgs = lua_gettop(pL);
    for (int i = 1; i <= iArgs; i++)
    {
      size_t l;
      if (lua_isstring(pL, i))
      {
        const char *s = luaL_tolstring(pL, i, &l);  /* convert it to string */
        qDebug() << "Lua console: " << s;
      }
      lua_pop(pL, 1);  /* pop result */
    }
    return 0;
  }
}

//----------------------------------------------------------------------------------------
//
CLuaScriptRunner::CLuaScriptRunner(std::weak_ptr<CScriptRunnerSignalContext> spSignalEmitterContext,
                                   tRunningScriptsCheck fnRunningScriptsCheck,
                                   QObject* pParent) :
  QObject(pParent),
  IScriptRunnerFactory(),
  m_wpSignalEmitterContext(spSignalEmitterContext),
  m_signalEmiterMutex(QMutex::Recursive),
  m_pSignalEmiters(),
  m_fnRunningScriptsCheck(fnRunningScriptsCheck)
{
}
CLuaScriptRunner::~CLuaScriptRunner()
{
}

//----------------------------------------------------------------------------------------
//
void CLuaScriptRunner::Initialize()
{
}

//----------------------------------------------------------------------------------------
//
void CLuaScriptRunner::Deinitialize()
{
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController>
CLuaScriptRunner::LoadScript(const QString& sScript,
                             tspScene spScene, tspResource spResource)
{
  std::shared_ptr<CScriptRunnerInstanceController> spRunner = CreateRunner(c_sMainRunner);
  RunScript(spRunner, sScript, spScene, spResource);
  return spRunner;
}

//----------------------------------------------------------------------------------------
//
void CLuaScriptRunner::RegisterNewComponent(const QString sName, QJSValue signalEmitter)
{
  CScriptRunnerSignalEmiter* pObject = nullptr;
  if (signalEmitter.isObject())
  {
    pObject = qobject_cast<CScriptRunnerSignalEmiter*>(signalEmitter.toQObject());
  }

  QMutexLocker lockerEmiter(&m_signalEmiterMutex);
  m_pSignalEmiters.insert({sName, pObject});
}

//----------------------------------------------------------------------------------------
//
void CLuaScriptRunner::UnregisterComponents()
{
  QMutexLocker lockerEmiter(&m_signalEmiterMutex);
  m_pSignalEmiters.clear();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IScriptRunnerInstanceController>
CLuaScriptRunner::RunAsync(const QString& sId,
                           const QString& sScript, tspResource spResource)
{
  std::shared_ptr<CScriptRunnerInstanceController> spRunner = CreateRunner(sId);

  auto spSignalEmitterContext = m_wpSignalEmitterContext.lock();
  if (nullptr == spSignalEmitterContext)
  {
    qWarning() << "m_wpSignalEmitterContext was null";
    return nullptr;
  }

  if (nullptr == spResource ||
      nullptr == spResource->m_spParent)
  {
    QString sError = tr("Script file, Scene or Project is null");
    qCritical() << sError;
    emit spSignalEmitterContext->showError(sError, QtMsgType::QtCriticalMsg);
    return nullptr;
  }

  RunScript(spRunner, sScript, nullptr, spResource);
  return spRunner;
}

//----------------------------------------------------------------------------------------
//
void CLuaScriptRunner::SlotHandleScriptFinish(const QString& sName, bool bSuccess,
                                              const QVariant& sRetVal)
{
  emit SignalRemoveScriptRunner(sName);

  if (!bSuccess)
  {
    qWarning() << tr("Error in script, unloading project.");
  }

  if (c_sMainRunner == sName || QVariant::String == sRetVal.type() || !m_fnRunningScriptsCheck())
  {
    if (QVariant::String == sRetVal.type())
    {
      emit SignalScriptRunFinished(bSuccess, sRetVal.toString());
    }
    else
    {
      emit SignalScriptRunFinished(bSuccess, QString());
    }
  }
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CScriptRunnerInstanceController> CLuaScriptRunner::CreateRunner(const QString& sId)
{
  std::shared_ptr<CScriptRunnerInstanceController> spRunner =
      std::make_shared<CScriptRunnerInstanceController>(
                           sId,
                           std::make_shared<CLuaScriptRunnerInstanceWorker>(
                             sId, m_wpSignalEmitterContext),
                           m_wpSignalEmitterContext);
  spRunner->setObjectName(sId);
  connect(spRunner.get(), &CScriptRunnerInstanceController::HandleScriptFinish,
          this, &CLuaScriptRunner::SlotHandleScriptFinish);
  connect(spRunner.get(), &CScriptRunnerInstanceController::SignalSceneLoaded,
          this, &CLuaScriptRunner::SignalSceneLoaded);

  connect(spRunner.get(), &CScriptRunnerInstanceController::SignalClearThreads,
          this, &CLuaScriptRunner::SignalClearThreads);
  connect(spRunner.get(), &CScriptRunnerInstanceController::SignalKill,
          this, &CLuaScriptRunner::SignalKill);
  connect(spRunner.get(), &CScriptRunnerInstanceController::SignalRunAsync,
          this, &CLuaScriptRunner::SignalRunAsync);

  QMutexLocker lockerEmiter(&m_signalEmiterMutex);
  for (auto& itEmiter : m_pSignalEmiters)
  {
    spRunner->RegisterNewComponent(itEmiter.first, itEmiter.second);
  }
  return spRunner;
}

//----------------------------------------------------------------------------------------
//
void CLuaScriptRunner::RunScript(std::shared_ptr<CScriptRunnerInstanceController> spRunner,
                                 const QString& sScript,
                                 tspScene spScene, tspResource spResource)
{
  spRunner->RunScript(sScript, spScene, spResource);
}

//----------------------------------------------------------------------------------------
//
CScriptRunnerUtilsLua::CScriptRunnerUtilsLua(QObject* pParent,
                                             QtLua::State* pState,
                                             std::shared_ptr<CScriptRunnerSignalContext> spSignalEmiterContext) :
  QObject(pParent),
  m_spSignalEmiterContext(spSignalEmiterContext),
  m_pState(pState)
{

}
CScriptRunnerUtilsLua::~CScriptRunnerUtilsLua()
{

}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerUtilsLua::SetCurrentProject(tspProject spProject)
{
  m_spProject = spProject;
}

//----------------------------------------------------------------------------------------
//
void CScriptRunnerUtilsLua::finishedScript(const QVariant& retVal)
{
  emit finishedScriptSignal(retVal);
}

#include "LuaScriptRunner.moc"
