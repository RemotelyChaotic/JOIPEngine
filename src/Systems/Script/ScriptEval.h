#ifndef CSCRIPTEVAL_H
#define CSCRIPTEVAL_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QJSValue>
#include <memory>

class CEvalSignalEmiter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CEvalSignalEmiter();
  ~CEvalSignalEmiter();

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine);
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser);

signals:
  void eval(QString sScript);
  void evalReturn(QVariant value);
};
Q_DECLARE_METATYPE(CEvalSignalEmiter)

//----------------------------------------------------------------------------------------
//
class CScriptEval : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEval)
public:
  CScriptEval(QPointer<CScriptRunnerSignalEmiter> pEmitter,
              QPointer<QJSEngine> pEngine);
  ~CScriptEval();

  QJSValue eval(const QString& sScript);

signals:
  void SignalQuitLoop();
};

//----------------------------------------------------------------------------------------
//
class CCommandEosIf;
class CCommandEosEval;
class CEosScriptEval : public CEosScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptEval)

public:
  CEosScriptEval(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                 QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptEval();

  QVariant eval(const QString& sScript);

signals:
  void SignalQuitLoop();

private:
  std::shared_ptr<CCommandEosIf> m_spCommandIf;
  std::shared_ptr<CCommandEosEval> m_spCommandEval;
};

#endif // CSCRIPTEVAL_H
