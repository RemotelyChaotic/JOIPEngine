#ifndef SCRIPTTEXTBOX_H
#define SCRIPTTEXTBOX_H

#include "Systems/Resource.h"
#include <QColor>
#include <QJSEngine>
#include <QJSValue>

class CDatabaseManager;
class CScriptRunnerSignalEmiter;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CScriptTextBox : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptTextBox)

public:
  CScriptTextBox(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                 QJSEngine* pEngine);
  ~CScriptTextBox();

  void SetCurrentProject(tspProject spProject);

public slots:
  void setBackgroundColors(QJSValue color);
  void setTextColors(QJSValue color);
  qint32 showButtonPrompts(QJSValue vsLabels);
  QString showInput();
  void showText(QString sText);
  void clear();

signals:
  void SignalQuitLoop();

private:
  std::vector<QColor> GetColors(const QJSValue& color, const QString& sSource);
  bool CheckIfScriptCanRun();

  std::shared_ptr<CScriptRunnerSignalEmiter> m_spSignalEmitter;
  tspProject                       m_spProject;
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QJSEngine*                       m_pEngine;
};

#endif // SCRIPTTEXTBOX_H
