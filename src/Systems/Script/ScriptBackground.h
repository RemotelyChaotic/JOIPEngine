#ifndef SCRIPTBACKGROUND_H
#define SCRIPTBACKGROUND_H

#include "Systems/Resource.h"
#include <QColor>
#include <QJSEngine>

class CDatabaseManager;
class CScriptRunnerSignalEmiter;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CScriptBackground : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptBackground)

public:
  CScriptBackground(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                    QJSEngine* pEngine);
  ~CScriptBackground();

  void SetCurrentProject(tspProject spProject);

public slots:
  void setBackgroundColor(QJSValue color);
  void setBackgroundTexture(QJSValue resource);

private:
  bool CheckIfScriptCanRun();

  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  tspProject                       m_spProject;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QJSEngine*                       m_pEngine;
  QColor                           m_currentColor;
};

#endif // SCRIPTBACKGROUND_H
