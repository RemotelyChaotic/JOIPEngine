#ifndef SCRIPTICON_H
#define SCRIPTICON_H

#include "ScriptObjectBase.h"
#include <QJSValue>
#include <memory>

class CDatabaseManager;

class CScriptIcon : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptIcon)

public:
  CScriptIcon(std::shared_ptr<CScriptRunnerSignalEmiter> spEmitter,
              QPointer<QJSEngine> pEngine);
  ~CScriptIcon();

public slots:
  void hide();
  void hide(QJSValue resource);
  void show(QJSValue resource);

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTICON_H
