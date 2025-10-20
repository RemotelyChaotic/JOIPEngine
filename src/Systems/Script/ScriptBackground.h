#ifndef SCRIPTBACKGROUND_H
#define SCRIPTBACKGROUND_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QColor>
#include <memory>

class CDatabaseManager;
class CResourceScriptWrapper;

class CBackgroundSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT

public:
  CBackgroundSignalEmitter();
  ~CBackgroundSignalEmitter();

signals:
  void backgroundColorChanged(QColor color);
  void backgroundTextureChanged(const QString& sResource);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CBackgroundScriptCommunicator : public CScriptCommunicator
{
public:
  CBackgroundScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CBackgroundScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptBackground : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptBackground)

public:
  CScriptBackground(std::weak_ptr<CScriptCommunicator> pCommunicator,
                    QPointer<QJSEngine> pEngine);
  CScriptBackground(std::weak_ptr<CScriptCommunicator> pCommunicator,
                    QtLua::State* pState);
  ~CScriptBackground();

public slots:
  void setBackgroundColor(QVariant color);
  void setBackgroundTexture(QVariant resource);

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
  QColor                           m_currentColor;
};

#endif // SCRIPTBACKGROUND_H
