#ifndef CSCRIPTEVAL_H
#define CSCRIPTEVAL_H

#include "ScriptObjectBase.h"
#include "ScriptRunnerSignalEmiter.h"
#include <QtLua/Value>
#include <QJSValue>
#include <memory>

class CEvalSignalEmiter : public CScriptRunnerSignalEmiter
{
  Q_OBJECT
public:
  CEvalSignalEmiter();
  ~CEvalSignalEmiter();

  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  std::shared_ptr<CScriptObjectBase> CreateNewScriptObject(QtLua::State* pState) override;

signals:
  void evalQuery(QString sScript);
  void evalReturn(QJSValue value);
};
Q_DECLARE_METATYPE(CEvalSignalEmiter)

//----------------------------------------------------------------------------------------
//
class CScriptEvalBase : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEvalBase)
public:
  CScriptEvalBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                  QPointer<QJSEngine> pEngine);
  CScriptEvalBase(QPointer<CScriptRunnerSignalEmiter> pEmitter,
                  QtLua::State* pState);
  ~CScriptEvalBase();

protected:
  QVariant EvalImpl(const QString& sScript);

signals:
  void SignalQuitLoop();
};

//----------------------------------------------------------------------------------------
//
class CScriptEvalJs : public CScriptEvalBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEvalJs)
public:
  CScriptEvalJs(QPointer<CScriptRunnerSignalEmiter> pEmitter,
              QPointer<QJSEngine> pEngine);
  ~CScriptEvalJs();

public slots:
  QJSValue eval(const QString& sScript);
};

//----------------------------------------------------------------------------------------
//
class CScriptEvalLua : public CScriptEvalBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEvalLua)

public:
  CScriptEvalLua(QPointer<CScriptRunnerSignalEmiter> pEmitter,
              QtLua::State* pState);
  ~CScriptEvalLua();

public slots:
  QVariant eval(const QString& sScript);
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
