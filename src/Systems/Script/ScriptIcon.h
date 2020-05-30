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

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

signals:
  void hideIcon(QString sResource);
  void showIcon(QString sResource);
};
Q_DECLARE_METATYPE(CIconSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptIcon : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptIcon)

public:
  CScriptIcon(QPointer<CScriptRunnerSignalEmiter> pEmitter,
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
