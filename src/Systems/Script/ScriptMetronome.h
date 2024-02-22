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

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;
  std::shared_ptr<CScriptObjectBase> CreateNewSequenceObject() override;

signals:
  void setBpm(qint32 iBpm);
  void setBeatResource(const QString& sResource);
  void setMuted(bool bMuted);
  void setPattern(const QList<double>& vdPattern);
  void setVolume(double dVolume);
  void start();
  void stop();
};
Q_DECLARE_METATYPE(CMetronomeSignalEmitter)

//----------------------------------------------------------------------------------------
//
class CScriptMetronome : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptMetronome)

public:
  CScriptMetronome(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                   QPointer<QJSEngine> pEngine);
  CScriptMetronome(QPointer<CScriptRunnerSignalEmiter> pEmitter,
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
