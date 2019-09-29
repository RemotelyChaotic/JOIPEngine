#ifndef SCRIPTICON_H
#define SCRIPTICON_H

#include <QJSEngine>
#include <QJSValue>
#include <memory>

class CDatabaseManager;
class CScriptRunnerSignalEmiter;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CScriptIcon : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptIcon)

public:
  CScriptIcon(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
              QJSEngine* pEngine);
  ~CScriptIcon();

  void SetCurrentProject(tspProject spProject);

public slots:
  void hide(QJSValue resource);
  void show(QJSValue resource);

private:
  bool CheckIfScriptCanRun();

  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  tspProject                       m_spProject;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QJSEngine*                       m_pEngine;
};

#endif // SCRIPTICON_H
