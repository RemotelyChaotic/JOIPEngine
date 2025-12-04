#ifndef SCRIPTICON_H
#define SCRIPTICON_H

#include "CommonScriptHelpers.h"
#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"

#include <QtLua/Value>

#include <QJSValue>
#include <memory>
#include <map>

class CDatabaseManager;
struct SResource;
typedef std::shared_ptr<SResource> tspResource;


class CIconSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CIconSignalEmitter();
  ~CIconSignalEmitter();

signals:
  void hideIcon(QString sResource);
  void iconStateChange(const QString& sResource, bool bShown);
  void showIcon(QString sResource);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CIconScriptCommunicator : public CScriptCommunicator
{
  public:
  CIconScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CIconScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptIcon : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptIcon)

public:
  CScriptIcon(std::weak_ptr<CScriptCommunicator> pCommunicator,
              QPointer<QJSEngine> pEngine);
  CScriptIcon(std::weak_ptr<CScriptCommunicator> pCommunicator,
              QtLua::State* pState);
  ~CScriptIcon();

public slots:
  void hide();
  void hide(QVariant resource);
  void show(QVariant resource);

  void registerIconCallback(const QVariant& resource, QVariant callback);

private slots:
  void HandleIconStateChange(const QString& sResource, bool bShown);

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  std::map<QString, script::tCallbackValue> m_callbacks;
};

#endif // SCRIPTICON_H
