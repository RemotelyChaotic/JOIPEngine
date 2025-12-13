#ifndef CSCRIPTOBJECTBASE_H
#define CSCRIPTOBJECTBASE_H

#include <QObject>
#include <QJSEngine>
#include <QPointer>
#include <memory>
#include <type_traits>

class CScriptCommunicator;
class CJsonInstructionSetParser;
namespace QtLua {
  class State;
}
class QJSEngine;
struct SProject;
typedef std::shared_ptr<SProject> tspProject;

class CScriptObjectBase : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CScriptObjectBase)

public:
  CScriptObjectBase(std::weak_ptr<CScriptCommunicator> pCommunicator);
  ~CScriptObjectBase();

  void Cleanup();
  void SetCurrentProject(tspProject spProject);

  template<typename T,
           typename std::enable_if_t<std::is_base_of_v<CScriptCommunicator, T>, void>>
  std::shared_ptr<T> SignalEmitter()
  {
    if (auto spComm = m_wpCommunicator.lock())
    {
      return std::dynamic_pointer_cast<T*>(m_wpCommunicator);
    }
    return nullptr;
  }

signals:
  void SignalInterruptExecution();
  void SignalQuitLoopRequest();

protected:
  bool CheckIfScriptCanRun();
  QVariant RequestValue(const QString& sKey);

  virtual void Cleanup_Impl();

  tspProject                                 m_spProject;
  std::weak_ptr<CScriptCommunicator>         m_wpCommunicator;

  std::shared_ptr<std::function<void()>>     m_spStopBase;
};

//----------------------------------------------------------------------------------------
//
class CJsScriptObjectBase : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CJsScriptObjectBase)

public:
  CJsScriptObjectBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                      QPointer<QJSEngine> pEngine);
  CJsScriptObjectBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                      QtLua::State* pState);
  ~CJsScriptObjectBase();

protected:
  QPointer<QJSEngine>                 m_pEngine = nullptr;
  QtLua::State*                       m_pState = nullptr;
};

//----------------------------------------------------------------------------------------
//
class CEosScriptObjectBase : public CScriptObjectBase
{
  Q_OBJECT
  Q_DISABLE_COPY(CEosScriptObjectBase)

public:
  CEosScriptObjectBase(std::weak_ptr<CScriptCommunicator> pCommunicator,
                       QPointer<CJsonInstructionSetParser> pParser);
  ~CEosScriptObjectBase();

protected:
  QPointer<CJsonInstructionSetParser>  m_pParser;
};

#endif // CSCRIPTOBJECTBASE_H
