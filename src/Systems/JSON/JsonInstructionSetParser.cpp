#include "JsonInstructionSetParser.h"
#include "JsonInstructionSetRunner.h"
#include "JsonInstructionSetSaxParser.h"
#include "JsonInstructionNode.h"

#include <nlohmann/json-schema.hpp>

#include <QAtomicInt>
#include <QDebug>
#include <QEventLoop>
#include <QMutex>
#include <QPointer>
#include <QThread>
#include <memory>
#include <optional>
#include <set>
#include <stack>
#include <string>
#include <tuple>

class CJsonInstructionSetRunnerPrivate;

namespace  {
  const char* c_sCommandsNode = "commands";

  void InstructionSetFormatChecker(const std::string& format, const std::string& value)
  {
    Q_UNUSED(value)
    if (format == "something")
    {
      //if (!check_value_for_something(value))
      //  throw std::invalid_argument("value is not a good something");
    }
    else
    {
      //throw std::logic_error("Don't know how to validate " + format);
    }
  }


  //--------------------------------------------------------------------------------------
  //
  /* json-parse the people - with custom error handler */
  class CustomErrorHandler : public nlohmann::json_schema::basic_error_handler
  {
    public:
      void error(const nlohmann::json_pointer<nlohmann::basic_json<>> &pointer,
                 const nlohmann::json &instance,
                 const std::string &message) override
      {
          nlohmann::json_schema::basic_error_handler::error(pointer, instance, message);
          sError = "Validation of json failed:" + QString::fromStdString(pointer.to_string())
                   + ":" + QString::fromStdString(message);
          qWarning() << sError;
      }

      QString sError = QString();
  };

  //--------------------------------------------------------------------------------------
  //
  SJsonException ExceptionToStruct(const QString& sErr)
  {
    QRegExp rxLine("line ([0-9]*)");
    QRegExp rxColumn("column ([0-9]*)");
    SJsonException ret;
    ret.m_sException = sErr;
    qint32 iPos = 0;
    if ((iPos = rxLine.indexIn(sErr, iPos)) != -1)
    {
      ret.m_iLineNr = rxLine.cap(1).toInt();
    }
    iPos = 0;
    if ((iPos = rxColumn.indexIn(sErr, iPos)) != -1)
    {
      ret.m_iColumn = rxLine.cap(1).toInt();
    }
    return ret;
  }

  SJsonException ExceptionToStruct(const std::exception& e)
  {
    return ExceptionToStruct(e.what());
  }
}

//----------------------------------------------------------------------------------------
// CJsonInstructionSetRunnerWorker & CJsonInstructionSetRunnerWorkerController class definition
//
class CJsonInstructionSetRunnerWorker : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CJsonInstructionSetRunnerWorker)
public:
  CJsonInstructionSetRunnerWorker(std::shared_ptr<CJsonInstructionNode> spNode) :
    QObject(nullptr),
    m_spNextNode(spNode)
  {}
  ~CJsonInstructionSetRunnerWorker() override
  {

  }

public:
  QAtomicInt     m_bInterrupted = 0;
  QAtomicInt     m_bRunning = 0;

public slots:
  //--------------------------------------------------------------------------------------
  //
  CJsonInstructionSetRunner::tRetVal CallNextCommand(ERunerMode runMode)
  {
    if (ERunerMode::eAutoRunAll == runMode)
    {
      SRunnerRetVal retValLast;
      m_bRunning = 1;
      while (nullptr != m_spNextNode)
      {
        if (1 == m_bInterrupted)
        {
          m_bRunning = 0;
          emit CallNextCommandRetVal(runMode, retValLast);
          return retValLast;
        }

        CJsonInstructionSetRunner::tRetVal retVal = CallNextCommandImpl(runMode);
        if (std::holds_alternative<SJsonException>(retVal))
        {
          m_bRunning = 0;
          emit CallNextCommandRetVal(runMode, retVal);
          return retVal;
        }
        retValLast = std::get<SRunnerRetVal>(retVal);
      }
      m_bRunning = 0;
      emit CallNextCommandRetVal(runMode, retValLast);
      return retValLast;
    }
    else if (ERunerMode::eRunOne == runMode)
    {
      if (nullptr != m_spNextNode)
      {
        m_bRunning = 1;
        auto retVal = CallNextCommandImpl(runMode);
        m_bRunning = 0;
        emit CallNextCommandRetVal(runMode, retVal);
        return retVal;
      }
    }

    // nothing to run
    CJsonInstructionSetRunner::tRetVal retVal = SRunnerRetVal{false, std::any()};
    emit CallNextCommandRetVal(runMode, retVal);
    return retVal;
  }

  //--------------------------------------------------------------------------------------
  //
signals:
  void CallNextCommandRetVal(ERunerMode runMode,
                             CJsonInstructionSetRunner::tRetVal retVal);
  void Fork(ERunerMode runMode,
            std::shared_ptr<CJsonInstructionNode> spNode,
            tInstructionMapValue args,
            const QString& sName,
            bool bAutorun);

protected:
  //--------------------------------------------------------------------------------------
  //
  CJsonInstructionSetRunner::tRetVal CallNextCommandImpl(ERunerMode runMode)
  {
    IJsonInstructionBase::tRetVal vRetVal = CallCommand();
    if (std::holds_alternative<SJsonException>(vRetVal))
    {
      return std::get<SJsonException>(vRetVal);
    }
    else if (std::holds_alternative<SRunRetVal<ENextCommandToCall::eChild>>(vRetVal))
    {
      auto& ret = std::get<SRunRetVal<ENextCommandToCall::eChild>>(vRetVal);
      return SRunnerRetVal{NextCommand(ret.m_type, ret.m_iBegin, ret.m_iEnd), std::any()};
    }
    else if (std::holds_alternative<SRunRetVal<ENextCommandToCall::eSibling>>(vRetVal))
    {
      auto& ret = std::get<SRunRetVal<ENextCommandToCall::eSibling>>(vRetVal);
      return SRunnerRetVal{NextCommand(ret.m_type), std::any()};
    }
    else if (std::holds_alternative<SRunRetVal<ENextCommandToCall::eForkThis>>(vRetVal))
    {
      auto& ret = std::get<SRunRetVal<ENextCommandToCall::eForkThis>>(vRetVal);
      for (auto& fork : ret.m_vForks)
      {
        emit Fork(runMode, m_spNextNode, fork.m_args, fork.m_sName, fork.m_bAutorun);
      }
      return SRunnerRetVal{NextCommand(ENextCommandToCall::eSibling), std::any()};
    }
    else if (std::holds_alternative<SRunRetVal<ENextCommandToCall::eFinish>>(vRetVal))
    {
      m_spNextNode = nullptr;
      return SRunnerRetVal{false, std::get<SRunRetVal<ENextCommandToCall::eFinish>>(vRetVal).m_retVal};
    }
    else return SRunnerRetVal{NextCommand(ENextCommandToCall::eChild, 0), std::any()};
  }

  //--------------------------------------------------------------------------------------
  //
  IJsonInstructionBase::tRetVal CallCommand()
  {
    if (nullptr != m_spNextNode)
    {
      if (auto spCommand = m_spNextNode->m_wpCommand.lock())
      {
        IJsonInstructionBase::tRetVal retVal =
            spCommand->Call(m_spNextNode->m_actualArgs);
        return retVal;
      }
    }
    return SJsonException{"Internal error.", "", "", 0, 0};
  }

  //--------------------------------------------------------------------------------------
  //
  void MakeChildrenExclusive(qint32 iBegin = 0, qint32 iEnd = -1)
  {
    if (nullptr != m_spNextNode)
    {
      for (qint32 i = 0; i < static_cast<qint32>(m_spNextNode->m_spChildren.size()); ++i)
      {
        if (i < iBegin) { m_spNextNode->m_spChildren[i]->m_bEnabled = false; }
        else
        {
          if (iEnd < iBegin || i < iEnd) { m_spNextNode->m_spChildren[i]->m_bEnabled = true; }
          else { m_spNextNode->m_spChildren[i]->m_bEnabled = false; }
        }
      }
    }
  }

  //--------------------------------------------------------------------------------------
  //
  bool NextCommand(ENextCommandToCall nextCmd, qint32 iBegin = 0, qint32 iEnd = -1)
  {
    if (nullptr != m_spNextNode)
    {
      // dfs
      if (m_spNextNode->m_spChildren.size() > 0 &&
          ENextCommandToCall::eChild == nextCmd)
      {
        iBegin = std::min(static_cast<qint32>(m_spNextNode->m_spChildren.size()-1),
                          std::max(0, iBegin));
        MakeChildrenExclusive(iBegin, iEnd);
        m_spNextNode = *std::next(m_spNextNode->m_spChildren.begin(), iBegin);
        return true;
      }
      else
      {
        auto spParent = m_spNextNode->m_wpParent.lock();
        while (nullptr != spParent)
        {
          auto it =
            std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), m_spNextNode);
          qint32 iIndex = std::distance(spParent->m_spChildren.begin(), it);
          while (iIndex+1 < static_cast<qint32>(spParent->m_spChildren.size()))
          {
            ++iIndex;
            if (spParent->m_spChildren[static_cast<size_t>(iIndex)]->m_bEnabled)
            {
              m_spNextNode = spParent->m_spChildren[static_cast<size_t>(iIndex)];
              return true;
            }
          }

          // go up one since we haven't found a node that can be executed
          m_spNextNode = spParent;
          spParent = spParent->m_wpParent.lock();
        }
      }
    }
    // nothing found -> reset since all commands have run
    m_spNextNode = nullptr;
    return false;
  }

private:
  std::shared_ptr<CJsonInstructionNode>                   m_spNextNode;
};

//----------------------------------------------------------------------------------------
//
class CJsonInstructionSetRunnerWorkerController : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CJsonInstructionSetRunnerWorkerController)

public:
  CJsonInstructionSetRunnerWorkerController(std::shared_ptr<CJsonInstructionNode> spNode) :
    QObject(nullptr),
    m_spWorker(std::make_shared<CJsonInstructionSetRunnerWorker>(spNode)),
    m_pThread(new QThread(this)),
    m_mutex(QMutex::Recursive)
  {
    QMutexLocker locker(&m_mutex);
    connect(m_spWorker.get(), &CJsonInstructionSetRunnerWorker::CallNextCommandRetVal,
            this, &CJsonInstructionSetRunnerWorkerController::SlotCallNextCommandRetVal, Qt::QueuedConnection);
    connect(m_spWorker.get(), &CJsonInstructionSetRunnerWorker::Fork,
            this, &CJsonInstructionSetRunnerWorkerController::Fork, Qt::QueuedConnection);

    m_pThread->setObjectName("CJsonInstructionSetRunnerWorkerController");
    m_pThread->start();
    while (!m_pThread->isRunning())
    {
      thread()->sleep(1);
    }
    m_spWorker->moveToThread(m_pThread.data());
  }
  ~CJsonInstructionSetRunnerWorkerController()
  {
    QMutexLocker locker(&m_mutex);
    Interrupt();
    m_spWorker = nullptr;
    delete m_pThread;
    m_pThread = nullptr;
  }

  CJsonInstructionSetRunner::tRetVal CallNextCommand(ERunerMode runMode, bool bBlocking)
  {
    QMutexLocker locker(&m_mutex);
    m_retVal = SRunnerRetVal{false, std::any()};
    m_bCommandDone = false;
    m_bBlockingCall = bBlocking;
    bool bOk = false;
    CJsonInstructionSetRunner::tRetVal retVal;
    if (bBlocking)
    {
      bOk = QMetaObject::invokeMethod(m_spWorker.get(), "CallNextCommand",
                                      Qt::BlockingQueuedConnection,
                                      Q_RETURN_ARG(CJsonInstructionSetRunner::tRetVal, retVal),
                                      Q_ARG(ERunerMode, runMode));
    }
    else
    {
      bOk = QMetaObject::invokeMethod(m_spWorker.get(), "CallNextCommand",
                                      Qt::QueuedConnection,
                                      Q_ARG(ERunerMode, runMode));
    }
    assert(bOk);
    if (!bOk) { m_bCommandDone = false; }
    return retVal;
  }

  void Interrupt()
  {
    QMutexLocker locker(&m_mutex);
    if (nullptr != m_pThread)
    {
      m_spWorker->m_bInterrupted = true;
      m_pThread->quit();
      while (!m_pThread->isFinished())
      {
        thread()->sleep(1);
      }
    }
  }

  bool IsRunning() const { QMutexLocker locker(&m_mutex); return m_spWorker->m_bRunning == 1; }
  bool HasMoreCommands() const { QMutexLocker locker(&m_mutex); return m_retVal.m_bHasMoreCommands; }
  bool IsBlockingCall() const { QMutexLocker locker(&m_mutex); return m_bBlockingCall; }
  bool IsDoneWithCommand() const { QMutexLocker locker(&m_mutex); return m_bCommandDone; }
  std::any RetVal() const { QMutexLocker locker(&m_mutex); return m_retVal.m_retVal; }

signals:
  void CallNextCommandRetVal(ERunerMode runMode, CJsonInstructionSetRunner::tRetVal retVal);
  void Fork(ERunerMode runMode,
            std::shared_ptr<CJsonInstructionNode> spNode,
            tInstructionMapValue args,
            const QString& sName,
            bool bAutorun);

protected slots:
  void SlotCallNextCommandRetVal(ERunerMode runMode, CJsonInstructionSetRunner::tRetVal retVal)
  {
    QMutexLocker locker(&m_mutex);
    if (std::holds_alternative<SRunnerRetVal>(retVal))
    {
      m_retVal = std::get<SRunnerRetVal>(retVal);
    }
    m_bCommandDone = true;
    locker.unlock();
    emit CallNextCommandRetVal(runMode, retVal);
  }

public:
  std::shared_ptr<CJsonInstructionSetRunnerWorker> m_spWorker;
  QPointer<QThread>                                m_pThread;
  mutable QMutex                                   m_mutex;
  SRunnerRetVal                                    m_retVal;
  bool                                             m_bCommandDone = true;
  bool                                             m_bBlockingCall = false;
};

//----------------------------------------------------------------------------------------
// CJsonInstructionSetRunnerPrivate class definition
//
class CJsonInstructionSetRunnerPrivate : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY(CJsonInstructionSetRunnerPrivate)
  friend class CJsonInstructionSetRunner;
  friend class CJsonInstructionSetParserPrivate;
  friend class CJSonSaxParser;

public:
  CJsonInstructionSetRunnerPrivate() :
    QObject(),
    m_vspBuiltCommands(),
    m_spWorkerController(nullptr),
    m_spValidationErrorHandler(std::make_unique<CustomErrorHandler>()),
    m_validator(nullptr, InstructionSetFormatChecker, nullptr),
    m_jsonBaseSchema(),
    m_jsonParsed(),
    m_sJson(),
    m_parseError(),
    m_bValidationOk(false)
  {
  }

  ~CJsonInstructionSetRunnerPrivate() override
  {
    // makes debugging easier
    if (nullptr != m_spWorkerController)
    {
      m_spWorkerController = nullptr;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  SJsonException ParseError() const
  {
    return m_parseError;
  }

  //--------------------------------------------------------------------------------------
  //
  bool Validate(const std::string& json)
  {
    if (m_jsonBaseSchema.is_null())
    {
      m_parseError = SJsonException();
      m_bValidationOk = true;
      return m_bValidationOk;
    }

    // parse json with default parser
    try
    {
      m_jsonParsed = nlohmann::json::parse(json);
    }
    catch (const std::exception &e)
    {
      m_parseError = ExceptionToStruct(e);
      return false;
    }

    // set schema
    try
    {
      m_validator.set_root_schema(m_jsonBaseSchema);
    }
    catch (const std::exception &e)
    {
      m_parseError = ExceptionToStruct(e);
      m_bValidationOk = false;
    }

    // validate the document - uses a custom error-handler, because we don't like
    // to throw around things
    m_validator.validate(m_jsonParsed, *m_spValidationErrorHandler);
    if (static_cast<bool>(*m_spValidationErrorHandler))
    {
      m_parseError = ExceptionToStruct(m_spValidationErrorHandler->sError);
      m_bValidationOk = false;
    }
    else
    {
      m_parseError = SJsonException();
      m_bValidationOk = true;
    }

    return m_bValidationOk;
  }

  //--------------------------------------------------------------------------------------
  //
  CJsonInstructionSetRunner::tRetVal CallNextCommand(ERunerMode runMode, bool bBlocking)
  {
    CJsonInstructionSetRunner::tRetVal retVal =
        m_spWorkerController->CallNextCommand(runMode, bBlocking);

    if (!bBlocking)
    {
      return SRunnerRetVal{true, std::any()};
    }
    else
    {
      return retVal;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void Interrupt()
  {
    if (nullptr != m_spWorkerController)
    {
      m_spWorkerController->Interrupt();
    }
  }

  //--------------------------------------------------------------------------------------
  //
  bool IsRunning() const
  {
    return nullptr != m_spWorkerController && m_spWorkerController->IsRunning();
  }

  //--------------------------------------------------------------------------------------
  //
  CJsonInstructionSetRunner::tRetVal Run(const QString& sInstructionSet, ERunerMode runMode,
                                         bool bBlocking)
  {
    if (!m_bValidationOk)
    { return SJsonException{"Parse error.", "", "", 0, 0}; }

    m_spWorkerController = nullptr;

    // validation success, start actually running the commands
    auto it = std::find_if(m_vspBuiltCommands.begin(), m_vspBuiltCommands.end(),
                           [&sInstructionSet](const std::pair<QString, std::shared_ptr<CJsonInstructionNode>>& pair){
      return pair.first == sInstructionSet;
    });
    if (m_vspBuiltCommands.end() != it)
    {
      if (static_cast<qint32>(it->second->m_spChildren.size()) > 0)
      {
        m_spWorkerController.reset(
              new CJsonInstructionSetRunnerWorkerController(*it->second->m_spChildren.begin()));
        if (!bBlocking)
        {
          connect(m_spWorkerController.get(), &CJsonInstructionSetRunnerWorkerController::CallNextCommandRetVal,
                  this, [this](ERunerMode, CJsonInstructionSetRunner::tRetVal retVal) {
            emit CommandRetVal(retVal);
          });
        }
        connect(m_spWorkerController.get(), &CJsonInstructionSetRunnerWorkerController::Fork,
                this, &CJsonInstructionSetRunnerPrivate::SlotFork);
        return CallNextCommand(runMode, bBlocking);
      }
    }

    return
        SJsonException{"Can not run instruction set " + sInstructionSet, sInstructionSet, sInstructionSet, 0, 0};
  }

signals:
  void CommandRetVal(CJsonInstructionSetRunner::tRetVal retVal);
  void Fork(std::shared_ptr<CJsonInstructionSetRunner> spNewRunner, const QString& sForkCommandsName, bool bAutorun);

protected slots:
  //--------------------------------------------------------------------------------------
  //
  void SlotFork(ERunerMode /*runMode*/,
                std::shared_ptr<CJsonInstructionNode> spNode,
                tInstructionMapValue args,
                const QString& sName,
                bool bAutorun)
  {
    std::shared_ptr<CJsonInstructionNode> spNewNode =
        std::make_shared<CJsonInstructionNode>(*spNode);
    spNewNode->ReparentChildren();
    // change args and make an orphan
    spNewNode->m_actualArgs = args;

    std::shared_ptr<CJsonInstructionNode> spNewRootNode =
        std::make_shared<CJsonInstructionNode>();
    spNewRootNode->m_spChildren.push_back(spNewNode);
    spNewRootNode->m_sName = sName;

    spNewNode->m_wpParent = spNewRootNode;

    std::shared_ptr<CJsonInstructionSetRunner> spRunner =
        std::make_shared<CJsonInstructionSetRunner>();
    spRunner->m_pPrivate->m_instructionMap = m_instructionMap;
    spRunner->m_pPrivate->m_vspBuiltCommands = { {sName, spNewRootNode} };
    // rest is not needed
    spRunner->m_pPrivate->m_bValidationOk = true;

    spRunner->m_pPrivate->m_spWorkerController =
        std::make_unique<CJsonInstructionSetRunnerWorkerController>(spNewNode);
    emit Fork(spRunner, sName, bAutorun);
  }

protected:
  std::map<QString, std::shared_ptr<IJsonInstructionBase>> m_instructionMap;
  std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>>
                                                          m_vspBuiltCommands;
  std::unique_ptr<CJsonInstructionSetRunnerWorkerController>
                                                          m_spWorkerController;
  std::unique_ptr<CustomErrorHandler>                     m_spValidationErrorHandler;
  nlohmann::json_schema::json_validator                   m_validator;
  nlohmann::json                                          m_jsonBaseSchema;
  nlohmann::json                                          m_jsonParsed;
  std::string                                             m_sJson;
  SJsonException                                          m_parseError;
  bool                                                    m_bValidationOk;
};

//----------------------------------------------------------------------------------------
// CJSonSaxParser helper funcntions
//
std::variant<EArgumentType, std::false_type>
TypeFromNode(const std::shared_ptr<CJsonInstructionNode>& spNode,
             const std::string& sKey,
             bool& bArray)
{
  tInstructionMapType* argsDefinition = spNode->m_argsDefinition;
  SInstructionArgumentType* argDefinition = spNode->m_argDefinitionArr;

  EArgumentType ret = EArgumentType::eBool;
  if (nullptr != argsDefinition)
  {
    auto it = argsDefinition->find(QString::fromStdString(sKey));
    if (argsDefinition->end() == it) { it = argsDefinition->find("*"); } // wildcard to allow all commands as children
    if (argsDefinition->end() != it) { ret = it->second.m_type; bArray = false; }
  }
  else if (nullptr != argDefinition) { ret  = argDefinition->m_type; bArray = true; }
  else return std::false_type();

  return ret;
}

//----------------------------------------------------------------------------------------
//
void InsertValueIntoArgs(const QVariant& value,
                         tInstructionMapValue& actualArgs, const std::string& sKey,
                         EArgumentType type,
                         bool bArray)
{
  if (!bArray)
  {
    actualArgs.insert(
          { QString::fromStdString(sKey),
            ValuefromVariant(value, type) });
  }
  else
  {
    // vector not found -> insert new vector
    auto it = actualArgs.find(QString::fromStdString(sKey));
    if (actualArgs.end() == it)
    {
      actualArgs.insert({QString::fromStdString(sKey),
                         {type, tInstructionArrayValue()}});
    }

    it = actualArgs.find(QString::fromStdString(sKey));
    tInstructionArrayValue& array = std::get<tInstructionArrayValue>(it->second.m_value);
    array.push_back(ValuefromVariant(value, type));
  }
}

//----------------------------------------------------------------------------------------
// CJSonSaxParser implementation
//
bool CJSonSaxParser::null()
{
  // null values are not allowed
  return false;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::boolean(bool val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(val);
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
using tNLohmanNumber = std::variant<CJSonSaxParser::number_integer_t,
                                    CJSonSaxParser::number_unsigned_t,
                                    CJSonSaxParser::number_float_t>;
QVariant FallbackNumberConverter(tNLohmanNumber val, EArgumentType toType)
{
  if (std::holds_alternative<CJSonSaxParser::number_integer_t>(val))
  {
    switch (toType)
    {
      case EArgumentType::eInt64: return QVariant(static_cast<qint64>(std::get<CJSonSaxParser::number_integer_t>(val)));
      case EArgumentType::eUInt64: return QVariant(static_cast<quint64>(std::get<CJSonSaxParser::number_integer_t>(val)));
      case EArgumentType::eDouble: return QVariant(static_cast<double>(std::get<CJSonSaxParser::number_integer_t>(val)));
      default: return QVariant();
    }
  }
  else if (std::holds_alternative<CJSonSaxParser::number_unsigned_t>(val))
  {
    switch (toType)
    {
      case EArgumentType::eInt64: return QVariant(static_cast<qint64>(std::get<CJSonSaxParser::number_unsigned_t>(val)));
      case EArgumentType::eUInt64: return QVariant(static_cast<quint64>(std::get<CJSonSaxParser::number_unsigned_t>(val)));
      case EArgumentType::eDouble: return QVariant(static_cast<double>(std::get<CJSonSaxParser::number_unsigned_t>(val)));
      default: return QVariant();
    }
  }
  else if (std::holds_alternative<CJSonSaxParser::number_float_t>(val))
  {
    switch (toType)
    {
      case EArgumentType::eInt64: return QVariant(static_cast<qint64>(std::get<CJSonSaxParser::number_float_t>(val)));
      case EArgumentType::eUInt64: return QVariant(static_cast<quint64>(std::get<CJSonSaxParser::number_float_t>(val)));
      case EArgumentType::eDouble: return QVariant(static_cast<double>(std::get<CJSonSaxParser::number_float_t>(val)));
      default: return QVariant();
    }
  }
  return QVariant();
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::number_integer(number_integer_t val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var = QVariant::fromValue(val);
        if (!var.convert(*foundValueType))
        {
          var = FallbackNumberConverter(val, *foundValueType);
          if (!var.isValid())
          {
            return false;
          }
        }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::number_unsigned(number_unsigned_t val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var = QVariant::fromValue(val);
        if (!var.convert(*foundValueType))
        {
          var = FallbackNumberConverter(val, *foundValueType);
          if (!var.isValid())
          {
            return false;
          }
        }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::number_float(number_float_t val, const string_t&)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(val);
        if (!var.convert(*foundValueType))
        {
          var = FallbackNumberConverter(val, *foundValueType);
          if (!var.isValid())
          {
            return false;
          }
        }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::string(string_t& val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QVariant var(QString::fromStdString(val));
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::binary(binary_t& val)
{
  static const std::set<EArgumentType> c_convertableTypes = {
    EArgumentType::eBool,
    EArgumentType::eInt64,
    EArgumentType::eUInt64,
    EArgumentType::eDouble,
    EArgumentType::eString
  };

  if (m_bParsingCommands && !m_parseStack.top()->m_bIgnoreChildren &&
      (nullptr != m_parseStack.top()->m_argsDefinition ||
       nullptr != m_parseStack.top()->m_argDefinitionArr))
  {
    bool bArray = false;
    auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
    if (std::holds_alternative<EArgumentType>(type))
    {
      const auto& foundValueType = c_convertableTypes.find(std::get<EArgumentType>(type));
      if (foundValueType != c_convertableTypes.end())
      {
        QByteArray arr;
        for (uchar c : val) { arr += c; }
        QVariant var(QString::fromUtf8(arr));
        if (!var.convert(*foundValueType)) { return false; }
        InsertValueIntoArgs(var, m_parseStack.top()->m_actualArgs, m_sCurrentKey,
                            *foundValueType, bArray);
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::start_object(std::size_t elements)
{
  Q_UNUSED(elements)

  if (m_bParsingCommands)
  {
    // if we're in an ignored branch
    if (m_parseStack.top()->m_bIgnoreChildren)
    {
      CreateAndAddIgnoreNode();
      return true;
    }

    std::shared_ptr<IJsonInstructionBase> spCurrentCommand =
        m_parseStack.top()->m_wpCommand.lock();

    // no command is parsing yet
    if (nullptr == spCurrentCommand)
    {
      auto it = m_instructionMap.find(QString::fromStdString(m_sCurrentKey));
      if (m_instructionMap.end() != it)
      {
        std::shared_ptr<CJsonInstructionNode> spNode =
            std::make_shared<CJsonInstructionNode>();
        spNode->m_sName = QString::fromStdString(m_sCurrentKey);
        spNode->m_wpCommand = it->second;
        spNode->m_wpParent = m_parseStack.top();
        spNode->m_bIgnoreChildren = false;
        spNode->m_argsDefinition = &it->second->ArgList();
        m_parseStack.top()->m_spChildren.push_back(spNode);
        m_parseStack.push(spNode);
      }
      else
      {
        std::shared_ptr<CJsonInstructionNode> spNode =
            std::make_shared<CJsonInstructionNode>();
        spNode->m_sName = QString::fromStdString(m_sCurrentKey);
        spNode->m_bIgnoreChildren = false;
        spNode->m_wpParent = m_parseStack.top();
        m_parseStack.top()->m_spChildren.push_back(spNode);
        m_parseStack.push(spNode);
        return true;
      }
    }
    else
    {
      // a command is being parsed
      std::shared_ptr<CJsonInstructionNode> spParentNode = m_parseStack.top();
      tInstructionMapType* argsDefinition = spParentNode->m_argsDefinition;
      SInstructionArgumentType* argDefinitionArr = spParentNode->m_argDefinitionArr;
      if (nullptr != argsDefinition || nullptr != argDefinitionArr)
      {
        bool bArray = false;
        auto type = TypeFromNode(m_parseStack.top(), m_sCurrentKey, bArray);
        if (std::holds_alternative<EArgumentType>(type))
        {
          // is it a new object?
          if (EArgumentType::eObject == std::get<EArgumentType>(type)._to_integral())
          {
            if (!bArray)
            {
              auto itInstruction = m_instructionMap.find(QString::fromStdString(m_sCurrentKey));
              if (m_instructionMap.end() != itInstruction)
              {
                spParentNode->m_actualArgs.insert({QString::fromStdString(m_sCurrentKey),
                                                   {EArgumentType::eObject, QString::fromStdString(m_sCurrentKey)}});
                std::shared_ptr<CJsonInstructionNode> spNode = std::make_shared<CJsonInstructionNode>();
                spNode->m_sName = QString::fromStdString(m_sCurrentKey);
                spNode->m_wpCommand = itInstruction->second;
                spNode->m_wpParent = m_parseStack.top();
                spNode->m_bIgnoreChildren = false;
                spNode->m_argsDefinition = &itInstruction->second->ArgList();
                m_parseStack.top()->m_spChildren.push_back(spNode);
                m_parseStack.push(spNode);
              }
              else
              {
                CreateAndAddIgnoreNode();
              }
            }
            else
            {
              // vector not found -> insert new vector
              auto it = spParentNode->m_actualArgs.find(QString::fromStdString(m_sCurrentKey));
              if (spParentNode->m_actualArgs.end() == it)
              {
                spParentNode->m_actualArgs.insert({QString::fromStdString(m_sCurrentKey),
                                                  {std::get<EArgumentType>(type), tInstructionArrayValue()}});
              }

              it = spParentNode->m_actualArgs.find(QString::fromStdString(m_sCurrentKey));
              tInstructionArrayValue& array = std::get<tInstructionArrayValue>(it->second.m_value);
              const QString sElemName = QString::number(array.size());
              array.push_back({EArgumentType::eObject, sElemName});

              // insert mostly empty node
              std::shared_ptr<CJsonInstructionNode> spNode = std::make_shared<CJsonInstructionNode>();
              spNode->m_sName = sElemName;
              spNode->m_wpParent = m_parseStack.top();
              spNode->m_bIgnoreChildren = false;
              m_parseStack.top()->m_spChildren.push_back(spNode);
              m_parseStack.push(spNode);
            }
          }
          // is it part of the arguments
          else if (EArgumentType::eMap == std::get<EArgumentType>(type)._to_integral())
          {
            std::shared_ptr<CJsonInstructionNode> spNode = std::make_shared<CJsonInstructionNode>();
            spNode->m_sName = QString::fromStdString(m_sCurrentKey);
            spNode->m_wpCommand = spParentNode->m_wpCommand;
            spNode->m_wpParent = m_parseStack.top();
            spNode->m_bIgnoreChildren = false;
            if (!bArray)
            {
              auto itArg = argsDefinition->find(QString::fromStdString(m_sCurrentKey));
              if (argsDefinition->end() == itArg) { itArg = argsDefinition->find("*"); }
              spNode->m_argsDefinition = &std::get<tInstructionMapType>(itArg->second.m_nestedType);
            }
            else
            {
              spNode->m_argsDefinition = &std::get<tInstructionMapType>(argDefinitionArr->m_nestedType);
            }
            m_parseStack.top()->m_spChildren.push_back(spNode);
            m_parseStack.push(spNode);
          }
          else
          {
            auto it = m_instructionMap.find(QString::fromStdString(m_sCurrentKey));
            if (m_instructionMap.end() != it)
            {
              std::shared_ptr<CJsonInstructionNode> spNode =
                  std::make_shared<CJsonInstructionNode>();
              spNode->m_sName = QString::fromStdString(m_sCurrentKey);
              spNode->m_wpCommand = it->second;
              spNode->m_wpParent = m_parseStack.top();
              spNode->m_bIgnoreChildren = false;
              spNode->m_argsDefinition = &it->second->ArgList();
              m_parseStack.top()->m_spChildren.push_back(spNode);
              m_parseStack.push(spNode);
            }
            else
            {
              std::shared_ptr<CJsonInstructionNode> spNode =
                  std::make_shared<CJsonInstructionNode>();
              spNode->m_sName = QString::fromStdString(m_sCurrentKey);
              spNode->m_bIgnoreChildren = false;
              spNode->m_wpParent = m_parseStack.top();
              m_parseStack.top()->m_spChildren.push_back(spNode);
              m_parseStack.push(spNode);
              return true;
            }
          }
        }
        else
        {
          CreateAndAddIgnoreNode();
        }
      }
      else
      {
        // we didn't find an argument definition
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::end_object()
{
  if (!m_parseStack.empty())
  {
    std::shared_ptr<CJsonInstructionNode> spChild = m_parseStack.top();
    m_parseStack.pop();
    if (m_parseStack.empty())
    {
      m_bParsingCommands = false;
    }
    else
    {
      m_sCurrentKey = m_parseStack.top()->m_sName.toStdString();

      std::shared_ptr<CJsonInstructionNode> spParent = m_parseStack.top();
      std::shared_ptr<IJsonInstructionBase> spChildCommand = spChild->m_wpCommand.lock();
      std::shared_ptr<IJsonInstructionBase> spParentCommand = spParent->m_wpCommand.lock();

      if (nullptr == spChildCommand)
      {
        // squash children
        auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
        if (spParent->m_spChildren.end() != itChild)
        {
          spParent->m_spChildren.erase(itChild);
          spParent->m_spChildren.insert(spParent->m_spChildren.end(),
                                        spChild->m_spChildren.begin(), spChild->m_spChildren.end());
          for (auto& spChildChild : spChild->m_spChildren)
          {
            spChildChild->m_wpParent = spParent;
          }
        }
      }
      else
      {
        if (nullptr != spParentCommand)
        {
          bool bArray = false;
          auto type = TypeFromNode(spParent, spChild->m_sName.toStdString(), bArray);
          if (std::holds_alternative<EArgumentType>(type))
          {
            // we popped a map type, so we need to insert the values into the arguments as a map value
            // we can also just remove the node from the children since we are done processing
            // the object
            if (EArgumentType::eMap == std::get<EArgumentType>(type)._to_integral())
            {
              if (!bArray)
              {
                spParent->m_actualArgs.insert({spChild->m_sName,
                                               {EArgumentType::eMap, spChild->m_actualArgs}});
              }
              else
              {
                // vector not found -> insert new vector
                auto it = spParent->m_actualArgs.find(spChild->m_sName);
                if (spParent->m_actualArgs.end() == it)
                {
                  spParent->m_actualArgs.insert({spChild->m_sName,
                                                 {EArgumentType::eArray, tInstructionArrayValue()}});
                }
                it = spParent->m_actualArgs.find(spChild->m_sName);
                tInstructionArrayValue& array = std::get<tInstructionArrayValue>(it->second.m_value);
                array.push_back({EArgumentType::eMap, spChild->m_actualArgs});
              }
              auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
              if (spParent->m_spChildren.end() != itChild)
              {
                spParent->m_spChildren.erase(itChild);
                spParent->m_spChildren.insert(spParent->m_spChildren.end(),
                                              spChild->m_spChildren.begin(), spChild->m_spChildren.end());
                for (auto& spChildChild : spChild->m_spChildren)
                {
                  spChildChild->m_wpParent = spParent;
                }
              }
            }
            else if (EArgumentType::eObject == std::get<EArgumentType>(type)._to_integral())
            {
              auto it = spChild->m_actualArgs.find(spChild->m_sName);
              if (spChild->m_actualArgs.end() != it)
              {
                if (!bArray)
                {
                  spParent->m_actualArgs.insert({spChild->m_sName,
                                                 {EArgumentType::eObject, it->second.m_value}});
                }
                else
                {
                  // vector not found -> insert new vector
                  auto it = spParent->m_actualArgs.find(spChild->m_sName);
                  if (spParent->m_actualArgs.end() == it)
                  {
                    spParent->m_actualArgs.insert({spChild->m_sName,
                                                   {EArgumentType::eArray, tInstructionArrayValue()}});
                  }
                  it = spParent->m_actualArgs.find(spChild->m_sName);
                  tInstructionArrayValue& array = std::get<tInstructionArrayValue>(it->second.m_value);
                  array.push_back({EArgumentType::eObject, it->second.m_value});
                }
                auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
                if (spParent->m_spChildren.end() != itChild)
                {
                  spParent->m_spChildren.erase(itChild);
                  spParent->m_spChildren.insert(spParent->m_spChildren.end(),
                                                spChild->m_spChildren.begin(), spChild->m_spChildren.end());
                  for (auto& spChildChild : spChild->m_spChildren)
                  {
                    spChildChild->m_wpParent = spParent;
                  }
                }
              }
            }
          }
        }
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::start_array(std::size_t elements)
{
  Q_UNUSED(elements)
  if (m_bParsingCommands)
  {
    // if we're in an ignored branch
    if (m_parseStack.top()->m_bIgnoreChildren)
    {
      CreateAndAddIgnoreNode();
      return true;
    }

    std::shared_ptr<IJsonInstructionBase> spCurrentCommand =
        m_parseStack.top()->m_wpCommand.lock();

    // no command is parsing yet
    if (nullptr == spCurrentCommand)
    {
      std::shared_ptr<CJsonInstructionNode> spNode =
          std::make_shared<CJsonInstructionNode>();
      spNode->m_sName = QString::fromStdString(m_sCurrentKey);
      spNode->m_bIgnoreChildren = false;
      spNode->m_wpParent = m_parseStack.top();
      m_parseStack.top()->m_spChildren.push_back(spNode);
      m_parseStack.push(spNode);
      return true;
    }
    else
    {
      // a command is being parsed
      std::shared_ptr<CJsonInstructionNode> spParentNode = m_parseStack.top();
      tInstructionMapType* argsDefinition = spParentNode->m_argsDefinition;
      if (nullptr != argsDefinition)
      {
        auto itArg = argsDefinition->find(QString::fromStdString(m_sCurrentKey));
        if (argsDefinition->end() != itArg)
        {
          // is it an array?
          if (EArgumentType::eArray == itArg->second.m_type._to_integral())
          {
            std::shared_ptr<CJsonInstructionNode> spNode = std::make_shared<CJsonInstructionNode>();
            spNode->m_sName = QString::fromStdString(m_sCurrentKey);
            spNode->m_wpCommand = spParentNode->m_wpCommand;
            spNode->m_wpParent = m_parseStack.top();
            spNode->m_bIgnoreChildren = false;
            spNode->m_argDefinitionArr = std::get<std::shared_ptr<SInstructionArgumentType>>(itArg->second.m_nestedType).get();
            m_parseStack.top()->m_spChildren.push_back(spNode);
            m_parseStack.push(spNode);
          }
          else
          {
            return false;
          }
        }
        else
        {
          CreateAndAddIgnoreNode();
        }
      }
      else
      {
        return false;
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::end_array()
{
  if (m_bParsingCommands)
  {
    std::shared_ptr<CJsonInstructionNode> spChild = m_parseStack.top();
    m_parseStack.pop();
    if (m_parseStack.empty())
    {
      m_bParsingCommands = false;
    }
    else
    {
      m_sCurrentKey = m_parseStack.top()->m_sName.toStdString();

      std::shared_ptr<CJsonInstructionNode> spParent = m_parseStack.top();
      std::shared_ptr<IJsonInstructionBase> spChildCommand = spChild->m_wpCommand.lock();
      std::shared_ptr<IJsonInstructionBase> spParentCommand = spParent->m_wpCommand.lock();

      if (nullptr == spChildCommand)
      {
        // squash children
        auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
        if (spParent->m_spChildren.end() != itChild)
        {
          spParent->m_spChildren.erase(itChild);
          spParent->m_spChildren.insert(spParent->m_spChildren.end(),
                                        spChild->m_spChildren.begin(), spChild->m_spChildren.end());
          for (auto& spChildChild : spChild->m_spChildren)
          {
            spChildChild->m_wpParent = spParent;
          }
        }
      }
      else
      {
        if (nullptr != spParentCommand)
        {
          auto itArg = spParent->m_argsDefinition->find(spChild->m_sName);
          if (spParent->m_argsDefinition->end() != itArg)
          {
            // we popped an array type, so we need to insert the values into the arguments as an array value
            // we can also just remove the node from the children since we are done processing
            // the object
            if (EArgumentType::eArray == itArg->second.m_type._to_integral())
            {
              auto it = spChild->m_actualArgs.find(spChild->m_sName);
              if (spChild->m_actualArgs.end() != it)
              {
                spParent->m_actualArgs.insert({spChild->m_sName,
                                               {EArgumentType::eArray, it->second.m_value}});
                auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
                if (spParent->m_spChildren.end() != itChild)
                {
                  spParent->m_spChildren.erase(itChild);
                  spParent->m_spChildren.insert(spParent->m_spChildren.end(),
                                                spChild->m_spChildren.begin(), spChild->m_spChildren.end());
                  for (auto& spChildChild : spChild->m_spChildren)
                  {
                    spChildChild->m_wpParent = spParent;
                  }
                }
              }
              else
              {
                spParent->m_actualArgs.insert({spChild->m_sName,
                                               {EArgumentType::eArray, tInstructionArrayValue{}}});
                auto itChild = std::find(spParent->m_spChildren.begin(), spParent->m_spChildren.end(), spChild);
                if (spParent->m_spChildren.end() != itChild)
                {
                  spParent->m_spChildren.erase(itChild);
                }
              }
            }
          }
        }
      }
    }
  }
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::key(string_t& val)
{
  // starting commands
  if (val == c_sCommandsNode)
  {
    m_bParsingCommands = true;

    // we need to filter parsing of commands, since an argument could be called: 'commands'
    if (m_parseStack.size() <= 0 || m_parseStack.top()->m_wpCommand.expired())
    {
      bool bIgnoreCommandKey = false;
      if (m_parseStack.size() > 0 && nullptr != m_parseStack.top()->m_argsDefinition)
      {
        auto it = m_parseStack.top()->m_argsDefinition->find(c_sCommandsNode);
        bIgnoreCommandKey = m_parseStack.top()->m_argsDefinition->end() != it;
      }

      if (!bIgnoreCommandKey)
      {
        // insert a node
        std::shared_ptr<CJsonInstructionNode> spRotNode =
            std::make_shared<CJsonInstructionNode>();
        spRotNode->m_sName = QString::fromStdString(m_sCurrentKey);
        spRotNode->m_bIgnoreChildren = false;
        m_parseStack.push(spRotNode);
        m_pParent->m_vspBuiltCommands.push_back(
              {QString::fromStdString(m_sCurrentKey), spRotNode});
      }
    }
  }

  // store key
  m_sCurrentKey = val;
  return true;
}

//----------------------------------------------------------------------------------------
//
bool CJSonSaxParser::parse_error(std::size_t position, const std::string& last_token,
                                 const nlohmann::detail::exception& ex)
{
  Q_UNUSED(position)
  m_pParent->m_parseError = ExceptionToStruct(ex);
  m_pParent->m_parseError.m_sToken = QString::fromStdString(last_token);
  return true;
}

//----------------------------------------------------------------------------------------
//
void CJSonSaxParser::CreateAndAddIgnoreNode()
{
  // we don't add to parents children, so this object will be deleted once popped from stack
  std::shared_ptr<CJsonInstructionNode> spNode =
      std::make_shared<CJsonInstructionNode>();
  spNode->m_sName = QString::fromStdString(m_sCurrentKey);
  spNode->m_wpParent = m_parseStack.top();
  spNode->m_bIgnoreChildren = true;
  m_parseStack.push(spNode);
}

//----------------------------------------------------------------------------------------
//
namespace
{
  nlohmann::json MapToJson(
      const tInstructionMapType& argDefinitionList, const tInstructionMapValue& args,
      const std::shared_ptr<CJsonInstructionNode>& spChild, qint32& iOffset);
  std::optional<nlohmann::json> NodeToJson(std::shared_ptr<CJsonInstructionNode> spNode);


  //--------------------------------------------------------------------------------------
  //
  void ElementToJson(const SInstructionArgumentValue& value,
                     const tNestedType& nestedType,
                     const std::shared_ptr<CJsonInstructionNode>& spChild,
                     qint32& iOffset,
                     nlohmann::json& elemParams)
  {
    switch(value.m_type)
    {
      case EArgumentType::eBool:
      {
        bool bValue = std::get<bool>(value.m_value);
        elemParams = bValue;
      } break;
      case EArgumentType::eInt64:
      {
        qint64 iValue = std::get<qint64>(value.m_value);
        elemParams = iValue;
      } break;
      case EArgumentType::eUInt64:
      {
        quint64 uiValue = std::get<quint64>(value.m_value);
        elemParams = uiValue;
      } break;
      case EArgumentType::eDouble:
      {
        double dValue = std::get<double>(value.m_value);
        elemParams = dValue;
      } break;
      case EArgumentType::eString:
      {
        QString sValue = std::get<QString>(value.m_value);
        elemParams = sValue.toStdString();
      } break;
      case EArgumentType::eObject:
      {
        if (static_cast<qint32>(spChild->m_spChildren.size()) > iOffset && 0 <= iOffset)
        {
          std::optional<nlohmann::json> optNode = NodeToJson(spChild->m_spChildren[iOffset]);
          if (optNode.has_value())
          {
            elemParams = optNode.value();
          }
        }
      } break;
      case EArgumentType::eArray:
      {
        std::shared_ptr<SInstructionArgumentType> argListValue =
            std::get<std::shared_ptr<SInstructionArgumentType>>(nestedType);
        tInstructionArrayValue values =
            std::get<tInstructionArrayValue>(value.m_value);
        elemParams = nlohmann::json::array();
        for (qint32 i = 0; static_cast<qint32>(values.size()) > i; ++i)
        {
          nlohmann::json innerObject;
          auto& elem = values[static_cast<size_t>(i)];
          ElementToJson(elem, argListValue->m_nestedType, spChild, iOffset, innerObject);
          iOffset++;
          elemParams.push_back(innerObject);
        }
      } break;
      case EArgumentType::eMap:
      {
        tInstructionMapType argListValue =
            std::get<tInstructionMapType>(nestedType);
        tInstructionMapValue values =
            std::get<tInstructionMapValue>(value.m_value);
        elemParams = MapToJson(argListValue, values, spChild, iOffset);
      } break;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  nlohmann::json MapToJson(
      const tInstructionMapType& argDefinitionList, const tInstructionMapValue& args,
      const std::shared_ptr<CJsonInstructionNode>& spChild, qint32& iOffset)
  {
    nlohmann::json elemParams = nlohmann::json::object();
    for (const auto& [sKey, arg] : argDefinitionList)
    {
      auto itInArgs = args.find(sKey);
      if (args.end() != itInArgs)
      {
        ElementToJson(itInArgs->second, arg.m_nestedType, spChild, iOffset,
                      elemParams[sKey.toStdString()]);
      }
    }

    return elemParams;
  }

  //--------------------------------------------------------------------------------------
  //
  std::optional<nlohmann::json> NodeToJson(std::shared_ptr<CJsonInstructionNode> spNode)
  {
    if (auto spCommand = spNode->m_wpCommand.lock())
    {
      auto argList = spCommand->ArgList();
      auto args = spNode->m_actualArgs;

      if (argList.size() > 0)
      {
        nlohmann::json elem = nlohmann::json::object();
        qint32 iOffset = 0;
        elem[spNode->m_sName.toStdString()] = MapToJson(argList, args, spNode, iOffset);
        return elem;
      }
      else if (spNode->m_spChildren.size() > 0)
      {
        nlohmann::json elem = nlohmann::json::object();
        for (qint32 i = 0; static_cast<qint32>(spNode->m_spChildren.size()) > i; ++i)
        {
          auto& spChildOfChild = spNode->m_spChildren[static_cast<size_t>(i)];
          std::optional<nlohmann::json> optNode = NodeToJson(spChildOfChild);
          if (optNode.has_value())
          {
            elem[spNode->m_sName.toStdString()] = optNode.value();
          }
        }
        return elem;
      }
    }
    // we don't have a command definition, but children,
    // just serialise children directly into the node
    else if (spNode->m_spChildren.size() > 0)
    {
      nlohmann::json elem = nlohmann::json::object();
      for (qint32 i = 0; static_cast<qint32>(spNode->m_spChildren.size()) > i; ++i)
      {
        auto& spChildOfChild = spNode->m_spChildren[static_cast<size_t>(i)];
        std::optional<nlohmann::json> optNode = NodeToJson(spChildOfChild);
        if (optNode.has_value())
        {
          elem[spNode->m_sName.toStdString()] = optNode.value();
        }
      }

      return elem;
    }
    return std::nullopt;
  }

  //--------------------------------------------------------------------------------------
  //
  nlohmann::json ToJson(std::shared_ptr<CJsonInstructionNode> spNode)
  {
    nlohmann::json commands = nlohmann::json::array();
    for (const std::shared_ptr<CJsonInstructionNode>& spChild : spNode->m_spChildren)
    {
      std::optional<nlohmann::json> optNode = NodeToJson(spChild);
      if (optNode.has_value())
      {
        commands.push_back(optNode.value());
      }
    }
    return commands;
  }
}

//----------------------------------------------------------------------------------------
// CJsonInstructionSetParserPrivate implementation
//
class CJsonInstructionSetParserPrivate
{
  friend class CJsonInstructionSetParser;

public:
  CJsonInstructionSetParserPrivate() :
    m_jsonBaseSchema(),
    m_error()
  {

  }
  ~CJsonInstructionSetParserPrivate()
  {

  }

  //--------------------------------------------------------------------------------------
  //
  SJsonException Error() const
  {
    return m_error;
  }

  //--------------------------------------------------------------------------------------
  //
  std::shared_ptr<CJsonInstructionSetRunner> ParseJson(const std::string& json)
  {
    auto spRunner =
        std::make_shared<CJsonInstructionSetRunner>();
    spRunner->m_pPrivate->m_instructionMap = m_instructionMap;
    spRunner->m_pPrivate->m_jsonBaseSchema = m_jsonBaseSchema;
    if (spRunner->m_pPrivate->Validate(json))
    {
      spRunner->m_pPrivate->m_sJson = json;
      CJSonSaxParser spParser(spRunner->m_pPrivate.get(),
                              m_instructionMap);
      if (nlohmann::json::sax_parse(spRunner->m_pPrivate->m_sJson, &spParser))
      {
        m_error = SJsonException();
        return spRunner;
      }
      else
      {
        m_error = spRunner->m_pPrivate->ParseError();
        m_error.m_sException = "Failed to parse json:" + m_error.m_sException;
        qWarning() << m_error.m_sException;
        return nullptr;
      }
    }
    else
    {
      m_error = spRunner->m_pPrivate->ParseError();
      m_error.m_sException = "Failed to validate json:" + m_error.m_sException;
      qWarning() << m_error.m_sException;
      return nullptr;
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterInstruction(const QString& sId,
                           const std::shared_ptr<IJsonInstructionBase>& spInstructionDefinition)
  {
    if (nullptr != spInstructionDefinition)
    {
      m_instructionMap.insert({sId, spInstructionDefinition});
    }
  }

  //--------------------------------------------------------------------------------------
  //
  void RegisterInstructionSetPath(const QString& sId,
                                  const QStringList& vsInstructionSetPath)
  {
    m_instructionSetPath.insert({sId, vsInstructionSetPath});
  }

  //--------------------------------------------------------------------------------------
  //
  void SetJsonBaseSchema(const std::string& json)
  {
    try
    {
      m_jsonBaseSchema = nlohmann::json::parse(json);
    }
    catch (const std::exception &e)
    {
      m_error = ExceptionToStruct(e);
    }
  }

  //--------------------------------------------------------------------------------------
  //
  QString ToJson(std::shared_ptr<CJsonInstructionSetRunner> spRunner)
  {
    nlohmann::json json;
    if (spRunner->Nodes().size() > 1 ||
        (spRunner->Nodes().size() == 1 && !spRunner->Nodes()[0].first.isEmpty()))
    {
      for (const std::pair<QString, std::shared_ptr<CJsonInstructionNode>>& pair : spRunner->Nodes())
      {
        json[pair.first.toStdString()][c_sCommandsNode] = nlohmann::json::array();
        json[pair.first.toStdString()][c_sCommandsNode] = ::ToJson(pair.second);
      }
    }
    else if (spRunner->Nodes().size() == 1)
    {
      json[c_sCommandsNode] = nlohmann::json::array();
      json[c_sCommandsNode] = ::ToJson(spRunner->Nodes()[0].second);
    }
    return QString::fromStdString(json.dump(4));
  }

protected:
  nlohmann::json                                           m_jsonBaseSchema;
  std::map<QString, QStringList>                           m_instructionSetPath;
  std::map<QString, std::shared_ptr<IJsonInstructionBase>> m_instructionMap;
  SJsonException                                           m_error;
};

//----------------------------------------------------------------------------------------
// CJsonInstructionSetRunner implementation
//
CJsonInstructionSetRunner::CJsonInstructionSetRunner() :
  m_pPrivate(std::make_unique<CJsonInstructionSetRunnerPrivate>())
{
  qRegisterMetaType<CJsonInstructionSetRunner::tRetVal>();
  qRegisterMetaType<ERunerMode>();
  qRegisterMetaType<std::shared_ptr<CJsonInstructionNode>>();
  qRegisterMetaType<tInstructionMapValue>();
  qRegisterMetaType<std::shared_ptr<CJsonInstructionSetRunner>>();

  connect(m_pPrivate.get(), &CJsonInstructionSetRunnerPrivate::CommandRetVal,
          this, &CJsonInstructionSetRunner::CommandRetVal);
  connect(m_pPrivate.get(), &CJsonInstructionSetRunnerPrivate::Fork,
          this, &CJsonInstructionSetRunner::Fork);
}

CJsonInstructionSetRunner::~CJsonInstructionSetRunner()
{

}

//----------------------------------------------------------------------------------------
//
CJsonInstructionSetRunner::tRetVal
CJsonInstructionSetRunner::CallNextCommand(ERunerMode runMode, bool bBlocking)
{
  return m_pPrivate->CallNextCommand(runMode, bBlocking);
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<IJsonInstructionBase> CJsonInstructionSetRunner::Instruction(const QString& sName) const
{
  auto it = m_pPrivate->m_instructionMap.find(sName);
  if (m_pPrivate->m_instructionMap.end() != it)
  {
    return it->second;
  }
  return nullptr;
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetRunner::Interrupt()
{
  m_pPrivate->Interrupt();
}

//----------------------------------------------------------------------------------------
//
bool CJsonInstructionSetRunner::IsRunning() const
{
  return m_pPrivate->IsRunning();
}

//----------------------------------------------------------------------------------------
//
CJsonInstructionSetRunner::tRetVal
CJsonInstructionSetRunner::Run(const QString& sInstructionSet, ERunerMode runMode, bool bBlocking)
{
  return m_pPrivate->Run(sInstructionSet, runMode, bBlocking);
}

//----------------------------------------------------------------------------------------
//
const std::vector<std::pair<QString, std::shared_ptr<CJsonInstructionNode>>>&
CJsonInstructionSetRunner::Nodes() const
{
  return m_pPrivate->m_vspBuiltCommands;
}

//----------------------------------------------------------------------------------------
//
CJsonInstructionSetParser::CJsonInstructionSetParser(QObject* pParent) :
  QObject(pParent),
  m_spPtr(std::make_unique<CJsonInstructionSetParserPrivate>())
{

}

CJsonInstructionSetParser::~CJsonInstructionSetParser()
{

}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::ClearInstructions()
{
  m_spPtr->m_instructionMap.clear();
}

//----------------------------------------------------------------------------------------
//
SJsonException CJsonInstructionSetParser::Error() const
{
  return m_spPtr->Error();
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionSetRunner>
CJsonInstructionSetParser::ParseJson(const QByteArray& json)
{
  return m_spPtr->ParseJson(QString::fromUtf8(json).toStdString());
}

//----------------------------------------------------------------------------------------
//
std::shared_ptr<CJsonInstructionSetRunner>
CJsonInstructionSetParser::ParseJson(const QString& sJson)
{
  return m_spPtr->ParseJson(sJson.toStdString());
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::RegisterInstruction(const QString& sId,
                                                    const std::shared_ptr<IJsonInstructionBase>& spInstructionDefinition)
{
  m_spPtr->RegisterInstruction(sId, spInstructionDefinition);
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::RegisterInstructionSetPath(const QString& sId,
                                                           const QString& sInstructionSetPath)
{
  if (sInstructionSetPath == "/")
  {
    m_spPtr->RegisterInstructionSetPath(sId, QStringList() << QString());
  }
  else
  {
    QStringList list = sInstructionSetPath.split("/");
    m_spPtr->RegisterInstructionSetPath(sId, list);
  }
}

//----------------------------------------------------------------------------------------
//
void CJsonInstructionSetParser::SetJsonBaseSchema(const QByteArray& schema)
{
  m_spPtr->SetJsonBaseSchema(QString::fromUtf8(schema).toStdString());
}

//----------------------------------------------------------------------------------------
//
QString CJsonInstructionSetParser::ToJson(std::shared_ptr<CJsonInstructionSetRunner> spRunner)
{
  if (nullptr != spRunner)
  {
    return m_spPtr->ToJson(spRunner);
  }
  return QString();
}

#include "JsonInstructionSetParser.moc"
