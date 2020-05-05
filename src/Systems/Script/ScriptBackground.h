#ifndef SCRIPTBACKGROUND_H
#define SCRIPTBACKGROUND_H

#include "ScriptObjectBase.h"
#include <QColor>
#include <memory>

class CDatabaseManager;

class CScriptBackground : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptBackground)

public:
  CScriptBackground(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
                    QPointer<QJSEngine> pEngine);
  ~CScriptBackground();

public slots:
  void setBackgroundColor(QJSValue color);
  void setBackgroundTexture(QJSValue resource);

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QColor                           m_currentColor;
};

#endif // SCRIPTBACKGROUND_H
