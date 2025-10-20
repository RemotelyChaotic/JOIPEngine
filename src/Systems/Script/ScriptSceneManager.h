#ifndef SCRIPTSCENEMANAGER_H
#define SCRIPTSCENEMANAGER_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
#include <memory>

class CDatabaseManager;

//----------------------------------------------------------------------------------------
//
class CSceneManagerSignalEmiter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CSceneManagerSignalEmiter();
  ~CSceneManagerSignalEmiter();

signals:
  void disable(QString sScene);
  void enable(QString sScene);
  void gotoScene(QString sScene);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CSceneManagerScriptCommunicator : public CScriptCommunicator
{
  public:
  CSceneManagerScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CSceneManagerScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptSceneManager : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptSceneManager)

public:
  CScriptSceneManager(std::weak_ptr<CScriptCommunicator> pCommunicator,
                      QPointer<QJSEngine> pEngine);
  CScriptSceneManager(std::weak_ptr<CScriptCommunicator> pCommunicator,
                      QtLua::State* pState);
  ~CScriptSceneManager();

public slots:
  void disable(QVariant scene);
  void enable(QVariant scene);
  void gotoScene(QVariant scene);

signals:
  void SignalQuitLoop();

private:
  QString GetScene(const QVariant& scene, const QString& sSource);

  std::shared_ptr<std::function<void()>> m_spStop;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

//----------------------------------------------------------------------------------------
//
class CCommandEosDisableScene;
class CCommandEosEnableScene;
class CEosScriptSceneManager : public CEosScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptSceneManager)

public:
  CEosScriptSceneManager(std::weak_ptr<CScriptCommunicator> pCommunicator,
                         QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptSceneManager();

  void Disable(const QString& sScene);
  void Enable(const QString& sScene);

signals:
  void SignalQuitLoop();

private:
  std::shared_ptr<CCommandEosDisableScene> m_spCommandDisable;
  std::shared_ptr<CCommandEosEnableScene>  m_spCommandEnable;
  std::shared_ptr<std::function<void()>> m_spStop;
};

#endif // SCRIPTSCENEMANAGER_H
