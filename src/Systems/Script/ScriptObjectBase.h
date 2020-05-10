#ifndef CSCRIPTOBJECTBASE_H
#define CSCRIPTOBJECTBASE_H

#include <QObject>
#include <QJSEngine>
#include <QPointer>
#include <memory>

class CScriptRunnerSignalEmiter;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CScriptObjectBase : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptObjectBase)

public:
  CScriptObjectBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                    QPointer<QJSEngine> pEngine);
  ~CScriptObjectBase();

  void Cleanup();
  void SetCurrentProject(tspProject spProject);

  template<typename T> T* SignalEmitter()
  {
    return qobject_cast<T*>(m_pSignalEmitter);
  }

protected:
  bool CheckIfScriptCanRun();

  virtual void Cleanup_Impl();

  tspProject                                 m_spProject;
  QPointer<CScriptRunnerSignalEmiter>        m_pSignalEmitter;
  QPointer<QJSEngine>                        m_pEngine;
};

#endif // CSCRIPTOBJECTBASE_H
