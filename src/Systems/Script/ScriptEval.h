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

signals:
  void evalQuery(QString sScript, QString sId);
  void evalReturn(QJSValue value, QString sId);

protected:
  std::shared_ptr<CScriptCommunicator>
  CreateCommunicatorImpl(std::shared_ptr<CScriptRunnerSignalEmiterAccessor> spAccessor) override;
};

//----------------------------------------------------------------------------------------
//
class CEvalScriptCommunicator : public CScriptCommunicator
{
  public:
  CEvalScriptCommunicator(const std::weak_ptr<CScriptRunnerSignalEmiterAccessor>& spEmitter);
  ~CEvalScriptCommunicator() override;

  CScriptObjectBase* CreateNewScriptObject(QPointer<QJSEngine> pEngine) override;
  CScriptObjectBase* CreateNewScriptObject(QPointer<CJsonInstructionSetParser> pParser) override;
  CScriptObjectBase* CreateNewScriptObject(QtLua::State* pState) override;
  CScriptObjectBase* CreateNewSequenceObject() override;
};

//----------------------------------------------------------------------------------------
//
class CScriptEvalBase : public CJsScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEvalBase)

public:
  CScriptEvalBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                  QPointer<QJSEngine> pEngine);
  CScriptEvalBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                  QtLua::State* pState);
  ~CScriptEvalBase();

protected:
  QVariant EvalImpl(const QString& sScript);

signals:
  void SignalQuitLoop();

private:
  std::shared_ptr<std::function<void()>> m_spStop;
};

//----------------------------------------------------------------------------------------
//
class CScriptEvalJs : public CScriptEvalBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptEvalJs)
public:
  CScriptEvalJs(std::weak_ptr<CScriptCommunicator> pCommunicator,
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
  CScriptEvalLua(std::weak_ptr<CScriptCommunicator> pCommunicator,
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
  CEosScriptEval(std::weak_ptr<CScriptCommunicator> pCommunicator,
                 QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptEval();

  QVariant eval(const QString& sScript);

signals:
  void SignalQuitLoop();

private:
  std::shared_ptr<CCommandEosIf> m_spCommandIf;
  std::shared_ptr<CCommandEosEval> m_spCommandEval;
  std::shared_ptr<std::function<void()>> m_spStop;
};

#endif // CSCRIPTEVAL_H
