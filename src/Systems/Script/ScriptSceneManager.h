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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;
  std::shared_ptr<CScriptObjectBase> CreateNewSequenceObject() override;

signals:
  void disable(QString sScene);
  void enable(QString sScene);
  void gotoScene(QString sScene);
};
Q_DECLARE_METATYPE(CSceneManagerSignalEmiter)

//----------------------------------------------------------------------------------------
//
class CScriptSceneManager : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptSceneManager)

public:
  CScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                      QPointer<QJSEngine> pEngine);
  CScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                      QtLua::State* pState);
  ~CScriptSceneManager();

public slots:
  void disable(QVariant scene);
  void enable(QVariant scene);
  void gotoScene(QVariant scene);

private:
  QString GetScene(const QVariant& scene, const QString& sSource);

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
  CEosScriptSceneManager(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                         QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptSceneManager();

  void Disable(const QString& sScene);
  void Enable(const QString& sScene);

private:
  std::shared_ptr<CCommandEosDisableScene> m_spCommandDisable;
  std::shared_ptr<CCommandEosEnableScene>  m_spCommandEnable;
};

#endif // SCRIPTSCENEMANAGER_H
