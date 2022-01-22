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

  virtual std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);

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
  ~CScriptMetronome();

public slots:
  void setBpm(qint32 iBpm);
  void setBeatResource(QJSValue resource);
  void setMuted(bool bMuted);
  void setPattern(const QList<double>& vdPattern);
  void setVolume(double dVolume);
  void start();
  void stop();

private:
  std::weak_ptr<CDatabaseManager>  m_wpDbManager;
};

#endif // CSCRIPTMETRONOME_H
