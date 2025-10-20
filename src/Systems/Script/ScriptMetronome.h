#ifndef CSCRIPTMETRONOME_H
#define CSCRIPTMETRONOME_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QObject>
#include <memory>

class CDatabaseManager;

class CMetronomeSignalEmitter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT

public:
  CMetronomeSignalEmitter();
  ~CMetronomeSignalEmitter();

signals:
  void setBpm(qint32 iBpm);
  void setBeatResource(const QStringList& sResource);
  void setMuted(bool bMuted);
  void setPattern(const QList<double>& vdPattern);
  void setVolume(double dVolume);
  void start();
  void stop();

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CMetronomeScriptCommunicator : public CScriptCommunicator
{
  public:
  CMetronomeScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CMetronomeScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptMetronome : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptMetronome)

public:
  CScriptMetronome(std::weak_ptr<CScriptCommunicator> pCommunicator,
                   QPointer<QJSEngine> pEngine);
  CScriptMetronome(std::weak_ptr<CScriptCommunicator> pCommunicator,
                   QtLua::State* pState);
  ~CScriptMetronome();

public slots:
  void setBpm(qint32 iBpm);
  void setBeatResource(QVariant resource);
  void setMuted(bool bMuted);
  void setPattern(const QList<double>& vdPattern);
  void setVolume(double dVolume);
  void start();
  void stop();

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // CSCRIPTMETRONOME_H
