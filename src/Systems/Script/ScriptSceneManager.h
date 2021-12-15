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
  ~CScriptSceneManager();

public slots:
  void disable(QJSValue scene);
  void enable(QJSValue scene);
  void gotoScene(QJSValue scene);

private:
  QString GetScene(const QJSValue& scene, const QString& sSource);

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
