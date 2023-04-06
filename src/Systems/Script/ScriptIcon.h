#ifndef SCRIPTICON_H
#define SCRIPTICON_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
#include <memory>

class CDatabaseManager;
struct SResource;
typedef std::shared_ptr<SResource> tspResource;


class CIconSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CIconSignalEmitter();
  ~CIconSignalEmitter();

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;

signals:
  void hideIcon(QString sResource);
  void showIcon(QString sResource);
};
Q_DECLARE_METATYPE(CIconSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptIcon : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptIcon)

public:
  CScriptIcon(QPointer<CScriptRunnerSignalEmiter> pEmitter,
              QPointer<QJSEngine> pEngine);
  CScriptIcon(QPointer<CScriptRunnerSignalEmiter> pEmitter,
              QtLua::State* pState);
  ~CScriptIcon();

public slots:
  void hide();
  void hide(QVariant resource);
  void show(QVariant resource);

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // SCRIPTICON_H
