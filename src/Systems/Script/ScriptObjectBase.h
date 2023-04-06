#ifndef CSCRIPTOBJECTBASE_H
#define CSCRIPTOBJECTBASE_H

#include <QObject>
#include <QJSEngine>
#include <QPointer>
#include <memory>

class CScriptRunnerSignalEmiter;
class CJsonInstructionSetParser;
namespace QtLua {
  class State;
}
class QJSEngine;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CScriptObjectBase : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptObjectBase)

public:
  CScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter);
  ~CScriptObjectBase();

  void Cleanup();
  void SetCurrentProject(tspProject spProject);

  template<typename T> T* SignalEmitter()
  {
    return qobject_cast<T*>(m_pSignalEmitter);
  }

signals:
  void SignalInterruptExecution();

protected:
  bool CheckIfScriptCanRun();

  virtual void Cleanup_Impl();

  tspProject                                 m_spProject;
  QPointer<CScriptRunnerSignalEmiter>        m_pSignalEmitter;
};

//----------------------------------------------------------------------------------------
//
class CJsScriptObjectBase : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CJsScriptObjectBase)

public:
  CJsScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                      QPointer<QJSEngine> pEngine);
  CJsScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                      QtLua::State* pState);
  ~CJsScriptObjectBase();

protected:
  QPointer<QJSEngine>                 m_pEngine = nullptr;
  QtLua::State*                       m_pState = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosScriptObjectBase : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptObjectBase)

public:
  CEosScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                       QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptObjectBase();

protected:
  QPointer<CJsonInstructionSetParser>  m_pParser;
};

#endif // CSCRIPTOBJECTBASE_H
