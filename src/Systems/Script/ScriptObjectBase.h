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
  CScriptObjectBase(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                    QPointer<QJSEngine> pEngine);
  ~CScriptObjectBase();

  void Cleanup();
  void SetCurrentProject(tspProject spProject);

protected:
  bool CheckIfScriptCanRun();

  virtual void Cleanup_Impl();

  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  tspProject                                 m_spProject;
  QPointer<QJSEngine>                        m_pEngine;
};

#endif // CSCRIPTOBJECTBASE_H
